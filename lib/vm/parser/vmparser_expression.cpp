#define EXPRESSION_MASTER_DEBUG (0)
#define EXPRESSION_UNIT_DEBUG   (1)
#define EXPRESSION_DEBUG        (0)

#define R m_R

#ifdef EXPRESSION_DEBUG
	#if EXPRESSION_DEBUG == EXPRESSION_MASTER_DEBUG
		#define EXPRESSION_LOG VM_PRINT
		#define EXPRESSION_ASSERT VM_ASSERT
	#elif EXPRESSION_DEBUG == EXPRESSION_UNIT_DEBUG
		#define EXPRESSION_LOG printf
		#define EXPRESSION_ASSERT assert
	#endif
#else
	#define EXPRESSION_LOG(...)
	#define EXPRESSION_ASSERT(...)
#endif

#include "vmparser.h"
#include "vmscope.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"

using namespace std;
namespace SenchaVM {
namespace Assembly {

static inline void __WRITE_SYMBOL( Sencha::Util::CBinaryWriter w , SymbolInfo* symbol ){
	w->write( symbol->toAssembleCode() );
	w->write( symbol->isArray() );
	w->write( symbol->isReferenceMember() );
	w->write( symbol->IsReference() );
	if( symbol->isReferenceMember() ){
		EXPRESSION_LOG( "this[%d]->[%d]\n" , symbol->TopSymbolAddr() , symbol->Addr() );
		w->writeInt32( symbol->TopSymbolAddr() );
	}
	w->writeInt32( symbol->Addr() );
	w->writeString( symbol->Name() ); // TODO:: 本番環境では消す
}
static inline void __WRITE_R( Sencha::Util::CBinaryWriter w , int R_index ){
	EXPRESSION_ASSERT( R_index >= 0 );
	w->write( EMnemonic::REG );
	w->writeInt32( R_index );
}
static inline void __WRITE_LITERAL_VALUE( Sencha::Util::CBinaryWriter w , string value ){
	w->write( EMnemonic::LIT_VALUE );
	w->writeDouble( atof( value.c_str() ) );
}
static inline void __WRITE_LITERAL_STRING( Sencha::Util::CBinaryWriter w , string value ){
	w->write( EMnemonic::LIT_STRING );
	w->writeString( value.c_str() );
}


// ***************************************************************************************************************
static inline bool IsMul_Div_Rem( const TOKEN& token ){
	return ( token.type == TokenType::Mul || token.type == TokenType::Div || token.type == TokenType::Rem );
}
static inline bool IsAdd_Sub( const TOKEN& token ){
	return ( token.type == TokenType::Add || token.type == TokenType::Sub );
}
static inline bool IsAssign( const TOKEN& token ){
	if( token.type == TokenType::Assign )    return true;
	if( token.type == TokenType::AddAssign ) return true;
	if( token.type == TokenType::SubAssign ) return true;
	if( token.type == TokenType::MulAssign ) return true;
	if( token.type == TokenType::DivAssign ) return true;
	if( token.type == TokenType::RemAssign ) return true;
	return false;
}

static inline void PrintOperator( const TOKEN& token ){
	switch( token.type ){
	case TokenType::Add :
		EXPRESSION_LOG( "加算\n" );
		break;
	case TokenType::Sub :
		EXPRESSION_LOG( "減算\n" );
		break;
	case TokenType::Mul :
		EXPRESSION_LOG( "掛け算\n" );
		break;
	case TokenType::Div :
		EXPRESSION_LOG( "割り算\n" );
		break;
	case TokenType::Rem :
		EXPRESSION_LOG( "余り算\n" );
		break;
	}
}
void Parser::_pushLITERAL( const TOKEN& token ){
	EXPRESSION_LOG( "INT_LITERAL = : R[%d] ,[token %s]" , R , token.text.c_str() );
	// CMD
	m_writer->write( EMnemonic::Mov );
	// SRC
	__WRITE_R( m_writer , R );
	// DST
	__WRITE_LITERAL_VALUE( m_writer , token.text );
	R++;
	EXPRESSION_LOG( "[WC %d]\n" , m_writer->count() );
}

void Parser::_pushLITERAL_String( const TOKEN& token ){
	EXPRESSION_LOG( " = : R[%d] , %s\n" , R , token.text.c_str() );
	// CMD
	m_writer->write( EMnemonic::Mov );
	// SRC
	__WRITE_R( m_writer , R );
	// DST
	__WRITE_LITERAL_STRING( m_writer , token.text );

	R++; 
}

// シンボル追加
// Rスタックにシンボルの値をセットする命令を書き込む
// 左辺式の場合Rへの書き込みは行わない
void Parser::_pushSYMBOL( SymbolInfo* symbol , bool isLeftExpression , bool isRefExpression ){
	if( isLeftExpression ){
		R++;
		return;
	}

	EXPRESSION_LOG( "pushSYMBOL = : R[%d] , %s[%d]\n" , R , symbol->Name().c_str() , symbol->Addr() );
	// CMD
	if( isRefExpression ) m_writer->write( EMnemonic::PMov );
	else                  m_writer->write( EMnemonic::Mov  );

	// SRC
	__WRITE_R( m_writer , R );
	// DST
	__WRITE_SYMBOL( m_writer , symbol );
	R++;
}
void Parser::_popOperation( const TOKEN& opeType ){
	int Dest = R - 1;
	int Src  = R - 2;
	R -= 1;
	switch( opeType.type ){
		case TokenType::Add : EXPRESSION_LOG( " + : R[%d] , R[%d]\n" , Src , Dest ); break;
		case TokenType::Sub : EXPRESSION_LOG( " - : R[%d] , R[%d]\n" , Src , Dest ); break;
		case TokenType::Mul : EXPRESSION_LOG( " * : R[%d] , R[%d]\n" , Src , Dest ); break;
		case TokenType::Div : EXPRESSION_LOG( " / : R[%d] , R[%d]\n" , Src , Dest ); break;
		case TokenType::Rem : EXPRESSION_LOG( " % : R[%d] , R[%d]\n" , Src , Dest ); break;
	}
	// CMD
	switch( opeType.type ){
		case TokenType::Add : m_writer->write( EMnemonic::Add ); break;
		case TokenType::Sub : m_writer->write( EMnemonic::Sub ); break;
		case TokenType::Mul : m_writer->write( EMnemonic::Mul ); break;
		case TokenType::Div : m_writer->write( EMnemonic::Div ); break;
		case TokenType::Rem : m_writer->write( EMnemonic::Rem ); break;
	}
	// SRC
	__WRITE_R( m_writer , Src );
	// DST
	__WRITE_R( m_writer , Dest );
}


// 計算スタックに積まれたシンボルを検索する
// 子が存在する場合、その下の階層を調べる
SymbolInfo* Parser::_findOperationSymbol( SymbolInfo* symbol ){
	//printf( "Find %s , %d\n" , symbol->Name().c_str() , symbol->ChildSymbolCount() );
	if( m_operationStack.size() == 0 ){
		return symbol;
	}
	string peekLetter = m_operationStack.top().Literal();
	SymbolInfo* existSymbol = symbol->getSymbol( peekLetter );	
	if( !existSymbol ){
		return symbol;
	}
	if( symbol->ChildSymbolCount() == 0 ){
		return symbol;
	}

	OperationStack child = _popOperation();

	//printf( "**child %s \n" , child.Literal().c_str() );
	symbol = symbol->getSymbol( child.Literal().c_str() );
	if( !symbol ){
		printf( "Error:シンボル発見できず \"%s\"\n" , child.Literal().c_str() );
	}
	EXPRESSION_ASSERT( symbol );
	return _findOperationSymbol( symbol );
}

void Parser::_popAssignOperation( const TOKEN& opeType , SymbolInfo* symbol ){
	EXPRESSION_LOG( "AssignOperation " );
	EXPRESSION_ASSERT( symbol );

	R -= 1;

	switch( opeType.type ){
		case TokenType::AddAssign : EXPRESSION_LOG( "+= : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
		case TokenType::SubAssign : EXPRESSION_LOG( "-= : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
		case TokenType::MulAssign : EXPRESSION_LOG( "*= : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
		case TokenType::DivAssign : EXPRESSION_LOG( "/= : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
		case TokenType::RemAssign : EXPRESSION_LOG( "%= : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
		case TokenType::Assign    : EXPRESSION_LOG( " = : %s , R[%d]\n" , symbol->Name().c_str() , R ); break;
	}
	if( symbol->IsReference() ){
		EXPRESSION_ASSERT( opeType.type == TokenType::Assign );
	}

	// CMD
	switch( opeType.type ){
		case TokenType::AddAssign : m_writer->write( EMnemonic::Add ); break;
		case TokenType::SubAssign : m_writer->write( EMnemonic::Sub ); break;
		case TokenType::MulAssign : m_writer->write( EMnemonic::Mul ); break;
		case TokenType::DivAssign : m_writer->write( EMnemonic::Div ); break;
		case TokenType::RemAssign : m_writer->write( EMnemonic::Rem ); break;
		case TokenType::Assign    : 
			if( symbol->IsReference() ){ m_writer->write( EMnemonic::PMov ); }
			else                       { m_writer->write( EMnemonic::Mov ) ; }	
			break;
	}
	
	// SRC
	__WRITE_SYMBOL( m_writer , symbol );

	// DST
	__WRITE_R( m_writer , R );
}
void Parser::_popCompareOperation( const TOKEN& opeType ){
	int CompareDst = R - 1;
	int CompareSrc = R - 2;
	R -= 1;
	switch( opeType.type ){
		case TokenType::GEq     : EXPRESSION_LOG( ">= : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
		case TokenType::Greater : EXPRESSION_LOG( " > : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
		case TokenType::LEq     : EXPRESSION_LOG( "<= : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
		case TokenType::Lesser  : EXPRESSION_LOG( " < : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
		case TokenType::Equal   : EXPRESSION_LOG( "== : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
		case TokenType::NotEqual: EXPRESSION_LOG( "!= : R[%d] , R[%d]\n" , CompareSrc , CompareDst ); break;
	}

	// CMD
	switch( opeType.type ){
		case TokenType::GEq     : m_writer->write( EMnemonic::CmpGeq ); break;
		case TokenType::Greater : m_writer->write( EMnemonic::CmpG   ); break;
		case TokenType::LEq     : m_writer->write( EMnemonic::CmpLeq ); break;
		case TokenType::Lesser  : m_writer->write( EMnemonic::CmpL   ); break;
		case TokenType::Equal   : m_writer->write( EMnemonic::CmpEq  ); break;
		case TokenType::NotEqual: m_writer->write( EMnemonic::CmpNEq ); break;
	}	
	__WRITE_R( m_writer , CompareSrc );	// SRC
	__WRITE_R( m_writer , CompareDst ); // DST
}
void Parser::_popLogicalOperation( const TOKEN& opeType ){
	int LogicDst = R - 1;
	int LogicSrc = R - 2;
	R -= 1;
	switch( opeType.type ){
		case TokenType::LogicalOr  : EXPRESSION_LOG( "Logical R[%d] || R[%d]\n" , LogicSrc , LogicDst ); break;
		case TokenType::LogicalAnd : EXPRESSION_LOG( "Logical R[%d] && R[%d]\n" , LogicSrc , LogicDst ); break;
	}
	// CMD
	switch( opeType.type ){
		case TokenType::LogicalOr  : m_writer->write( EMnemonic::LogOr  ); break;
		case TokenType::LogicalAnd : m_writer->write( EMnemonic::LogAnd ); break;
	}

	__WRITE_R( m_writer , LogicSrc ); // SRC
	__WRITE_R( m_writer , LogicDst ); // DST
}

// インクリメント・デクリメント評価
// スタックに直前、もしくは直後のシンボルが詰んであることが前提となっている。
// 一旦シンボルを取り出し、計算後に再度スタックに詰みなおす
void Parser::_popAfterIncOperation( const TOKEN& opeType ){
	OperationStack peekSymbol = _popOperation();
	R -= 1;

	SymbolInfo* symbol = peekSymbol.Symbol();
	EXPRESSION_ASSERT( symbol && "[後置インクリメント/デクリメント] [シンボルが不正です]" );

	switch( opeType.type ){
		case TokenType::Inc : EXPRESSION_LOG( "++ : %s\n" , symbol->Name().c_str() ); break;
		case TokenType::Dec : EXPRESSION_LOG( "-- : %s\n" , symbol->Name().c_str() ); break;
	}
	_pushSYMBOL( symbol , false , false );
	_pushOperation( OperationStack( symbol ) );
	if( opeType.type == TokenType::Inc ){ m_writer->write( EMnemonic::Inc ); }
	if( opeType.type == TokenType::Dec ){ m_writer->write( EMnemonic::Dec ); }
	__WRITE_SYMBOL( m_writer , symbol );
}


void Parser::_popPrevIncOperation( const TOKEN& opeType ){
	OperationStack peekSymbol = _popOperation();
	R -= 1;

	SymbolInfo* symbol = peekSymbol.Symbol();
	EXPRESSION_ASSERT( symbol && "[前置インクリメント/デクリメント] [シンボルが不正です]" );

	switch( opeType.type ){
		case TokenType::Inc : EXPRESSION_LOG( "++ : %s\n" , symbol->Name().c_str() ); break;
		case TokenType::Dec : EXPRESSION_LOG( "-- : %s\n" , symbol->Name().c_str() ); break;
	}
	if( opeType.type == TokenType::Inc ){ m_writer->write( EMnemonic::Inc ); }
	if( opeType.type == TokenType::Dec ){ m_writer->write( EMnemonic::Dec ); }
	__WRITE_SYMBOL( m_writer , symbol );
	_pushSYMBOL( symbol , false , false );
	_pushOperation( OperationStack( symbol ) );
}


void Parser::_popControlFlowResult(){
	R -= 1;
}

void Parser::_popArrayIndexOperation( SymbolInfo* arraySymbol , ExpressionParameter* param ){
	R--;
	EXPRESSION_ASSERT( arraySymbol );

	m_writer->write( EMnemonic::Mul );               // 1byte
	m_writer->write( EMnemonic::REG );               // 1byte
	m_writer->writeInt32( R );                       // 4byte
	m_writer->write( EMnemonic::LIT_VALUE );         // 1byte
	m_writer->writeDouble( arraySymbol->DataSize() );// 8byte

	// SRC
	if( param->bracketCount > 1 ) m_writer->write( EMnemonic::ArrayIndexAdd ); // 1byte
	else                          m_writer->write( EMnemonic::ArrayIndexSet ); // 1byte

	// DST
	m_writer->write( EMnemonic::REG );               // 1byte
	m_writer->writeInt32( R );                       // 4byte

	EXPRESSION_LOG( "**ArrayIndexSet!! Index = R[%d] [DATA_SIZE %d] [WRITE_COUNT %d]\n" , R , arraySymbol->DataSize() , m_writer->count() );
}
void Parser::_popSYMBOL(){
	R--;
}

// ***************************************************************************************************************


void Parser::_expression( ParseParameter* param ){
	ExpressionParameter expParam( m_writer->count() );
	CBinaryWriter prev = m_writer;
	m_writer = CBinaryWriter( new BinaryWriter() );


	EXPRESSION_LOG( "******Expression\n" );
	TOKEN token = getToken();
	_exp( &expParam );
	token = getToken();
	if( m_operationStack.size() > 0 ){
		_popOperation();
	}
	if( R > 0 ){
		R--;
	}
	if( token.type == TokenType::Comma ){
		nextToken();
		_parse_variable( param );
	}

	//EXPRESSION_LOG( "[OperationStack size \"%d\"][CurrentRegister \"%d\"]\n" , m_operationStack.size() , R );
	EXPRESSION_ASSERT( m_operationStack.size() == 0 );
	EXPRESSION_ASSERT( R == 0 );

	CStream stream = m_writer->getStream();
	while( stream->hasNext() ){ prev->write( stream->getByte() ); }
	m_writer = prev;

	EXPRESSION_LOG( "計算終了 [WC %d][TOKEN %s]\n" , m_writer->count() , getToken(0).text.c_str() );
	EXPRESSION_LOG( "\n\n\n" );
}

void Parser::_exp( ExpressionParameter* const param ){
	_unary_expression( param );
	_assign_expression( param );
#undef __MATCH_TOKEN
}

// =  , += , -= , *= , /= , %=
void Parser::_assign_expression( ExpressionParameter* const param ){
	_logical_or_expression( param );
	const TOKEN& opeType = getToken();
	if( IsAssign( opeType ) ){
		//EXPRESSION_LOG( "**AssignExpression 開始 %d\n" , m_writer->count() );
		ExpressionParameter expParam( m_writer->count() , param , false );
		CBinaryWriter prevSymbolExpression = m_writer;  // "="以前のシンボル評価
		m_writer = CBinaryWriter( new BinaryWriter() ); // "="以降の計算式評価

		OperationStack symbol = m_operationStack.top();
		nextToken();

		
		expParam.isReferenceSymbol = symbol.Symbol()->IsReference();
		_exp( &expParam );

		//EXPRESSION_LOG( "AssignExpression 評価 [WC %d][OP_STK %d]\n" , m_writer->count() , m_operationStack.size() );
				
		CStream stream = prevSymbolExpression->getStream();
		while( stream->hasNext() ){ m_writer->write( stream->getByte() ); }
				
		_popOperation();
		_popAssignOperation( opeType , symbol.Symbol() );
		if( m_operationStack.size() > 1 ){
			R--;
			_pushSYMBOL( symbol.Symbol() , false , false );
		}
	}
}


// *********************************************************************************
// 論理演算 "||"
// 直前の式が真であれば次の式は評価せず、
// 次のAndか計算終了地点が見つかるまで飛ばす必要がある
// *********************************************************************************
void Parser::_logical_or_expression( ExpressionParameter* const param ){
//#define LOGICAL_PRINT(...)
#define LOGICAL_PRINT VM_PRINT
	_logical_and_expression( param );
	TOKEN logical = getToken();
	while( logical.type == TokenType::LogicalOr ){
		EXPRESSION_LOG( "論理演算 : [%s] \n" , logical.text.c_str() );
		// 位置保存
		// 命令書き込み
		int pos = 0;
		m_writer->write( EMnemonic::JumpNotZero );
		pos = m_writer->count();
		m_writer->writeInt32( 0 );

		// 次のトークンへ進め、演算を行う
		nextToken();
		_logical_and_expression( param );
		_popLogicalOperation( logical );

		int program_count = m_writer->count();
		if( param ){
			program_count += param->prog_counter;
		}
		m_writer->writeInt32( program_count , pos );
		LOGICAL_PRINT( "JumpNotZero [pos %d][addr %d]\n" , pos , m_writer->count() );
		logical = getToken();
	}
#undef LOGICAL_PRINT
}

// *********************************************************************************
// 論理演算 "&&"
// 直前の式が真であれば次の式も評価するが、
// 偽である場合は&&の次の式ではなく、次のorか計算終了地点まで飛ばす必要がある
// *********************************************************************************
void Parser::_logical_and_expression( ExpressionParameter* const param ){
//#define LOGICAL_PRINT(...)
#define LOGICAL_PRINT VM_PRINT
//#define LOGICAL_PRINT printf
	_equality_expression( param );
	TOKEN logical = getToken();

	while( logical.type == TokenType::LogicalAnd ){
		LOGICAL_PRINT( ">>> 論理演算AND[&&]\n" , logical.text.c_str() );

		// 位置保存
		// 命令書き込み
		int pos = 0;
		m_writer->write( EMnemonic::JumpZero );
		pos = m_writer->count();
		m_writer->writeInt32( 0 );

		// 次のトークンへ進め、演算を行う
		nextToken();
		_equality_expression( param );
		_popLogicalOperation( logical );

		int program_count = m_writer->count();
		if( param ){
			program_count += param->prog_counter;
		}
		m_writer->writeInt32( program_count , pos );
		LOGICAL_PRINT( "JumpZero [pos %d][addr %d][token %s]\n" , pos , program_count , getToken().text.c_str() );
		logical = getToken();
	}
#undef LOGICAL_PRINT
}

// == , !=
void Parser::_equality_expression( ExpressionParameter* const param ){
	_relational_expression( param );
	TOKEN opeType = getToken();
	if( opeType.type == TokenType::Equal || opeType.type == TokenType::NotEqual ){
		nextToken();
		_relational_expression( param );
		OperationStack value1 = _popOperation();
		OperationStack value2 = _popOperation();
		_popCompareOperation( opeType );
	}
}

// >= , <= , > , <
void Parser::_relational_expression( ExpressionParameter* const param ){
	_add_sub_expression( param );
	TOKEN opeType = getToken();
	if( opeType.type == TokenType::GEq || opeType.type == TokenType::Greater || opeType.type == TokenType::LEq || opeType.type == TokenType::Lesser ){
		nextToken();
		_add_sub_expression( param );

		OperationStack value1 = _popOperation();
		OperationStack value2 = _popOperation();
		_popCompareOperation( opeType );
	}
}

// +,-
void Parser::_add_sub_expression( ExpressionParameter* const param ){
	_mul_div_expression( param );
	TOKEN opeType = getToken();
	while( IsAdd_Sub( opeType ) ){
		nextToken();
		_mul_div_expression( param );
		_popOperation();
		_popOperation( opeType );
		opeType = getToken();
	}
}

// *,/,%
void Parser::_mul_div_expression( ExpressionParameter* const param ){
	_token_expression( param );
	TOKEN opeType = getToken();
	while( IsMul_Div_Rem( opeType ) ){
		nextToken();
		_token_expression( param );
		OperationStack value = _popOperation();
		_popOperation( opeType );
		opeType = getToken();
	}
}

// ************************************************************************************
// シンボル発見後の処理
// 基本的にパラメータ(symbolParam)はNULLでないものが必ず入ってくる
void Parser::_process_symbol_expression( ExpressionParameter* const symbolParam , ExpressionParameter* const param , SymbolInfo* symbol ){
	assert( symbolParam );
	bool isRefExpression = false;
	if( param ){
		isRefExpression = param->isReferenceSymbol;
		param->symbolAfterDotSyntax = symbolParam->symbolAfterDotSyntax;
	}

	switch( symbolParam->symbolAfterDotSyntax ){
	case ExpressionParameter::Variable :
		symbol = _findOperationSymbol( symbol );
		_pushSYMBOL( symbol , IsAssign( getToken() ) , isRefExpression );
		_pushOperation( symbol );
		_after_inc_expression( param );
		break;
	case ExpressionParameter::Function :
		symbolParam->symbol.pop();
		while( symbolParam->symbol.size() > 0 ){
			OperationStack stack = _popOperation();
			symbolParam->symbol.pop();
		}
		break;
	}
}


// ************************************************************************************
// 直前にシンボルが来ている場合（クラスメンバへのアクセス）
// シンボルスタックには1個以上のシンボルが存在している。
// 直前のシンボルからメンバである子シンボルを取得し、それをスタックに積み上げる
void Parser::_process_letter_expression( ExpressionParameter* const symbolParam , const string& letter ){
	if( !symbolParam ){
		return;
	}
	if( symbolParam->symbol.size() > 0 ){
		SymbolInfo* symbol = symbolParam->symbol.top()->getSymbol( letter );
		assert( symbol );
		symbolParam->symbol.push( symbol );
	}
}


// ************************************************************************************
// 演算子ではないトークンがやってきたとき
void Parser::_token_expression( ExpressionParameter* const param ){
	_primary_expression( param ); // 最優先( .[]() )
	_unary_expression( param );   // 単項(-,!)

	const TOKEN& currentToken = getToken();
	switch( currentToken.type ){
	case TokenType::Digit :
		{
			_pushLITERAL( currentToken );
			_pushOperation( OperationStack( currentToken.text ) );
			nextToken();
		}
		break;
	case TokenType::String :
		{
			_pushLITERAL_String( currentToken );
			_pushOperation( OperationStack( currentToken.text ) );
			nextToken();
		}
		break;
	case TokenType::VariableSymbol :
	case TokenType::RefSymbol :
		{
			const TOKEN& symbolName = nextToken();
			SymbolInfo* symbol = m_currentScope->getSymbol( symbolName.text );
			if( !symbol ){
				printf( "%s\n" , symbolName.text.c_str() );
			}
			assert( symbol );
			EXPRESSION_ASSERT( symbol );
			nextToken();

			ExpressionParameter symbolParam( m_writer->count() , param , symbol->IsReference() );
			symbolParam.symbol.push( symbol );
			_primary_expression( &symbolParam ); // 最優先( .[]() )
			_process_symbol_expression( &symbolParam , param , symbol );
		}
		break;
	case TokenType::Letter :
		{
			// 関数
			if( getToken(1).type == TokenType::Lparen ){
				_func_expression( param );
				return;
			}
			EXPRESSION_LOG( "LETTER \"%s\"\n" , currentToken.text.c_str() );
			nextToken();
			_process_letter_expression( param , currentToken.text );
			_primary_expression( param );
			_pushOperation( currentToken.text );
		}
		break;
	case TokenType::AsString :
		{
			nextToken();
			assert( getToken().type == TokenType::Lparen );
			nextToken();
			_exp( param );
			assert( getToken().type == TokenType::Rparen );
			nextToken();

			// スタックを一つ戻す
			R--;

			// 先頭にあるデータ情報に対して文字列命令
			// 命令終了後、その再度データを詰む
			m_writer->write( EMnemonic::AsString );
			m_writer->write( EMnemonic::REG );
			m_writer->writeInt32( R );
			EXPRESSION_LOG( "string R[%d]\n" , R );

			// 再度詰みなおす
			R++;
		}
		break;
	case TokenType::AsInteger :
		break;
	}
}


// ************************************************************************************
// メンバ関数タイプである場合は
// 直前のシンボルをレジスタに積んでから(thisポインタを渡す)
void Parser::_method_expression( Scope* funcScope , ExpressionParameter* const param ){
	if( !funcScope ){
		return;
	}
	if( funcScope->isStructMethodScope() ){
		assert( param );
		assert( param->symbol.top() );

		_pushSYMBOL( param->symbol.top() , false , true );
		_popSYMBOL();

		m_writer->write( EMnemonic::Push );
		m_writer->write( EMnemonic::REG );
		m_writer->writeInt32( R );
		m_writer->write( true );
		param->symbolAfterDotSyntax = ExpressionParameter::Function;
	}
}


// ************************************************************************************
// 直前のシンボルが存在する場合、メソッドなのでクラススコープから関数検索
// シンボルが存在しない場合はグローバルスコープとして扱い、全体から関数検索
Scope* Parser::_findMethodScope( string& scopeName , ExpressionParameter* const param ){
	Scope* result = NULL;
	SymbolInfo* symbol = NULL;
	if( param ){
		if( param->symbol.size() > 0 ){
			symbol = param->symbol.top();
			//cout << "top symbol:" << symbol->Name() << endl;
		}
	}
	if( symbol ){
		assert( symbol->getClass() );
		Scope* scope = m_currentScope->findScopeFromTop( symbol->getClass()->Name() );
		if( scope ){
			Scope* funcScope = scope->findScope( scopeName );
			assert( funcScope );
			scopeName = symbol->getClass()->Name() + "##" + scopeName;
			//cout << "scopeName:" << scopeName << endl;
			return funcScope;
		}
	}
	return m_currentScope->findScopeFromTop( scopeName );
}

// ************************************************************************************
// クラスの場合暗黙的にthisが最初に呼ばれるので、
// クラス関数の場合はパラメータ検索は1番からスタートするようにする
static int getArgIndexFirst( Scope* funcScope ){
	if( !funcScope ){
		return 0;
	}
	if( funcScope->isStructMethodScope() ){
		return 1;
	}
	return 0;
}

// ************************************************************************************
// 関数処理
// 直前にシンボルがある場合はクラスメンバ関数として扱い、
// 存在しない場合はグローバルな関数として扱う
void Parser::_func_expression( ExpressionParameter* const param ){
	string funcName = getToken().text;
	nextToken();
	EXPRESSION_ASSERT( getToken().type == TokenType::Lparen && "関数と想定していたシンボルの隣りに'('がありません\n" );
	nextToken();
	// 関数スコープを探してきてシンボル表を取得できるようにする
	Scope* funcScope = _findMethodScope( funcName , param );
	_method_expression( funcScope , param );

	ExpressionParameter pushCountParam( m_writer->count() , param , false );
	int argCount = getArgIndexFirst( funcScope );
	while( getToken().type != TokenType::Rparen ){
		bool isReference = 0;
		if( funcScope ){
			SymbolInfo* argSymbol = funcScope->getSymbol( argCount );
			EXPRESSION_ASSERT( argSymbol );
			EXPRESSION_LOG( "arg[%s]\n" , argSymbol->Name().c_str() );
			isReference = argSymbol->IsReference();
		}

		ExpressionParameter functionArgSymbolParam( m_writer->count() , &pushCountParam , isReference );
		_exp( &functionArgSymbolParam );
		_popSYMBOL();
		_popOperation();
		m_writer->write( EMnemonic::Push );
		m_writer->write( EMnemonic::REG );
		m_writer->writeInt32( R );
		m_writer->write( functionArgSymbolParam.isReferenceSymbol );
		EXPRESSION_LOG( "PUSH %d\n" , R );
		if( getToken().type == TokenType::Comma ){
			nextToken();
		}
		pushCountParam.pushCount++;
		argCount++;
	}
	nextToken();

	m_writer->write( EMnemonic::Call );
	m_writer->write( EMnemonic::LIT_STRING );
	m_writer->writeString( funcName );

	EXPRESSION_LOG( "CALL %s\n" , funcName.c_str() );
	m_writer->write( EMnemonic::Mov );
	m_writer->write( EMnemonic::REG );
	m_writer->writeInt32( R );
	m_writer->write( EMnemonic::REG );
	m_writer->writeInt32( REG_INDEX_FUNC );
	_pushOperation( OperationStack( funcName ) );
	R++;
	EXPRESSION_LOG( "CALL END [Token %s]\n" , getToken().text.c_str() );
}

// ()[].
void Parser::_primary_expression( ExpressionParameter* const param ){
	TOKEN prevSymbolToken = getToken( -1 );
	TOKEN currentToken = getToken();
	SymbolInfo* prevSymbol = m_currentScope->getSymbol( prevSymbolToken.text );
	if( !prevSymbol && param && param->symbol.size() > 0 ){
		prevSymbol = param->symbol.top();
	}

	if( 
		currentToken.type == TokenType::Lparen   || 
		currentToken.type == TokenType::Dot      || 
		currentToken.type == TokenType::Lbracket || 
		currentToken.type == TokenType::Assign ){
		switch( currentToken.type ){
		case TokenType::Lparen :
			{
				nextToken();
				_exp( param );
				EXPRESSION_LOG( "CheckMatch :: '(' ... '%s'\n" , getToken().text.c_str() );
				EXPRESSION_ASSERT( getToken().type == TokenType::Rparen );
				currentToken = nextToken();
			}
			break;
		case TokenType::Dot :
			{
				EXPRESSION_LOG( "DotSyntax :: '%s'\n" , getToken().text.c_str() );
				nextToken();
				_token_expression( param );
			}
			break;
		case TokenType::Lbracket :
			{
				param->bracketCount++;
				param->symbol.push( prevSymbol );
				EXPRESSION_LOG( "Lbracket :: '%s'[WC %d]\n" , getToken().text.c_str() , m_writer->count() );
				nextToken();
				_exp( param );
				EXPRESSION_LOG( "bracket end!! [ ... %s\n" , getToken().text.c_str() );
				EXPRESSION_ASSERT( getToken().type == TokenType::Rbracket );
				currentToken = nextToken();

				int nextSymbolType = 0; 
				if( getToken().type == TokenType::Dot && getToken(1).type == TokenType::Letter && getToken(2).type == TokenType::Lparen ){ // .xxx() メンバ関数
					nextSymbolType = 1;
				}
				else if( getToken().type == TokenType::Dot && getToken(1).type == TokenType::Letter && getToken(2).type != TokenType::Lparen ){ // .xxx メンバ変数
					nextSymbolType = 2;
				}

				switch( nextSymbolType ){
				case 0 :
				case 2 :
					_popOperation();
					break;
				}
				_popArrayIndexOperation( prevSymbol , param );
				_primary_expression( param );
			}
			break;
		}
	}
}

// ++,--
void Parser::_after_inc_expression( ExpressionParameter* const param ){
	if( getToken().type == TokenType::Inc || getToken().type == TokenType::Dec ){
		_popAfterIncOperation( getToken() );
		nextToken();
	}
}

// !,-,--,++
void Parser::_unary_expression( ExpressionParameter* const param ){
	const TOKEN& currentToken = getToken();
	switch( currentToken.type ){
	case TokenType::Not :
		{
			EXPRESSION_LOG( "単項演算子 Not(!) \n" );
			nextToken();
			_exp( param );
			EXPRESSION_LOG( "LogicalNotOperation R[%d] == 0 ? true : false \n" , R );
		}
		break;
	case TokenType::Sub :
		{
			TOKEN zeroToken( "0" , TokenType::Digit );
			EXPRESSION_LOG( "単項演算子 Minus(-) \n" );
			nextToken();
			_pushLITERAL( zeroToken );
			_pushOperation( OperationStack( zeroToken.text ) );
			_token_expression( param );
			_popOperation( TOKEN( "-" , TokenType::Sub ) );
			EXPRESSION_LOG( "MinusOperation R[%d] \n" , R );
		}
		break;
	case TokenType::Inc :
	case TokenType::Dec :
		{
			nextToken();
			_token_expression( param );
			_popPrevIncOperation( currentToken );
		}
		break;
	}
}



} // namespace Assembly
} // namespace SenchaVM

