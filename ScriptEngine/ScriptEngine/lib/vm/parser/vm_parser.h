#pragma once
#include "../../util/logger.h"
#include "vm_scope.h"
#include "..\lexer\vm_lexer.h"
#include "..\symbol\vm_symbol.h"
#include "..\assembly\vm_assembly_info.h"
#include "error\vm_error.h"
#include <stack>
#include <queue>
#include <stdarg.h>
namespace Sencha {
namespace VM {
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
class VMDriver;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


class varinfo {
public :
	enum Type{
		None          ,
		Symbol        , 
		LiteralValue  , 
		LiteralString ,
	};
private :
	Type m_typeId;
	SymbolInfo* m_symbol;
	string m_literal_string;
	double m_literal_value;
	int m_arrayIndex;
	bool m_isArray;
	bool m_ref;
public :
	const Type& GetType() const {
		return this->m_typeId;
	}
	const double& ToDouble() const {
		return this->m_literal_value;
	}
	varinfo(){
		this->Set( None , NULL , 0 , 0 , "" , false );
	}
	varinfo( SymbolInfo* symbol ){
		this->Set( symbol );
	}
	varinfo( double literal_value ){
		this->Set( LiteralValue , NULL , 0 , literal_value , "" , false );
	}
	varinfo( string literal_string ){
		this->Set( LiteralString , NULL , 0 , 0 , literal_string , false );
	}
	void Set( SymbolInfo* symbol ){
		this->Set( Symbol , symbol , 0 , 0 , "" , false );
		if( symbol->IsReference() ){
			this->Ref();
		}
	}
	void Set( Type typeId , SymbolInfo* symbol , int index , double literal_value , string literal_string , bool isArray ){
		this->m_typeId = typeId;
		this->m_symbol = symbol;
		this->m_literal_string = literal_string;
		this->m_literal_value = literal_value;
		this->m_arrayIndex = index;
		this->m_isArray = isArray;
		this->m_ref = false;
	}

	operator Type() const {
		return this->m_typeId;
	}
	operator SymbolInfo*() const {
		return this->m_symbol;
	}
	operator double() const {
		return this->m_literal_value;
	}
	operator string() const {
		return this->m_literal_string;
	}
	const int& Index() const {
		return this->m_arrayIndex;
	}
	void Index( int index ){
		this->m_arrayIndex = index;
		this->m_isArray = true;
	}
	bool IsArray(){
		return this->m_isArray;
	}
	void Ref(){
		this->m_ref = true;
	}
	bool IsRef(){
		return this->m_ref;
	}
};

class var_chain {
private :
	vector<varinfo> var_list;
public :
	size_t size(){
		return var_list.size();
	}
	varinfo& operator[]( size_t index ){
		assert( index >= 0 && index < size() );
		return var_list[index];
	}
	void push( varinfo& v ){
		var_list.push_back( v );
	}
	varinfo& peek(){
		assert( size() > 0 );
		return var_list[size()-1];
	}
	int array_count(){
		int count = 0;
		for( size_t i = 0 ; i < size() ; i++ ){ if( var_list[i].IsArray() ){ count++; } }
		return count;
	}
	bool contains_array(){
		for( size_t i = 0 ; i < size() ; i++ ){ if( var_list[i].IsArray() ){ return true; } }
		return false;
	}
};

// ************************************************
// �\����͊�
// ************************************************
class Parser : public IAssembleReader {
	friend class Args;
	friend class interpreter;
private :
	// �W�����v���ߏ��
	// continue��/break�����g�p����Ƃ��ɂǂ��ł��̖��߂��������̂��A
	// �Ƃ��������R�[�h�A�h���X���L�^����
	struct JumpInfo {
		int pos;
	};

	// ��͎��p�����[�^
	// Parse�֐��Ƀ|�C���^�œn��
	// ��{�I�ɂ��̃C���X�^���X�͎����̈�Ɏ��(_parse�͍ċA����̂�)
	class Args {
	private :
		Parser* m_parser;
	public :
		vector<JumpInfo> Continue; // continue���������̃o�C�g�ʒu
		vector<JumpInfo> Break; // break���������̃o�C�g�ʒu
		Args( Parser* parser ){
			this->m_parser = parser;
		}
		void WriteBreak( int breakPos ){
			for( size_t i = 0 ; i < Break.size() ; i++ ){
				this->m_parser->m_writer->writeInt32( breakPos , Break[i].pos );
			}
		}
		void WriteContinue( int continuePos ){
			for( size_t i = 0 ; i < Continue.size() ; i++ ){
				this->m_parser->m_writer->writeInt32( continuePos , Continue[i].pos );
			}
		}
	};

	// �p�[�T�[��͊��
	// �p�[�T�[�̃R���g���[���S�ʂ���舵���@�\���p����ɒ񋟂���
	class interpreter {
	protected :
		Parser* m_parser;
	public :
		interpreter( Parser* parser ){
			m_parser = parser;
		}
		string getFullName( string& funcName ){
			return this->m_parser->m_currentScope->toFullName( funcName );
		}
		bool NextTokenIf( int tokenType ){
			return m_parser->getToken(1).type == tokenType;
		}
		bool TokenIf( int tokenType ){
			return m_parser->getToken(0).type == tokenType;
		}
		void Next(){
			m_parser->nextToken();
		}
		void Back(){
			this->m_parser->backToken();
		}
		void Log( const char* formatString , ... ){
			va_list args;
			va_start( args , formatString );
			if( this->m_parser->m_log ){
				this->m_parser->m_log->print( formatString , args );
			}
			va_end( args );
		}
		int getTokenType(){
			return m_parser->getToken().type;
		}
		int getTokenInt(){
			return atoi( m_parser->getToken().text.c_str() );
		}
		const char* getTokenString(){
			return m_parser->getToken().text.c_str();
		}
		double getTokenDouble(){
			return atof( m_parser->getToken().text.c_str() );
		}
		void WriteEndFunc(){
			this->m_parser->m_writer->write( EMnemonic::EndFunc );
		}
		void WriteReturn( int R ){
			this->m_parser->m_writer->write( EMnemonic::RET );
			this->m_parser->m_writer->write( EMnemonic::REG );
			this->m_parser->m_writer->write( R );
			this->WriteEndFunc();
		}
		/*
		 * �W�����v��A�h���X��Writer�̌��݈ʒu��ݒ�
		 * @param pos ... �W�����v���߂̈ړ���A�h���X�ݒ�n�_(s32�^)
		 */
		void WriteJmpPos( int pos ){
			this->m_parser->m_writer->writeInt32( this->m_parser->m_writer->count() , pos );
		}

		int WriteJmpCommand( int cmd , int jmp ){
			this->m_parser->m_writer->write( cmd );
			int pos = this->m_parser->m_writer->count();
			this->m_parser->m_writer->writeInt32( jmp );
			return pos;
		}
		int WriteJZ( int pos ){
			return this->WriteJmpCommand( EMnemonic::JumpZero , pos );
		}
		int WriteJZ(){
			return this->WriteJZ(0);
		}
		int WriteJNZ( int pos ){
			return this->WriteJmpCommand( EMnemonic::JumpNotZero , pos );
		}
		int WriteJNZ(){
			return this->WriteJNZ(0);
		}
		int WriteJ( int pos ){
			return this->WriteJmpCommand( EMnemonic::Jmp , pos );
		}
		int WriteJ(){
			return this->WriteJ(0);
		}
		int GetWritePos(){
			return this->m_parser->m_writer->count();
		}
		int GetFuncAddres( string& funcName ){
			return this->m_parser->getFuncAddres( funcName );
		}

		/*
		 * �V�����o�C�i�����C�^�𐶐����ČÂ����Ԃ�
		 */
		CBinaryWriter CreateNewWriter(){
			CBinaryWriter result = this->m_parser->m_writer;
			this->m_parser->m_writer = CBinaryWriter( new BinaryWriter() );
			return result;
		}
		/*
		 * ���C�^�[�����Ĉ���O�̏�Ԃ�Ԃ�
		 */
		CBinaryWriter SetWriter( CBinaryWriter newObj ){
			CBinaryWriter result = this->m_parser->m_writer;
			this->m_parser->m_writer = newObj;
			return result;
		}
		/*
		 * ���C�^�[�I�u�W�F�N�g������������
		 */
		void AppendWriter( CBinaryWriter src ){
			this->m_parser->m_writer->append( *src );
		}

		/*
		 * ���݂̃g�[�N���ɑΉ��������O�̌^�����擾
		 */
		Type* getType(){
			Type* t = (Type*)this->m_parser->m_currentScope->findScopeFromTop( this->getTokenString() );
			return t;
		}

		/* 
		 * �A�Z���u���̓o�^
		 * (�|�C���^�n���ɂ����ق����R�s�[���������Ȃ��̂ő����H��������Ȃ��̂ł��̂�����������)
		 */
		void EntryAssembly( AsmInfo* funcAssembly ){ 
			this->m_parser->m_asm->entryAssembly( funcAssembly );
		}

		int GetFunctionAddres( string& funcName ){
			return this->m_parser->m_asm->find( funcName );
		}
		/*
		 * �w��̖��O�̃A�Z���u�����擾����
		 */
		AsmInfo* GetAssembly( string funcName ){
			return this->m_parser->m_asm->indexAt( GetFunctionAddres( funcName ) );
		}

		/*
		 * ���݂̃X�R�[�v���犄�蓖�Ă���ϐ��̗̈�𔻒肷��B 
		 * ���[�J���̈�Ȃ̂��ÓI�̈�Ȃ̂������݂̃X�R�[�v���画��
		 */
		ESymbolType GetVariableLocationInCurrentScope(){
			ESymbolType scopeSymbolType = VariableLocal;
			if( this->m_parser->m_currentScope->ScopeLevel() == 0 ){
				scopeSymbolType = VariableGlobal;
			}
			return scopeSymbolType;
		}
		/*
		 * �֐���͏���
		 */
		MethodInfo* GoToFunction( const string& funcName ){
			this->m_parser->m_writer = CBinaryWriter( new BinaryWriter() );
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToFunctionScope( funcName );
			return reinterpret_cast<MethodInfo*>( this->m_parser->m_currentScope );
		}
		/*
		 * �\���̃X�R�[�v�ֈړ�
		 */
		Type* GoToStruct( const string& typeName ){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToStructScope( typeName );
			return reinterpret_cast<Type*>( this->m_parser->m_currentScope );
		}
		/*
		 * �`�����N�X�R�[�v�ֈړ�
		 */
		Scope* GoToChunk(){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToChildScope( "" );
			return this->m_parser->m_currentScope;
		}

		/*
		 * ���̃X�R�[�v�ɖ߂�
		 */
		Scope* GoToBack(){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->backToChildScope();
			return this->m_parser->m_currentScope;
		}
		/*
		 * �o�C�i�����C�^�[�̃X�g���[�����擾
		 */
		CStream GetCurrentStream(){
			return this->m_parser->m_writer->getStream();
		}
		/*
		 * ���̃g�[�N�����w��̂��̂łȂ��ꍇ�G���[�ƌ��Ȃ���2059�ԃG���[�𓊂���B
		 * �w��̂��̂ł���ꍇ�͎��ɐi�߂�
		 */
		void ErrorCheckNextToken( int tokenType ){
			if( !this->NextTokenIf( tokenType ) ){
				this->Next();
				throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
			}
			this->Next();
		}
		/*
		 * ���݂̃g�[�N�����w��̂��̂łȂ��ꍇ�G���[�ƌ��Ȃ���2059�ԃG���[�𓊂���B
		 */
		void ErrorCheckToken( int tokenType ){
			if( !this->TokenIf( tokenType ) ){
				throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
			}
		}

		/*
		 * ��͏���
		 */
		void Parse( Args* args ){
			this->m_parser->parse( args );
			this->Back();
		}
		/*
		 * ��͏���
		 */
		void Parse(){
			this->Parse( NULL );
		}

		/*
		 * �w��̃g�[�N����������܂ŉ�͂�i�߂�
		 * ������Ȃ��ꍇ�̓G���[��Ԃ�
		 */
		void ParseWhile( int tokenType , Args* args ){
			if( this->TokenIf( tokenType ) ){
				return;
			}
			while( this->m_parser->hasNext() ){
				this->m_parser->parse( args );
				if( this->TokenIf( tokenType ) ){
					return;
				}
			}
			throw VMError( new ERROR_INFO_C2143( tokenType ) );
		}
		/*
		 * �w��̃g�[�N����������܂ŉ�͂�i�߂�
		 */
		void ParseWhile( int tokenType ){
			this->ParseWhile( tokenType , NULL );
		}

		/*
		 * ���݂̃g�[�N������V���{�����擾����B
		 */
		SymbolInfo* getSymbol(){
			const string& symbolName = m_parser->getToken().text;
			return m_parser->m_currentScope->getSymbol( symbolName );	
		}
		/*
		 * �V���{���o�^
		 */
		SymbolInfo* addSymbol( string& symbolName ){
			return m_parser->m_currentScope->addSymbol( symbolName );
		}
		/*
		 * �f�[�^�^�C�v���X�R�[�v���̂ǂ��炩����V���{���擾
		 */
		SymbolInfo* getSymbolInScopeOrType( Type* t , const string& symbolName ){
			if( t ){
				return t->getSymbol( symbolName );
			}
			return m_parser->m_currentScope->getSymbol( symbolName );
		}
		/*
		 * ���݂̃g�[�N������V���{�����݂��邩�m�F
		 */
		bool ExistSymbol(){
			const string& symbolName = m_parser->getToken().text;
			SymbolInfo* symbol = m_parser->m_currentScope->getSymbol( symbolName );
			if( symbol ) return true;
			return false;
		}
	};

	// �ϐ��V���{�����
	class parse_variable : public interpreter {
	public :
		parse_variable( Parser* parser );
	};
	// �\���̉��
	class parse_struct : public interpreter {
	public : 
		parse_struct( Parser* parser );
	};
	// �֐����
	class parse_function : public interpreter {
	private :
		string funcName;
		vector<SymbolInfo*> args;
	public : 
		const vector<SymbolInfo*>& GetArgs() const{
			return this->m_parser->m_currentScope->getSymbols();
		}
		int GetStackFrame(){
			return this->m_parser->m_currentScope->getSymbolCountMaxInAllScope( this->GetVariableLocationInCurrentScope() );
		}
		// �\���̃X�R�[�v�ł��邩�ǂ���
		// �e�X�R�[�v�����Ĕ��f����
		bool isStructScope(){
			Scope* parent = this->m_parser->m_currentScope->getParentScope();
			if( !parent ){
				return false;
			}
			return parent->isStructScope();
		}
		// ���݂̃X�R�[�v���\���̃����o�֐��X�R�[�v�ł��邱�Ƃ�ʒm����
		void NotifyStructMethodScope(){
			this->m_parser->m_currentScope->notifyStructMethodScope();
		}
		parse_function( Parser* parser );
		/*
		 * �֐��̗\��o�^
		 */
		void TransactFunction();
		/*
		 * ����������m��\������B
		 */
		void CommitFunction();
		void This();
	};
	// as���Z�q���
	class parse_as : public interpreter {
	public :
		parse_as( Parser* parser , varinfo& var );
		void checkArray( varinfo& var );
	};
	// �z����
	class parse_array : public interpreter {
	public :
		parse_array( Parser* parser , SymbolInfo* symbol );
	};
	// �`�����N���
	class parse_chunk : public interpreter {
	public :
		parse_chunk( Parser* parser , Args* args );
	};
	// return��
	class parse_return : public interpreter {
	public :
		parse_return( Parser* parser );
	};
	class parse_if : public interpreter {
	public :
		parse_if( Parser* parser , Args* args );
	};
	class parse_switch : public interpreter {
	public :
		parse_switch( Parser* parser );
	};
	class parse_for : public interpreter {
	public :
		parse_for( Parser* parser );
	};
	class parse_while : public interpreter {
	public :
		parse_while( Parser* parser );
	};
	class parse_continue : public interpreter {
	public :
		parse_continue( Parser* parser , Args* args );
	};
	class parse_break : public interpreter {
	public :
		parse_break( Parser* parser , Args* args );
	};

	// ���]�����
	class expression : public interpreter {
	public :
		int R;
		void ExprPushData( const double& literal_value ){
			varinfo exp_data( literal_value );
			MovR( exp_data );
		}
		void ExprPushData( const string& literal_string ){
			varinfo exp_data( literal_string );
			MovR( exp_data );
		}
		void ExprPushData( var_chain& var ){
			if( var.contains_array() ){
				R -= var.array_count();
			}
			MovR( var );
		}

		void PushThis( var_chain& var ){
			this->m_parser->m_writer->write( EMnemonic::MovPtr );
			this->WriteR();
			this->WriteData( var );
			this->R++;
			this->PushPtr();
		}

		void Assign( var_chain& src , int opetype ){
			switch( opetype ){
				case Token::Type::AddAssign : this->m_parser->m_writer->write( EMnemonic::Add ); break;
				case Token::Type::SubAssign : this->m_parser->m_writer->write( EMnemonic::Sub ); break;
				case Token::Type::MulAssign : this->m_parser->m_writer->write( EMnemonic::Mul ); break;
				case Token::Type::DivAssign : this->m_parser->m_writer->write( EMnemonic::Div ); break;
				case Token::Type::RemAssign : this->m_parser->m_writer->write( EMnemonic::Rem ); break;
				case Token::Type::Assign    : this->m_parser->m_writer->write( EMnemonic::Mov ); break;
			}
			this->WriteData( src );
			this->WritePopR();
		}

		void CalcStack( int opetype ){
			switch( opetype ){
				case Token::Type::Add        : this->m_parser->m_writer->write( EMnemonic::Add );    break;
				case Token::Type::Sub        : this->m_parser->m_writer->write( EMnemonic::Sub );    break;
				case Token::Type::Mul        : this->m_parser->m_writer->write( EMnemonic::Mul );    break;
				case Token::Type::Div        : this->m_parser->m_writer->write( EMnemonic::Div );    break;
				case Token::Type::Rem        : this->m_parser->m_writer->write( EMnemonic::Rem );    break;
				case Token::Type::Equal      : this->m_parser->m_writer->write( EMnemonic::CmpEq );  break;
				case Token::Type::NotEqual   : this->m_parser->m_writer->write( EMnemonic::CmpNEq ); break;
				case Token::Type::GEq        : this->m_parser->m_writer->write( EMnemonic::CmpGeq ); break;
				case Token::Type::Greater    : this->m_parser->m_writer->write( EMnemonic::CmpG );   break;
				case Token::Type::LEq        : this->m_parser->m_writer->write( EMnemonic::CmpLeq ); break;
				case Token::Type::Lesser     : this->m_parser->m_writer->write( EMnemonic::CmpL );   break;
				case Token::Type::LogicalAnd : this->m_parser->m_writer->write( EMnemonic::LogAnd ); break;
				case Token::Type::LogicalOr  : this->m_parser->m_writer->write( EMnemonic::LogOr );  break;
			}
			this->WriteR( -2 );
			this->WriteR( -1 );
			R -= 1;
		}

		void Push(){
			this->m_parser->m_writer->write( EMnemonic::Push );
			this->WritePopR();
		}

		void PushPtr(){
			this->m_parser->m_writer->write( EMnemonic::PushPtr );
			this->WritePopR();
		}

		void CallFunction( string& funcName ){
			this->m_parser->m_writer->write( EMnemonic::ST );
			this->m_parser->m_writer->write( R );
			this->m_parser->m_writer->write( EMnemonic::Call );
			this->m_parser->m_writer->writeInt32( this->GetFuncAddres( funcName ) );
			this->m_parser->m_writer->write( EMnemonic::Mov );
			this->m_parser->m_writer->write( EMnemonic::REG );
			this->m_parser->m_writer->write( R );
			this->m_parser->m_writer->write( EMnemonic::REG );
			this->m_parser->m_writer->write( 0 );
			this->m_parser->m_writer->write( EMnemonic::LD );
			this->m_parser->m_writer->write( R );
			R++;
		}

		void WriteData( varinfo& src ){
			switch( (varinfo::Type)src ){
			case varinfo::LiteralValue :
				this->m_parser->m_writer->write( EMnemonic::LIT_VALUE );
				this->m_parser->m_writer->writeDouble( (double)src );
				break;
			case varinfo::LiteralString : 
				this->m_parser->m_writer->write( EMnemonic::LIT_STRING );
				this->m_parser->m_writer->writeString( (string)src );
				break;
			case varinfo::Symbol :	
				{
					this->m_parser->m_writer->write( ((SymbolInfo*)src)->toCode() );
					this->m_parser->m_writer->writeInt32( 1 );
					this->m_parser->m_writer->write( src.IsArray() );
					this->m_parser->m_writer->write( src.IsRef() );
					this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src)->Addr() );
					if( src.IsArray() ){
						this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src)->DataTypeSizeOf() );
						this->m_parser->m_writer->writeInt32( src.Index() );
					}
				}
				break;
			}
		}
		void WriteData( var_chain& src ){
			this->m_parser->m_writer->write( ((SymbolInfo*)src[0])->toCode() );
			this->m_parser->m_writer->writeInt32( src.size() );
			for( size_t i = 0 ; i < src.size() ; i++ ){
				this->m_parser->m_writer->write( src[i].IsArray() );
				this->m_parser->m_writer->write( src[i].IsRef() );
				this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src[i])->Addr() );
				if( src[i].IsArray() ){
					this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src[i])->DataTypeSizeOf() );
					this->m_parser->m_writer->writeInt32( src[i].Index() );
				}
			}
		}

		void MovR( varinfo& src ){
			this->m_parser->m_writer->write( EMnemonic::Mov );
			this->WritePushR();
			this->WriteData( src );
		}

		void MovR( var_chain& src ){
			this->m_parser->m_writer->write( EMnemonic::Mov );
			this->WritePushR();
			this->WriteData( src );
		}
		void WritePushR(){
			this->WriteR();
			this->R++;
		}
		void WritePopR(){
			this->R--;
			this->WriteR();
		}
		void WriteR(){
			this->m_parser->m_writer->write( EMnemonic::REG );
			this->m_parser->m_writer->write( R );
		}
		void WriteR( int ofs ){
			this->m_parser->m_writer->write( EMnemonic::REG );
			this->m_parser->m_writer->write( R + ofs );
		}
		void WriteMovR( varinfo& src ){
			this->MovR( src );
		}
		void WriteNot(){
			this->m_parser->m_writer->write( EMnemonic::Not );
			this->WritePopR();
			this->R++;
		}
		void WriteMinus(){
			this->m_parser->m_writer->write( EMnemonic::Minus );
			this->WritePopR();
			this->R++;
		}

		expression( Parser* parser );
		expression( Parser* parser , expression* e );

		void Clone( expression* e ){
			this->R = e->R;
		}
	};
	// ���]���������
	class expression_base : public interpreter {
	private :
		expression* m_exp;
	public :
		expression_base( expression* exp , Parser* parser ) : interpreter( parser ){
			m_exp = exp;
		}
		void ExprPushData( const double& literal_value ){ m_exp->ExprPushData( literal_value ); }
		void ExprPushData( const string& literal_string ){ m_exp->ExprPushData( literal_string ); }
	};
	// �]��0 
	// =
	// +=
	// -=
	// *=
	// /=
	// %=
	class expression0 : public expression_base {
	public :
		expression0( expression* exp , Parser* parser , var_chain& v );
	};

	// �]��1
	// ||
	class expression1 : public expression_base {
	public :
		expression1( expression* exp , Parser* parser );
	};

	// �]��2
	// &&
	class expression2 : public expression_base {
	public :
		expression2( expression* exp , Parser* parser );
	};

	// �]��3
	// !=
	// ==
	// >=
	// <=
	// >
	// <
	class expression3 : public expression_base {
	public :
		expression3( expression* exp , Parser* parser );
	};

	// �]��3
	// +
	// -
	class expression4 : public expression_base {
	public :
		expression4( expression* exp , Parser* parser );
	};

	// �]��4
	// *
	// /
	// %
	class expression5 : public expression_base {
	public :
		expression5( expression* exp , Parser* parser );
	};

	// �]��5
	// symbol
	class expression6 : public expression_base {
	public :
		expression6( expression* exp , Parser* parser );
	};

	// �ϐ��]��
	class expression_variable : public expression_base {
	private :
		expression* expr;
		var_chain var;
		Type* type;
	public :
		expression_variable( expression* exp , Parser* parser );
		expression_variable( expression* exp , Parser* parser , var_chain& var , Type* t );
	private :
		void exp();
		void checkMemberFunc( const string& symbolName );
		void bracket( const string& symbolName );
		void dot( const string& symbolName );
		void memberFunc( string& symbolName );
		bool isExistSymbolInType( const string& symbolName ){
			assert( this->type );
			return this->type->getSymbol( symbolName ) ? true : false;
		}
		SymbolInfo* getThis(){
			assert( this->type );
			string name = this->type->Name();
			return this->type->getSymbol( name );
		}
	};

	// []�]��
	class expression_bracket : public expression_base {
	public :
		expression_bracket( expression* exp , Parser* parser , Type* type , var_chain& v );
	};

	// �֐��]��
	class expression_func : public expression_base {
	public :
		expression_func( expression* exp , Parser* parser );
	};
private :
	VMAssembleCollection* m_asm;
	VMBuiltIn* m_built_in;
	CBinaryWriter m_writer;
	Scope* m_scope;
	Scope* m_currentScope;
	ITokenizer* m_token;
	Log* m_log;
public :
	Parser( ITokenizer* tokenizer , VMBuiltIn* built_in , Log* logger );
	Parser( ITokenizer* tokenizer , VMBuiltIn* built_in );
	Parser( ITokenizer* tokenizer );
	virtual ~Parser();
	virtual AsmInfo* getAssembly( int index ){
		if( this->m_asm ) return this->m_asm->indexAt( index );
		return NULL;
	}
	virtual AsmInfo* getAssembly( std::string name ){
		if( this->m_asm ) return this->m_asm->indexAt( this->m_asm->find( name ) );
		return NULL;
	}
private :
	void initialize( ITokenizer* tokenizer , VMBuiltIn* built_in , Log* logger );
	void execute();
private :
	const Token& backToken();
	const Token& nextToken();
	const Token& getToken();
	const Token& getToken(int ofs);
	bool hasNext();
private :
	void parse( Args* args );
	int getFuncAddres( string& funcName );
};
typedef std::shared_ptr<Parser> CParser;

} // namespace Assembly
} // namespace VM
} // namespace Sencha

