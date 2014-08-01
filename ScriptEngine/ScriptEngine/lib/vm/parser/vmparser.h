#pragma once
#include "vmscope.h"
#include "..\lexer\vmlexer.h"
#include "..\symbol\vmsymbol.h"
#include "..\assembly\vmassembly_info.h"
#include <stack>

namespace SenchaVM {
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


// 計算機スタック
// 変数、関数などのシンボルはm_symbol  ,
// 演算子なら                m_operator,
// リテラルの場合            m_literal ,
// にそれぞれの対応値を入れる
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


class EXP_DATA {
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
	
public :
	std::stack<int> m_Addres;
	const Type& GetType(){
		return this->m_typeId;
	}
	double ToDouble(){
		return this->m_literal_value;
	}
	EXP_DATA(){
		this->Set( None , NULL , 0 , 0 , "" , false );
	}
	EXP_DATA( SymbolInfo* symbol ){
		this->Set( symbol );
	}
	EXP_DATA( SymbolInfo* symbol , int index ){
		this->Set( Symbol , symbol , index , 0 , "" , true );
	}
	EXP_DATA( double literal_value ){
		this->Set( LiteralValue , NULL , 0 , literal_value , "" , false );
	}
	EXP_DATA( string literal_string ){
		this->Set( LiteralString , NULL , 0 , 0 , literal_string , false );
	}
	void CopyAddresStack( const EXP_DATA& src ){
		m_Addres = src.m_Addres;
	}
	void Set( SymbolInfo* symbol ){
		this->Set( Symbol , symbol , 0 , 0 , "" , false );
	}
	void Set( Type typeId , SymbolInfo* symbol , int index , double literal_value , string literal_string , bool isArray ){
		this->m_typeId = typeId;
		this->m_symbol = symbol;
		this->m_literal_string = literal_string;
		this->m_literal_value = literal_value;
		this->m_arrayIndex = index;
		this->m_isArray = isArray;
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
	void PushAddres( int addres ){
		m_Addres.push( addres );
	}
	int PopAddres(){
		int result = m_Addres.top();
		m_Addres.pop();
		return result;
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
};

// ************************************************
// 構文解析器
// ************************************************
class Parser {
	friend class interpreter;
private :
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


		/*
		 * 現在のトークンからシンボルを取得する。
		 */
		SymbolInfo* getSymbol(){
			const string& symbolName = m_parser->getToken().text;
			return m_parser->m_currentScope->getSymbol( symbolName );	
		}

		SymbolInfo* addSymbol( string& symbolName ){
			std::cout << "シンボル:" << symbolName << "登録" << std::endl;
			return m_parser->m_currentScope->addSymbol( symbolName );
		}

		/*
		 * スコープかシンボルの子階層に存在するシンボルのどちらかを取得
		 * 先に親シンボルを探す
		 */
		SymbolInfo* getSymbolInScopeOrParentSymbol( SymbolInfo* parentSymbol , const string& symbolName ){
			if( parentSymbol ){
				return parentSymbol->getSymbol( symbolName );
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

		/*
		 * あるシンボルの子階層にそのシンボルが存在するか
		 * 存在する場合、メンバーとして扱う
		 */
		bool ExistSymbolMember( string& inst , string& member ){
			SymbolInfo* instSymbol = m_parser->m_currentScope->getSymbol( inst );
			assert( instSymbol );
			SymbolInfo* memberSymbol = instSymbol->getSymbol( member );
			if( memberSymbol ) return true;
			return false;
		}
	};

	class variable : public interpreter {
	public :
		variable( Parser* parser );
	};

	class left_expression : public interpreter {
	public :
		left_expression( Parser* parser );
	};

	class as : public interpreter {
	public :
		as( Parser* parser , EXP_DATA& var );
	};

	class expression : public interpreter {
	public :
		int R;
		void ExprPushData( const double& literal_value ){
			EXP_DATA exp_data( literal_value );
			MovR( exp_data );
		}
		void ExprPushData( const string& literal_string ){
			EXP_DATA exp_data( literal_string );
			MovR( exp_data );
		}
		void ExprPushData( SymbolInfo* const symbolInfo ){
			EXP_DATA exp_data( symbolInfo );
			MovR( exp_data );
		}
		void ExprPushData( SymbolInfo* const symbolInfo , int index ){
			EXP_DATA exp_data( symbolInfo , index );
			MovR( exp_data );
		}
		void ExprPushData( EXP_DATA& exp_data ){
			MovR( exp_data );
		}
		void Assign( EXP_DATA& src ){
			this->WriteData( src );
			this->WritePopR();
			switch( (EXP_DATA::Type)src ){
			case EXP_DATA::LiteralValue : printf( "mov %0.2f,R[%d]\n" , (double)src , R ); break;
			case EXP_DATA::LiteralString : printf( "mov %s,R[%d]\n" , ((string)src).c_str() , R ); break;
			case EXP_DATA::Symbol : 
				printf( "mov " );
				printf( "%s[%d]" , ((SymbolInfo*)src)->Name().c_str() , ((SymbolInfo*)src)->Addr() );
				while( src.m_Addres.size() > 0 ){
					printf( "+%d" , src.PopAddres() );
				}
				if( src.IsArray() ){
					printf( "+R%d" , src.Index() );
				}
				printf( ",R[%d]\n" , R );
				break;
			}
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
			case TokenType::Add : printf( "add R[%d] , R[%d]\n" , R - 2 , R - 1 ); break;
			case TokenType::Sub : printf( "sub R[%d] , R[%d]\n" , R - 2 , R - 1 ); break;
			case TokenType::Mul : printf( "mul R[%d] , R[%d]\n" , R - 2 , R - 1 ); break;
			case TokenType::Div : printf( "div R[%d] , R[%d]\n" , R - 2 , R - 1 ); break;
			case TokenType::Rem : printf( "rem R[%d] , R[%d]\n" , R - 2 , R - 1 ); break;
			}
			this->WriteR( -2 );
			this->WriteR( -1 );
			R -= 1;
		}

		void WriteData( EXP_DATA& src ){
			switch( (EXP_DATA::Type)src ){
			case EXP_DATA::LiteralValue :
				this->m_parser->m_writer->write( EMnemonic::LIT_VALUE );
				this->m_parser->m_writer->writeDouble( (double)src );
				break;
			case EXP_DATA::LiteralString : 
				this->m_parser->m_writer->write( EMnemonic::LIT_STRING );
				this->m_parser->m_writer->writeString( (string)src );
				break;
			case EXP_DATA::Symbol :	
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
		void MovR( EXP_DATA& src ){
			switch( (EXP_DATA::Type)src ){
			case EXP_DATA::LiteralValue : printf( "mov R[%d],%0.2f\n" , R , (double)src ); break;
			case EXP_DATA::LiteralString : printf( "mov R[%d],%s\n" , R , ((string)src).c_str() ); break;
			case EXP_DATA::Symbol : 
				printf( "mov " );
				printf( "R[%d]," , R );
				printf( "%s[%d]" , ((SymbolInfo*)src)->Name().c_str() , ((SymbolInfo*)src)->Addr() );
				if( src.IsArray() ){
					printf( "+R%d" , src.Index() );
				}
				printf( "\n");
				break;
			}

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
		void WriteMovR( EXP_DATA& src ){
			this->MovR( src );
		}
		expression( Parser* parser , EXP_DATA& v );
	};


	class expression_base : public interpreter {
	private :
		expression* m_exp;
	public :
		expression_base( expression* exp , Parser* parser ) : interpreter( parser ){
			m_exp = exp;
		}
		void ExprPushData( const double& literal_value ){ m_exp->ExprPushData( literal_value ); }
		void ExprPushData( const string& literal_string ){ m_exp->ExprPushData( literal_string ); }
		void ExprPushData( SymbolInfo* const symbolInfo ){ m_exp->ExprPushData( symbolInfo ); }
		void ExprPushData( SymbolInfo* const symbolInfo , int index ){ m_exp->ExprPushData( symbolInfo , index ); }
	};

	class expression0 : public expression_base {
	public :
		expression0( expression* exp , Parser* parser , EXP_DATA& v );
	};
	class expression1 : public expression_base {
	public :
		expression1( expression* exp , Parser* parser , EXP_DATA& v );
	};
	class expression2 : public expression_base {
	public :
		expression2( expression* exp , Parser* parser , EXP_DATA& v );
	};
	class expression3 : public expression_base {
	public :
		expression3( expression* exp , Parser* parser , EXP_DATA& v );
	};
	class expression_variable : public expression_base {
	private :
		expression* expr;
		EXP_DATA var;
		SymbolInfo* parentSymbol;
	public :
		expression_variable( expression* exp , Parser* parser , EXP_DATA& var );
		expression_variable( expression* exp , Parser* parser , EXP_DATA& var , SymbolInfo* instSymbol );
	private :
		void exp();
		void bracket( const string& symbolName );
		void dot( const string& symbolName );
		void memberFunc( const string& symbolName );
		bool isExistSymbolInParent( const string& symbolName ){
			assert( this->parentSymbol );
			return this->parentSymbol->getSymbol( symbolName ) ? true : false;
		}
	};

	class expression_bracket : public expression_base {
	public :
		expression_bracket( expression* exp , Parser* parser , SymbolInfo* parent , EXP_DATA& v );
	};


	class parse_array : public interpreter {
	public :
		parse_array( Parser* parser , SymbolInfo* symbol );
	};
private :
	// ジャンプ命令情報
	// continue文/break文を使用するときにどこでその命令を見つけたのか、
	// という内部コードアドレスを記録する
	struct JumpInfo {
		int codeAddr;
	};

	// 解析時パラメータ
	// _parse関数にポインタで渡す
	// 基本的にこのインスタンスは自動領域に取る(_parseは再帰するので)
	struct ParseParameter {
		vector<JumpInfo> continueAddr ; // continue文発見時のバイト位置
		vector<JumpInfo> breakAddr    ; // break文発見時のバイト位置
		size_t args;
		ParseParameter(){
			args = 0;
		}
	};

	// 
	struct ExpressionParameter {
		enum SymbolAfterDotSyntax {
			Variable , 
			Function ,
		};
		bool isReferenceSymbol;    // 左辺のシンボルが参照型である
		int prog_counter;          // プログラムカウンタ
		stack<SymbolInfo*> symbol; //
		int bracketCount;          // '['～']'発見回数
		int pushCount;             // 関数パラメータ渡しpush回数

		SymbolAfterDotSyntax symbolAfterDotSyntax;
		ExpressionParameter( int pc ){
			this->prog_counter = pc;
			this->isReferenceSymbol = false;
			this->symbolAfterDotSyntax = Variable;
			this->bracketCount = 0;
			this->pushCount = 0;
		}
		ExpressionParameter( int pc , ExpressionParameter* param , bool is_reference_symbol ){
			int bracket_count = 0;
			int push_count = 0;
			if( param ){
				pc += param->prog_counter;
				bracket_count += param->bracketCount;
				push_count += param->pushCount;
			}
			this->prog_counter = pc;
			this->isReferenceSymbol = is_reference_symbol;
			this->symbolAfterDotSyntax = Variable;
			this->bracketCount = bracket_count;
			this->pushCount = push_count;
		}
	};

	vector<TOKEN> m_tokens;
	stack<OperationStack> m_operationStack;
	VMAssembleCollection m_assemblyCollection;
	CBinaryWriter m_writer;
	CScope m_scope;
	Scope* m_currentScope;
	size_t m_pos;
	int m_R;
public :
	const VMAssembleCollection& getResult(){ return m_assemblyCollection; }
	Parser( vector<TOKEN> tokens );
	~Parser();
private :
	void _execute();
	void _initialize( vector<TOKEN> tokens );
private :
	// トークン処理など
	const TOKEN& backToken();
	const TOKEN& nextToken();
	const TOKEN& getToken();
	const TOKEN& getToken(int ofs);
	bool hasNext();
	void _consume( int consumeCount );
private :
	void _entryFunction( string funcName , size_t args );
	void _entryClass( string className );
private :
	// シンボルを計算スタックに積む
	void _pushOperation( OperationStack item );
	OperationStack _popOperation();
private :
	// ステートメントなど
	void _parse( ParseParameter* param );
	void _parse_variable( ParseParameter* param );
	void _parse_letter( ParseParameter* param );
	void _parse_struct( ParseParameter* param );
	void _parse_function( ParseParameter* param );
	void _parse_if( ParseParameter* param );
	void _parse_switch( ParseParameter* param );
	void _parse_for( ParseParameter* param );
	void _parse_while( ParseParameter* param );
	void _parse_chunk( ParseParameter* param );
	void _parse_continue( ParseParameter* param );
	void _parse_break( ParseParameter* param );
	void _parse_struct( SymbolInfo* structSymbol );
	void _parse_as( SymbolInfo* symbol );
	void _parse_return( ParseParameter* param );
private :
	// 計算式系
	// 以下優先度順になっている
	void _expression( ParseParameter* param );
	void _exp( ExpressionParameter* const param );
	void _assign_expression( ExpressionParameter* const param );      // = += -= *= /= %=
	void _logical_or_expression( ExpressionParameter* const param );  // ||
	void _logical_and_expression( ExpressionParameter* const param ); // &&
	void _equality_expression( ExpressionParameter* const param );    // == !=
	void _relational_expression( ExpressionParameter* const param );  // >= <= > <
	void _add_sub_expression( ExpressionParameter* const param );     // +-
	void _mul_div_expression( ExpressionParameter* const param );     // */%
	void _token_expression( ExpressionParameter* const param );       // [A-Za-z_][A-Za-z_0-9]*

	void _method_expression( Scope* funcScope , ExpressionParameter* const param );
	void _func_expression( ExpressionParameter* const param );        // [A-Za-z_][A-Za-z_0-9]*(
	void _unary_expression( ExpressionParameter* const param );       // !-
	void _primary_expression( ExpressionParameter* const param );     // []().
	void _after_inc_expression( ExpressionParameter* const param );   // ++
private :
	void _process_letter_expression( ExpressionParameter* const symbolParam , const string& letter );
	void _process_symbol_expression( ExpressionParameter* const symbolParam , ExpressionParameter* const param , SymbolInfo* symbol );
private :
	SymbolInfo* _findOperationSymbol( SymbolInfo* symbol );
	void _popOperation( const TOKEN& opeType );
	void _pushLITERAL( const TOKEN& token );
	void _pushLITERAL_String( const TOKEN& token );
	void _pushSYMBOL( SymbolInfo* symbol , bool isLeftExpression , bool isReferenceExpression );
	void _popAssignOperation( const TOKEN& opeType , SymbolInfo* symbol );
	void _popCompareOperation( const TOKEN& opeType );
	void _popLogicalOperation( const TOKEN& opeType );
	void _popControlFlowResult();
	void _popAfterIncOperation( const TOKEN& opeType );
	void _popPrevIncOperation( const TOKEN& opeType );
	void _popArrayIndexOperation( SymbolInfo* arraySymbol , ExpressionParameter* param );
	void _popSYMBOL();
private :
	Scope* _findMethodScope( string& scopeName , ExpressionParameter* const param );
	void _skipParen();
	bool _isVariable( const TOKEN& _0 , const TOKEN& _1 ){
		if( _0.type == TokenType::VariableSymbol && _1.type == TokenType::Letter ) return true;
		if( _0.type == TokenType::RefSymbol && _1.type == TokenType::Letter )      return true;
		return false;
	}
};
typedef std::shared_ptr<Parser> CParser;

} // namespace Assembly
} // namespace SenchaVM

