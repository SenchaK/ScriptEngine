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
		Symbol        , 
		LiteralValue  , 
		LiteralString ,
	};
private :
	Type m_typeId;
	SymbolInfo* m_symbol;
	string m_literal_string;
	double m_literal_value;
public :
	const Type& GetType(){
		return this->m_typeId;
	}
	double ToDouble(){
		return this->m_literal_value;
	}
	EXP_DATA( SymbolInfo* symbol ){
		this->m_typeId = Symbol;
		this->m_symbol = symbol;
		this->m_literal_string = "";
		this->m_literal_value = 0;
	}
	EXP_DATA( double literal_value ){
		this->m_typeId = LiteralValue;
		this->m_symbol = NULL;
		this->m_literal_string = "";
		this->m_literal_value = literal_value;
	}
	EXP_DATA( string literal_string ){
		this->m_typeId = LiteralString;
		this->m_symbol = NULL;
		this->m_literal_string = literal_string;
		this->m_literal_value = 0;
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
		 * 存在しない場合は登録する
		 */
		SymbolInfo* getSymbol(){
			const string& symbolName = m_parser->getToken().text;
			SymbolInfo* symbol = m_parser->m_currentScope->getSymbol( symbolName );
			if( !symbol ){
				symbol = m_parser->m_currentScope->addSymbol( symbolName );
			}
			return symbol;
		}
	};

	class variable : public interpreter {
	public :
		variable( Parser* parser );
	};

	class as : public interpreter {
	public :
		as( Parser* parser , SymbolInfo* symbol );
	};

	class expression : public interpreter {
	private :
		expression* prev;
		stack<EXP_DATA> m_operationStack;
		int R;
	public :
		void ExprPushData( const double& literal_value ){
			m_operationStack.push( EXP_DATA( literal_value ) );
		}
		void ExprPushData( const string& literal_string ){
			m_operationStack.push( EXP_DATA( literal_string ) );
		}
		void ExprPushData( SymbolInfo* const symbolInfo ){
			m_operationStack.push( EXP_DATA( symbolInfo ) );
		}
		void Assign( EXP_DATA& src1 ){
			this->WriteData( src1 );
			this->WritePopR();
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
			}
		}
		void MovR( EXP_DATA& src ){
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
		void WriteMovR( EXP_DATA& src ){
			this->MovR( src );
		}
		expression( Parser* parser , SymbolInfo* symbol );
		expression( expression* prev , Parser* parser , SymbolInfo* symbol );
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
	};

	class expression0 : public expression_base {
	public :
		expression0( expression* exp , Parser* parser , SymbolInfo* symbol );
	};
	class expression1 : public expression_base {
	public :
		expression1( expression* exp , Parser* parser , SymbolInfo* symbol );
	};
	class expression2 : public expression_base {
	public :
		expression2( expression* exp , Parser* parser , SymbolInfo* symbol );
	};
	class expression3 : public expression_base {
	public :
		expression3( expression* exp , Parser* parser , SymbolInfo* symbol );
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

