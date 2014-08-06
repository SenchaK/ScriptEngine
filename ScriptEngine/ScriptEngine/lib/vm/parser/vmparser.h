#pragma once
#include "vmscope.h"
#include "..\lexer\vmlexer.h"
#include "..\symbol\vmsymbol.h"
#include "..\assembly\vmassembly_info.h"
#include "error\vmerror.h"
#include <stack>
#include <queue>

namespace SenchaVM {
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


// �I�u�W�F�N�g���^�f�[�^
class Metadata {
};

// �v�Z�@�X�^�b�N
// �ϐ��A�֐��Ȃǂ̃V���{����m_symbol  ,
// ���Z�q�Ȃ�                m_operator,
// ���e�����̏ꍇ            m_literal ,
// �ɂ��ꂼ��̑Ή��l������
class OperationStack {
private :
	int         m_type;
	SymbolInfo* m_symbol;
	TOKEN_TYPE  m_operator;
	string      m_literal;
public :
	OperationStack( SymbolInfo* symbol ){
		m_type     = 0;
		m_symbol   = symbol;
		m_operator = TokenType::NONCE;
		m_literal  = "";
	}
	OperationStack( TOKEN_TYPE operator_type ){
		m_type     = 1;
		m_symbol   = NULL;
		m_operator = operator_type;
		m_literal  = "";
	}
	OperationStack( string literal_value ){
		m_type     = 2;
		m_symbol   = NULL;
		m_operator = TokenType::NONCE;
		m_literal  = literal_value;
	}
	TOKEN_TYPE getOperator(){
		return m_operator;
	}
	SymbolInfo* const Symbol(){
		return m_symbol;
	}
	const string& Literal(){
		return m_literal;
	}
};


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
	const Type& GetType(){
		return this->m_typeId;
	}
	double ToDouble(){
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

	operator Type(){
		return this->m_typeId;
	}
	operator SymbolInfo*(){
		return this->m_symbol;
	}
	operator double(){
		return this->m_literal_value;
	}
	operator string(){
		return this->m_literal_string;
	}
	int Index(){
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
class Parser {
	friend class interpreter;
private :
	// �W�����v���ߏ��
	// continue��/break�����g�p����Ƃ��ɂǂ��ł��̖��߂��������̂��A
	// �Ƃ��������R�[�h�A�h���X���L�^����
	struct JumpInfo {
		int codeAddr;
	};

	// ��͎��p�����[�^
	// _parse�֐��Ƀ|�C���^�œn��
	// ��{�I�ɂ��̃C���X�^���X�͎����̈�Ɏ��(_parse�͍ċA����̂�)
	struct Context {
		vector<JumpInfo> continueAddr ; // continue���������̃o�C�g�ʒu
		vector<JumpInfo> breakAddr    ; // break���������̃o�C�g�ʒu
		vector<SymbolInfo*> symbolArgs;
		size_t args;
		Context(){
			args = 0;
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
		bool NextTokenIf( TOKEN_TYPE tokenType ){
			return m_parser->getToken(1).type == tokenType;
		}
		bool TokenIf( TOKEN_TYPE tokenType ){
			return m_parser->getToken(0).type == tokenType;
		}
		void Next(){
			m_parser->nextToken();
		}
		TOKEN_TYPE getTokenType(){
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
		void WhiteEndFunc(){
			this->m_parser->m_writer->write( EMnemonic::EndFunc );
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
		void EntryAssembly( const AssemblyInfo& funcAssembly ){ 
			this->m_parser->m_assemblyCollection.assemblyInfo.push_back( funcAssembly );
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
		void ErrorCheckNextToken( TOKEN_TYPE tokenType ){
			if( !this->NextTokenIf( tokenType ) ){
				this->Next();
				throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
			}
			this->Next();
		}
		/*
		 * ��͏���
		 */
		void Parse( Context* param ){
			this->m_parser->_parse( param );
		}
		/*
		 * �w��̃g�[�N����������܂ŉ�͂�i�߂�
		 * ������Ȃ��ꍇ�̓G���[��Ԃ�
		 */
		void ParseWhile( TOKEN_TYPE tokenType , Context* param ){
			if( this->TokenIf( tokenType ) ){
				return;
			}
			while( this->m_parser->hasNext() ){
				this->Parse( param );
				if( this->TokenIf( tokenType ) ){
					return;
				}
			}
			throw VMError( new ERROR_INFO_C2143( tokenType ) );
		}
		/*
		 * �w��̃g�[�N����������܂ŉ�͂�i�߂�
		 */
		void ParseWhile( TOKEN_TYPE tokenType ){
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
		/*
		 * ����V���{���̎q�K�w�ɂ��̃V���{�������݂��邩
		 * ���݂���ꍇ�A�����o�[�Ƃ��Ĉ���
		 */
		bool ExistSymbolMember( string& inst , string& member ){
			SymbolInfo* instSymbol = m_parser->m_currentScope->getSymbol( inst );
			assert( instSymbol );
			SymbolInfo* memberSymbol = instSymbol->getSymbol( member );
			if( memberSymbol ) return true;
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
		vector<SymbolInfo*> GetArgs(){
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
		void EntryFunction();
		void This();
	};
	// as���Z�q���
	class parse_as : public interpreter {
	public :
		parse_as( Parser* parser , varinfo& var );
	};
	// �z����
	class parse_array : public interpreter {
	public :
		parse_array( Parser* parser , SymbolInfo* symbol );
	};
	// �`�����N���
	class parse_chunk : public interpreter {
	public :
		parse_chunk( Parser* parser , SymbolInfo* symbol );
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
		void PushThis( varinfo _this ){
			_this.Ref();
			this->MovR( _this );
			this->Push();
		}

		void Assign( var_chain& src ){
		//	this->WriteData( src );
			this->WritePopR();
			printf( "mov " );
			for( size_t i = 0 ; i < src.size() ; i++ ){
				if( src[i].IsArray() ){
					printf( "*(" );
				}
				if( src[i].IsRef() ){
					printf( "&" );
				}
				printf( "%s[%d]" , ((SymbolInfo*)src[i])->Name().c_str() , ((SymbolInfo*)src[i])->Addr() );
				if( src[i].IsArray() ){
					printf( "+(sizeof(%s)*R%d))" ,  ((SymbolInfo*)src[i])->DataTypeName().c_str() , src[i].Index() );
				}
				if( i + 1 < src.size() ){
					printf( "+" );
				}
			}
			printf( ",R%d\n" , R );
		}

		void CalcStack( int opetype ){
			switch( opetype ){
			case TokenType::Add : this->m_parser->m_writer->write( EMnemonic::Add ); break;
			case TokenType::Sub : this->m_parser->m_writer->write( EMnemonic::Sub ); break;
			case TokenType::Mul : this->m_parser->m_writer->write( EMnemonic::Mul ); break;
			case TokenType::Div : this->m_parser->m_writer->write( EMnemonic::Div ); break;
			case TokenType::Rem : this->m_parser->m_writer->write( EMnemonic::Rem ); break;
			}
			switch( opetype ){
			case TokenType::Add : printf( "add R%d , R%d\n" , R - 2 , R - 1 ); break;
			case TokenType::Sub : printf( "sub R%d , R%d\n" , R - 2 , R - 1 ); break;
			case TokenType::Mul : printf( "mul R%d , R%d\n" , R - 2 , R - 1 ); break;
			case TokenType::Div : printf( "div R%d , R%d\n" , R - 2 , R - 1 ); break;
			case TokenType::Rem : printf( "rem R%d , R%d\n" , R - 2 , R - 1 ); break;
			}
			this->WriteR( -2 );
			this->WriteR( -1 );
			R -= 1;
		}

		void Push(){
			printf( "push R%d\n" , R-1 );
			R--;
		}

		void CallFunction( const string& funcName ){
			printf( "st  %d\n" , R );
			printf( "cal %s\n" , funcName.c_str() );
			printf( "mov R%d , R%d\n" , R , 0 );
			printf( "ld  %d\n" , R );
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
				this->m_parser->m_writer->write( ((SymbolInfo*)src)->toAssembleCode() );
				this->m_parser->m_writer->write( ((SymbolInfo*)src)->isArray() );
				this->m_parser->m_writer->write( ((SymbolInfo*)src)->isReferenceMember() );
				this->m_parser->m_writer->write( ((SymbolInfo*)src)->IsReference() );
				if( ((SymbolInfo*)src)->isReferenceMember() ){
					this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src)->TopSymbolAddr() );
				}
				this->m_parser->m_writer->writeInt32( ((SymbolInfo*)src)->Addr() );
				this->m_parser->m_writer->writeString( ((SymbolInfo*)src)->Name() );
				break;
			}
		}
		void MovR( varinfo& src ){
			switch( (varinfo::Type)src ){
			case varinfo::LiteralValue : 
				printf( "mov R%d,%0.2f\n" , R , (double)src ); 
				break;
			case varinfo::LiteralString : 
				printf( "mov R%d,%s\n" , R , ((string)src).c_str() ); 
				break;
			case varinfo::Symbol : 
				printf( "mov " );
				printf( "R%d," , R );
				if( src.IsArray() ){
					printf( "*(" );
				}
				if( src.IsRef() ){
					printf( "&" );
				}
				printf( "%s[%d]" , ((SymbolInfo*)src)->Name().c_str() , ((SymbolInfo*)src)->Addr() );
				if( src.IsArray() ){
					printf( "+(sizeof(%s)*R%d))" ,  ((SymbolInfo*)src)->DataTypeName().c_str() , src.Index() );
				}
				printf( "\n");
				break;
			}

			this->m_parser->m_writer->write( EMnemonic::Mov );
			this->WritePushR();
			this->WriteData( src );
		}

		void MovR( var_chain& src ){
			printf( "mov " );
			printf( "R%d," , R );
			for( size_t i = 0 ; i < src.size() ; i++ ){
				if( src[i].IsArray() ){
					printf( "*(" );
				}
				if( src[i].IsRef() ){
					printf( "&" );
				}
				printf( "%s[%d]" , ((SymbolInfo*)src[i])->Name().c_str() , ((SymbolInfo*)src[i])->Addr() );
				if( src[i].IsArray() ){
					printf( "+(sizeof(%s)*R%d))" ,  ((SymbolInfo*)src[i])->DataTypeName().c_str() , src[i].Index() );
				}
				if( i + 1 < src.size() ){
					printf( "+" );
				}
			}
			printf( "\n");

			this->m_parser->m_writer->write( EMnemonic::Mov );
			this->WritePushR();
			//this->WriteData( src );
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
		expression( Parser* parser );
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
	class expression0 : public expression_base {
	public :
		expression0( expression* exp , Parser* parser , var_chain& v );
	};
	class expression1 : public expression_base {
	public :
		expression1( expression* exp , Parser* parser );
	};
	class expression2 : public expression_base {
	public :
		expression2( expression* exp , Parser* parser );
	};
	class expression3 : public expression_base {
	public :
		expression3( expression* exp , Parser* parser );
	};
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
		void bracket( const string& symbolName );
		void dot( const string& symbolName );
		void memberFunc( const string& symbolName );
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
	class expression_bracket : public expression_base {
	public :
		expression_bracket( expression* exp , Parser* parser , Type* type , var_chain& v );
	};
	class expression_func : public expression_base {
	public :
		expression_func( expression* exp , Parser* parser );
	};
private :
	vector<TOKEN> m_tokens;
	VMAssembleCollection m_assemblyCollection;
	CBinaryWriter m_writer;
	CScope m_scope;
	Scope* m_currentScope;
	size_t m_pos;
public :
	const VMAssembleCollection& getResult(){ return m_assemblyCollection; }
	Parser( vector<TOKEN> tokens );
	~Parser();
private :
	void _execute();
	void _initialize( vector<TOKEN> tokens );
private :
	// �g�[�N�������Ȃ�
	const TOKEN& backToken();
	const TOKEN& nextToken();
	const TOKEN& getToken();
	const TOKEN& getToken(int ofs);
	bool hasNext();
	void _consume( int consumeCount );
private :
	// �X�e�[�g�����g�Ȃ�
	void _parse( Context* param );
	void _parse_if( Context* param );
	void _parse_switch( Context* param );
	void _parse_for( Context* param );
	void _parse_while( Context* param );
	void _parse_chunk( Context* param );
	void _parse_continue( Context* param );
	void _parse_break( Context* param );
	void _parse_struct( SymbolInfo* structSymbol );
	void _parse_return( Context* param );
private :
	void _skipParen();
};
typedef std::shared_ptr<Parser> CParser;

} // namespace Assembly
} // namespace SenchaVM

