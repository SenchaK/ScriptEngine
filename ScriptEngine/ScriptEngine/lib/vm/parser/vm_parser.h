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
// 構文解析器
// ************************************************
class Parser : public IAssembleReader {
	friend class Args;
	friend class interpreter;
private :
	// ジャンプ命令情報
	// continue文/break文を使用するときにどこでその命令を見つけたのか、
	// という内部コードアドレスを記録する
	struct JumpInfo {
		int pos;
	};

	// 解析時パラメータ
	// Parse関数にポインタで渡す
	// 基本的にこのインスタンスは自動領域に取る(_parseは再帰するので)
	class Args {
	private :
		Parser* m_parser;
	public :
		vector<JumpInfo> Continue; // continue文発見時のバイト位置
		vector<JumpInfo> Break; // break文発見時のバイト位置
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

	// パーサー解析基底
	// パーサーのコントロール全般を取り扱う機能を継承先に提供する
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
		 * ジャンプ先アドレスにWriterの現在位置を設定
		 * @param pos ... ジャンプ命令の移動先アドレス設定地点(s32型)
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
		 * 新しいバイナリライタを生成して古いやつを返す
		 */
		CBinaryWriter CreateNewWriter(){
			CBinaryWriter result = this->m_parser->m_writer;
			this->m_parser->m_writer = CBinaryWriter( new BinaryWriter() );
			return result;
		}
		/*
		 * ライターを貰って一歩前の状態を返す
		 */
		CBinaryWriter SetWriter( CBinaryWriter newObj ){
			CBinaryWriter result = this->m_parser->m_writer;
			this->m_parser->m_writer = newObj;
			return result;
		}
		/*
		 * ライターオブジェクトを結合させる
		 */
		void AppendWriter( CBinaryWriter src ){
			this->m_parser->m_writer->append( *src );
		}

		/*
		 * 現在のトークンに対応した名前の型情報を取得
		 */
		Type* getType(){
			Type* t = (Type*)this->m_parser->m_currentScope->findScopeFromTop( this->getTokenString() );
			return t;
		}

		/* 
		 * アセンブリの登録
		 * (ポインタ渡しにしたほうがコピーが発生しないので早い？かもしれないのでそのうち直すかも)
		 */
		void EntryAssembly( AsmInfo* funcAssembly ){ 
			this->m_parser->m_asm->entryAssembly( funcAssembly );
		}

		int GetFunctionAddres( string& funcName ){
			return this->m_parser->m_asm->find( funcName );
		}
		/*
		 * 指定の名前のアセンブリを取得する
		 */
		AsmInfo* GetAssembly( string funcName ){
			return this->m_parser->m_asm->indexAt( GetFunctionAddres( funcName ) );
		}

		/*
		 * 現在のスコープから割り当てられる変数の領域を判定する。 
		 * ローカル領域なのか静的領域なのかを現在のスコープから判定
		 */
		ESymbolType GetVariableLocationInCurrentScope(){
			ESymbolType scopeSymbolType = VariableLocal;
			if( this->m_parser->m_currentScope->ScopeLevel() == 0 ){
				scopeSymbolType = VariableGlobal;
			}
			return scopeSymbolType;
		}
		/*
		 * 関数解析準備
		 */
		MethodInfo* GoToFunction( const string& funcName ){
			this->m_parser->m_writer = CBinaryWriter( new BinaryWriter() );
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToFunctionScope( funcName );
			return reinterpret_cast<MethodInfo*>( this->m_parser->m_currentScope );
		}
		/*
		 * 構造体スコープへ移動
		 */
		Type* GoToStruct( const string& typeName ){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToStructScope( typeName );
			return reinterpret_cast<Type*>( this->m_parser->m_currentScope );
		}
		/*
		 * チャンクスコープへ移動
		 */
		Scope* GoToChunk(){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->goToChildScope( "" );
			return this->m_parser->m_currentScope;
		}

		/*
		 * 一つ上のスコープに戻る
		 */
		Scope* GoToBack(){
			this->m_parser->m_currentScope = this->m_parser->m_currentScope->backToChildScope();
			return this->m_parser->m_currentScope;
		}
		/*
		 * バイナリライターのストリームを取得
		 */
		CStream GetCurrentStream(){
			return this->m_parser->m_writer->getStream();
		}
		/*
		 * 次のトークンが指定のものでない場合エラーと見なして2059番エラーを投げる。
		 * 指定のものである場合は次に進める
		 */
		void ErrorCheckNextToken( int tokenType ){
			if( !this->NextTokenIf( tokenType ) ){
				this->Next();
				throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
			}
			this->Next();
		}
		/*
		 * 現在のトークンが指定のものでない場合エラーと見なして2059番エラーを投げる。
		 */
		void ErrorCheckToken( int tokenType ){
			if( !this->TokenIf( tokenType ) ){
				throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
			}
		}

		/*
		 * 解析処理
		 */
		void Parse( Args* args ){
			this->m_parser->parse( args );
			this->Back();
		}
		/*
		 * 解析処理
		 */
		void Parse(){
			this->Parse( NULL );
		}

		/*
		 * 指定のトークンが見つかるまで解析を進める
		 * 見つからない場合はエラーを返す
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
		 * 指定のトークンが見つかるまで解析を進める
		 */
		void ParseWhile( int tokenType ){
			this->ParseWhile( tokenType , NULL );
		}

		/*
		 * 現在のトークンからシンボルを取得する。
		 */
		SymbolInfo* getSymbol(){
			const string& symbolName = m_parser->getToken().text;
			return m_parser->m_currentScope->getSymbol( symbolName );	
		}
		/*
		 * シンボル登録
		 */
		SymbolInfo* addSymbol( string& symbolName ){
			return m_parser->m_currentScope->addSymbol( symbolName );
		}
		/*
		 * データタイプかスコープかのどちらかからシンボル取得
		 */
		SymbolInfo* getSymbolInScopeOrType( Type* t , const string& symbolName ){
			if( t ){
				return t->getSymbol( symbolName );
			}
			return m_parser->m_currentScope->getSymbol( symbolName );
		}
		/*
		 * 現在のトークンからシンボル存在するか確認
		 */
		bool ExistSymbol(){
			const string& symbolName = m_parser->getToken().text;
			SymbolInfo* symbol = m_parser->m_currentScope->getSymbol( symbolName );
			if( symbol ) return true;
			return false;
		}
	};

	// 変数シンボル解析
	class parse_variable : public interpreter {
	public :
		parse_variable( Parser* parser );
	};
	// 構造体解析
	class parse_struct : public interpreter {
	public : 
		parse_struct( Parser* parser );
	};
	// 関数解析
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
		// 構造体スコープであるかどうか
		// 親スコープを見て判断する
		bool isStructScope(){
			Scope* parent = this->m_parser->m_currentScope->getParentScope();
			if( !parent ){
				return false;
			}
			return parent->isStructScope();
		}
		// 現在のスコープが構造体メンバ関数スコープであることを通知する
		void NotifyStructMethodScope(){
			this->m_parser->m_currentScope->notifyStructMethodScope();
		}
		parse_function( Parser* parser );
		/*
		 * 関数の予約登録
		 */
		void TransactFunction();
		/*
		 * 完了したら確定申告する。
		 */
		void CommitFunction();
		void This();
	};
	// as演算子解析
	class parse_as : public interpreter {
	public :
		parse_as( Parser* parser , varinfo& var );
		void checkArray( varinfo& var );
	};
	// 配列解析
	class parse_array : public interpreter {
	public :
		parse_array( Parser* parser , SymbolInfo* symbol );
	};
	// チャンク解析
	class parse_chunk : public interpreter {
	public :
		parse_chunk( Parser* parser , Args* args );
	};
	// return文
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

	// 式評価基底
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
	// 式評価処理基底
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
	// 評価0 
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

	// 評価1
	// ||
	class expression1 : public expression_base {
	public :
		expression1( expression* exp , Parser* parser );
	};

	// 評価2
	// &&
	class expression2 : public expression_base {
	public :
		expression2( expression* exp , Parser* parser );
	};

	// 評価3
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

	// 評価3
	// +
	// -
	class expression4 : public expression_base {
	public :
		expression4( expression* exp , Parser* parser );
	};

	// 評価4
	// *
	// /
	// %
	class expression5 : public expression_base {
	public :
		expression5( expression* exp , Parser* parser );
	};

	// 評価5
	// symbol
	class expression6 : public expression_base {
	public :
		expression6( expression* exp , Parser* parser );
	};

	// 変数評価
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

	// []評価
	class expression_bracket : public expression_base {
	public :
		expression_bracket( expression* exp , Parser* parser , Type* type , var_chain& v );
	};

	// 関数評価
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

