#include "vmparser.h"
#include "vmscope.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"
#include "error\vmerror.h"

using namespace std;
#define PARSER_ASSERT VM_ASSERT


namespace SenchaVM {
namespace Assembly {

static const TOKEN EndToken( "END_TOKEN" , TokenType::END_TOKEN );

Parser::Parser( vector<TOKEN> tokens ){
	_initialize( tokens );
	_execute();
}
Parser::~Parser(){
	VM_PRINT( "Parser Finish!!\n" );
}
void Parser::_initialize( vector<TOKEN> tokens ){
	m_pos = 0;
	m_tokens = tokens;
	m_scope = Assembly::CScope( new Assembly::Scope( "global" , SCOPE_LEVEL_GLOBAL ) );
	m_currentScope = m_scope.get();
	m_writer = CBinaryWriter( new BinaryWriter() );
	m_R = 0;
	//printf( "Global %p\n" , m_currentScope );
}
// 一個前のトークンに戻る
const TOKEN& Parser::backToken(){
	m_pos--;
	return getToken();
}
// 次のトークンへ進める
const TOKEN& Parser::nextToken(){
	m_pos++;
	return getToken();
}
// 現在のトークンを取得する
const TOKEN& Parser::getToken(){
	if( m_pos >= m_tokens.size() ) return EndToken;
	return m_tokens[m_pos];
}
// 現在の位置からオフセット値を計算した場所にあるトークンを取得
const TOKEN& Parser::getToken(int ofs){
	size_t pos = m_pos + ofs;
	if( pos >= m_tokens.size() ) return EndToken;
	return m_tokens[pos];
}
// 次のトークンある？
bool Parser::hasNext(){
	if( m_pos >= m_tokens.size() ) return false;
	return true;
}
// 指定数トークンを進める
void Parser::_consume( int consumeCount ){
	m_pos += consumeCount;
}
// 計算に使用するシンボル、リテラルなどをスタックに積む
void Parser::_pushOperation( OperationStack item ){
	m_operationStack.push( item );
}
// シンボル、リテラルを上から戻す
OperationStack Parser::_popOperation(){
	PARSER_ASSERT( m_operationStack.size() > 0 );
	OperationStack result = m_operationStack.top();
	m_operationStack.pop();
	return result;
}
// 解析開始
void Parser::_execute(){
	try
	{
		while( hasNext() ){
			_parse( NULL );
		}
		_entryFunction( "global" , 0 );
	}
	catch( VMError& error )
	{
		std::cout << error.getMessage() << std::endl;
		m_assemblyCollection.clear();
	}
}

/*
 * 変数解析
 */
Parser::parse_variable::parse_variable( Parser* parser ) : Parser::interpreter( parser ){
	expression e( parser );
}


/*
 * asによるデータ型付け
 */
Parser::parse_as::parse_as( Parser* parser , varinfo& var ) : Parser::interpreter( parser ){
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		SymbolInfo* dataSymbol = this->getSymbol();
		((SymbolInfo*)var)->setClass( dataSymbol );
		((SymbolInfo*)var)->copyAndAddChildrenOfSymbol( dataSymbol );
	}
}

/*
 * 配列型である場合評価
 */
Parser::parse_array::parse_array( Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	//assert( this->NextTokenIf( TokenType::Lparen ) );
	//this->Next();
	//if( this->NextTokenIf( TokenType::Letter ) ){
	//	this->Next();
	//	as a( parser , symbol );
	//}
	//if( this->NextTokenIf( TokenType::Digit ) ){
	//	this->Next();
	//	int arrayLength = this->getTokenInt();
	//	this->Next();
	//	assert( this->TokenIf( TokenType::Rparen ) );
	//	this->Next();
	//	assert( this->TokenIf( TokenType::Semicolon ) );
	//	this->Next();
	//	symbol->ArrayLength( arrayLength );
	//}
}

/*
 * 式評価
 */
Parser::expression::expression( Parser* parser ) : Parser::interpreter( parser ){
	R = 0;
	expression3( this , parser );
}


/*
 * =
 * +=
 * -=
 * *=
 * /=
 * %=
 */
Parser::expression0::expression0( expression* exp , Parser* parser , var_chain& var ) : Parser::expression_base( exp , parser ) {
	bool isExpression = false;
	if( this->NextTokenIf( TokenType::Assign ) )    isExpression = true;
	if( this->NextTokenIf( TokenType::AddAssign ) ) isExpression = true;
	if( this->NextTokenIf( TokenType::SubAssign ) ) isExpression = true;
	if( this->NextTokenIf( TokenType::MulAssign ) ) isExpression = true;
	if( this->NextTokenIf( TokenType::RemAssign ) ) isExpression = true;
	if( isExpression ){
		this->Next();
		const TOKEN_TYPE& opetype = this->getTokenType();
		expression1( exp , parser );
		exp->Assign( var );
	}
}

/*
 * +
 * -
 */
Parser::expression1::expression1( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	expression2 expr( exp , parser );

	while( this->NextTokenIf( TokenType::Add ) || this->NextTokenIf( TokenType::Sub ) ){
		this->Next();
		const TOKEN_TYPE& opetype = this->getTokenType();
		expression2( exp , parser );
		exp->CalcStack( opetype );
	}
}

/*
 * *
 * /
 * %
 */
Parser::expression2::expression2( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	expression3 expr( exp , parser );

	while( this->NextTokenIf( TokenType::Mul ) || this->NextTokenIf( TokenType::Div ) || this->NextTokenIf( TokenType::Rem ) ){
		this->Next();
		const TOKEN_TYPE& opetype = this->getTokenType();
		expression3( exp , parser );
		exp->CalcStack( opetype );
	}
}

/*
 * TOKEN
 */
Parser::expression3::expression3( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	if( this->NextTokenIf( TokenType::VariableSymbol ) ){
		this->Next();
		expression_variable( exp , parser );
	}
	else if( this->NextTokenIf( TokenType::RefSymbol ) ){
		this->Next();
		expression_variable( exp , parser );
	}
	else if( this->NextTokenIf( TokenType::Digit ) ){
		this->Next();
		this->ExprPushData( this->getTokenDouble() );
	}
	else if( this->NextTokenIf( TokenType::String ) ){
		this->Next();
		this->ExprPushData( this->getTokenString() );
	}
	else if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		expression_func( exp , parser );
	}
	else if( this->NextTokenIf( TokenType::Lbracket ) ){
		this->Next();
	}
	else if( this->NextTokenIf( TokenType::AsString ) ){
		this->Next();
	}
	else if( this->NextTokenIf( TokenType::AsInteger ) ){
		this->Next();
	}
	else if( this->NextTokenIf( TokenType::Lparen ) ){
		this->Next();
		expression1( exp , parser );
		if( this->NextTokenIf( TokenType::Rparen ) ){
			this->Next();
		}
	}
}

/* 
 * 変数シンボル
 */
Parser::expression_variable::expression_variable( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	this->expr = exp;
	this->parentSymbol = NULL;
	this->exp();
}

Parser::expression_variable::expression_variable( expression* exp , Parser* parser , var_chain& v , SymbolInfo* instSymbol ) : Parser::expression_base( exp , parser ) {
	this->var = v;
	this->expr = exp;
	this->parentSymbol = instSymbol;
	this->exp();
}

void Parser::expression_variable::exp(){
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		string symbolName = this->getTokenString();
		SymbolInfo* symbol = this->getSymbolInScopeOrParentSymbol( this->parentSymbol , symbolName );
		if( !symbol ){
			if( this->parentSymbol ){
				throw VMError( new ERROR_INFO_C2065( symbolName ) );
			}
			symbol = this->addSymbol( symbolName );
			printf( "シンボル生成 : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );
		}
		varinfo current( symbol );
		this->var.push( current );
		if( this->NextTokenIf( TokenType::Dot ) ){
			this->Next();
			this->dot( symbolName );
		}
		else if( this->NextTokenIf( TokenType::Lbracket ) ){
			this->Next();
			this->bracket( symbolName );
		}
		else if( this->NextTokenIf( TokenType::Lparen ) ){
			this->Next();
			this->memberFunc( symbolName );
		}
		else if( this->NextTokenIf( TokenType::As ) ){
			this->Next();
			parse_as( this->m_parser , current );
		}
		else {
			expression0( this->expr , this->m_parser , this->var );
			this->expr->ExprPushData( this->var );
		}
	}
}



void Parser::expression_variable::dot( const string& symbolName ){
	SymbolInfo* instSymbol = this->getSymbolInScopeOrParentSymbol( this->parentSymbol , symbolName );
	if( !instSymbol ){
		throw VMError( new ERROR_INFO_C2065( symbolName ) );
	}
	expression_variable( expr , m_parser , this->var , instSymbol );
}

void Parser::expression_variable::bracket( const string& symbolName ){
	expression_bracket( this->expr , this->m_parser , this->parentSymbol , this->var );
}

void Parser::expression_variable::memberFunc( const string& symbolName ){
	if( !this->isExistSymbolInParent( symbolName ) ){ throw VMError( new ERROR_INFO_C2065( symbolName ) ); }
	varinfo inst( this->parentSymbol );
	this->expr->PushThis( inst );
	while( !this->NextTokenIf( TokenType::Rparen ) ){
		expression1( this->expr , m_parser );
		if( this->NextTokenIf( TokenType::Comma ) ){
			this->Next();
		}
		this->expr->Push();
	}
	this->Next();
	this->expr->CallFunction( symbolName );
}




Parser::expression_bracket::expression_bracket( expression* exp , Parser* parser , SymbolInfo* parent , var_chain& var ) : Parser::expression_base( exp , parser ) {
	expression1( exp , this->m_parser );
	if( !this->NextTokenIf( TokenType::Rbracket ) ){
		throw VMError( new ERROR_INFO_C2143() );
	}
	varinfo& current = var.peek();
	current.Index( exp->R-1 );
	this->Next();
	if( this->NextTokenIf( TokenType::Dot ) ){
		this->Next();
		SymbolInfo* instSymbol = this->getSymbolInScopeOrParentSymbol( parent , ((SymbolInfo*)current)->Name() );
		expression_variable( exp , this->m_parser , var , instSymbol );
	}
	else {
		expression0( exp , m_parser , var );
		exp->ExprPushData( var );
	}
}

Parser::expression_func::expression_func( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ){
	string funcName = this->getTokenString();
	if( !this->NextTokenIf( TokenType::Lparen ) ){
		this->Next();
		throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
	}

	this->Next();
	while( !this->NextTokenIf( TokenType::Rparen ) ){
		expression1( exp , parser );
		if( this->NextTokenIf( TokenType::Comma ) ){
			this->Next();
		}
		exp->Push();
	}
	this->Next();
	exp->CallFunction( funcName );
}

// 解析処理
// 各ステートメントの処理を行う
void Parser::_parse( ParseParameter* param ){
	const TOKEN& Next = getToken();
	switch( Next.type ){
	case TokenType::Function :
		_parse_function( param );
		break;
// **************************************************************
// $xxxなどの定義
// 未定義ならシンボル追加する
// **************************************************************
	case TokenType::VariableSymbol :
	case TokenType::RefSymbol :
		this->backToken();
		parse_variable( this );
		//_parse_variable( param );
		break;
	case TokenType::Inc :
	case TokenType::Dec :
	case TokenType::AsString :
	case TokenType::AsInteger :
		_expression( param );
		if( getToken().type == TokenType::Semicolon ){
			nextToken();
		}
		break;
	case TokenType::Switch :
		_parse_switch( param );
		break;
	case TokenType::For :
		_parse_for( param );
		break;
	case TokenType::While :
		_parse_while( param );
		break;
	case TokenType::If :
		_parse_if( param );
		break;
	case TokenType::Continue :
		_parse_continue( param );
		break;
	case TokenType::Break :
		_parse_break( param );
		break;
	case TokenType::YIELD :
		break;
	case TokenType::Return :
		_parse_return( param );
		break;
	case TokenType::Struct :
		_parse_struct( param );
		break;
	case TokenType::Namespace :
		break;
	case TokenType::Letter :
		_parse_letter( param );
		break;
	case TokenType::BeginChunk :
		_parse_chunk( param );
		break;
	case TokenType::EndChunk : 
		break;
	}
	nextToken();
}

// continueは
// ・for
// ・while
// 文内でしか使用できない文であるため、必ずパラメータがやってくる（来ない場合は処理上おかしい）
// そのため、まず最初にparamの有効チェックを行う。
// パラメータがある場合、そのポインタに現在の書き込み位置をプッシュする
void Parser::_parse_continue( ParseParameter* param ){
	PARSER_ASSERT( param );
	JumpInfo jumpInfo;
	m_writer->write( EMnemonic::Jmp );
	jumpInfo.codeAddr = m_writer->count();
	param->continueAddr.push_back( jumpInfo );
	m_writer->writeInt32( 0 );	
	nextToken();
	if( getToken().type == TokenType::Semicolon ){
		nextToken();
	}
}

// breakは
// ・for
// ・while
// ・switch
// 文内でしか使用できない文であるため、必ずパラメータがやってくる（来ない場合は処理上おかしい）
// そのため、まず最初にparamの有効チェックを行う。
// パラメータがある場合、そのポインタに現在の書き込み位置をプッシュする
void Parser::_parse_break( ParseParameter* param ){
	PARSER_ASSERT( param );
	JumpInfo jumpInfo;
	m_writer->write( EMnemonic::Jmp );
	jumpInfo.codeAddr = m_writer->count();
	param->breakAddr.push_back( jumpInfo );
	m_writer->writeInt32( 0 );
	nextToken();
	if( getToken().type == TokenType::Semicolon ){
		nextToken();
	}
}

// return文解析
// returnの次の計算式を評価し、その結果を格納する
// 基本的にはR0番にしか戻り値は入らない
// return文より下には行ってはいけないので発見したらそこで関数終了
void Parser::_parse_return( ParseParameter* param ){
	PARSER_ASSERT( param );
	PARSER_ASSERT( getToken().type == TokenType::Return );
	nextToken();
	_expression( param );

	m_writer->write( EMnemonic::RET );
	m_writer->write( EMnemonic::REG );
	m_writer->writeInt32( this->m_R );
	m_writer->write( EMnemonic::EndFunc );

	PARSER_ASSERT( getToken().type  == TokenType::Semicolon );
	if( getToken().type  == TokenType::Semicolon ){
		nextToken();
	}
}

// 変数宣言・変数への代入命令など
// $が見つかった場合、もしくは ','に続いて変数が存在する場合呼ばれる 
void Parser::_parse_variable( ParseParameter* param ){
	using namespace Assembly;
	const TOKEN& t_0 = getToken(0);
	const TOKEN& t_1 = getToken(1);
	const TOKEN& t_2 = getToken(2);
	const TOKEN& t_3 = getToken(3);

	//VM_PRINT( "現在解析中のスコープに変数[%s]シンボルを追加\n" , t_1.text.c_str() );
	// $VariableName
	//	 			 = | 
	//				+= | 
	//				-= |
	//				*= | 
	//				/= | 
	//				%=
	if( _isVariable( t_0 , t_1 ) ){
		// m_currentScope ... これは現在のスコープインスタンス
		// スコープは入れ子になるのでスコープクラスには子スコープのポインタを持たせる
		// 現在のスコープから更に子階層のスコープへ移動させる場合は、子スコープを生成しカレントスコープの参照を移す(戻ってきたときに元の位置に戻れるようにスタックで保存する)
		// 子スコープは親スコープの参照を持ち、シンボル検索時は親スコープから検索する
		// 親スコープに同名のシンボルが存在する場合、それを優先的に扱う
		// if,while,switch,forは定義されたタイミングでスコープを移す

		// getSymbolはシンボル名を指定し、該当するシンボルインスタンスを返却する作りにする
		SymbolInfo* symbol = m_currentScope->getSymbol( t_1.text );

		// 存在しないシンボルの場合はその場で生成
		if( !symbol ){
			symbol = m_currentScope->addSymbol( t_1.text );
			VM_PRINT( "シンボル生成 : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );
		}
		if( t_0.type == TokenType::RefSymbol ){
			symbol->IsReference( true );
		}
		if( t_2.type == TokenType::As ){
			_parse_as( symbol );
			return;
		}

		_expression( param );
		if( getToken().type == TokenType::Semicolon ){
			nextToken();
		}
	}
}


static void dumpSYMBOL( SymbolInfo* symbol , int childCount ){
	if( symbol->getClass() ){
		printf( "[class %s]\n" , symbol->getClass()->Name().c_str() );
	}
	for( int i = 0 ; i < symbol->ChildSymbolCount() ; i++ ){
		for( int j = 0 ; j < childCount ; j++ ){
			printf( ">>" );
		}
		SymbolInfo* child = symbol->getChild(i);
		printf( "child[%d] %s[len %d]" , child->Addr() , child->Name().c_str() , child->ArrayLength() );
		if( child->getClass() ){
			printf( "[class %s]" , child->getClass()->Name().c_str() );
		}
		printf( "\n" );
		dumpSYMBOL( child , childCount + 1 );
	}
}


// データ型付け処理
// 対象のシンボルに指定のデータ型シンボルを新規に作成していく
// 各シンボルには先頭から数えたアドレスを割り当てる
// todo:: 関数分けしたほうが良い
void Parser::_parse_as( SymbolInfo* symbol ){
#define AS_DEBUG
#ifdef AS_DEBUG
	#define AS_LOG    VM_PRINT
	#define AS_ASSERT VM_ASSERT
//	#define AS_LOG    printf
//	#define AS_ASSERT assert
#else
	#define AS_LOG(...)
	#define AS_ASSERT(...)
#endif
	nextToken(); // "$"飛ばす
	nextToken(); // シンボル飛ばす
	nextToken(); // "as"飛ばす

	// *************************************************
	// as arrayで配列として処理する
	// *************************************************
	if( getToken().type == TokenType::Array ){
		nextToken(); // array飛ばす
		AS_ASSERT( getToken().type == TokenType::Lparen );
		nextToken(); // '('飛ばす

		// *************************************************
		// データ型である場合
		// *************************************************
		if( getToken().type == TokenType::Letter ){
			string dataTypeName = getToken().text;
			SymbolInfo* dataSymbol = m_currentScope->getSymbol( dataTypeName );
			AS_ASSERT( dataSymbol );
			symbol->setClass( dataSymbol );
			symbol->copyAndAddChildrenOfSymbol( dataSymbol );
			symbol->setupChildrenAddresToParentAddresOffset();

			nextToken(); // LETTER飛ばす
			AS_ASSERT( getToken().type == TokenType::Comma );
			nextToken(); // ','飛ばす
		}

		// *************************************************
		// 配列番号
		// *************************************************
		AS_ASSERT( getToken().type == TokenType::Digit );
		int arrayLength = atoi( getToken().text.c_str() );
		nextToken(); // 数字飛ばす
		AS_ASSERT( getToken().type == TokenType::Rparen );
		nextToken(); // ')'飛ばす
		AS_ASSERT( getToken().type == TokenType::Semicolon );
		nextToken(); // ';'飛ばす

		symbol->ArrayLength( arrayLength );
		return;
	}

	AS_ASSERT( getToken().type == TokenType::Letter );
	string dataTypeName = getToken().text;
	SymbolInfo* dataSymbol = m_currentScope->getSymbol( dataTypeName );
	AS_ASSERT( dataSymbol );
	AS_LOG( ">>As Log [SymbolName %s][DataSymbol %s]\n" , symbol->Name().c_str() , dataSymbol->Name().c_str() );
	symbol->setClass( dataSymbol );
	symbol->copyAndAddChildrenOfSymbol( dataSymbol );
	symbol->setupChildrenAddresToParentAddresOffset();
	//cout << "Symbol:" << symbol->Name() << " , Addr:" << symbol->Addr() << endl;;
	//dumpSYMBOL( symbol , 1 );
	nextToken();
	if( getToken().type == TokenType::Semicolon )   nextToken();
	else if( getToken().type == TokenType::Comma )  nextToken();
}


// 文字確認
// 関数である場合、評価を行う
void Parser::_parse_letter( ParseParameter* param ){
#define LETTER_DEBUG
#ifdef LETTER_DEBUG
//	#define LETTER_LOG printf
	#define LETTER_LOG VM_PRINT
	#define LETTER_ASSERT VM_ASSERT
#else
	#define LETTER_LOG(...)
	#define LETTER_ASSERT(...)
#endif
	LETTER_LOG( "PARSE->LETTER... " );
	const TOKEN& t_0 = getToken(0);
	const TOKEN& t_1 = getToken(1);
	if( t_0.type == TokenType::Letter && t_1.type == TokenType::Lparen ){
		LETTER_LOG( "Function!![%s]\n" , getToken().text.c_str() );
		_expression( param );
		if( getToken().type == TokenType::Semicolon ){
			nextToken();
		}
		LETTER_LOG( "Function Expression End!! [%s]\n" , getToken().text.c_str() );
	}
#undef LETTER_DEBUG
}

//	--------------------------------------------------------
//	構造体解析
//	--------------------------------------------------------
void Parser::_parse_struct( ParseParameter* param ){
//#define STRUCT_PRINT(...)
#define STRUCT_PRINT VM_PRINT
//#define STRUCT_PRINT printf
	STRUCT_PRINT( "構造体 解析開始\n" );
	TOKEN className = nextToken();
	PARSER_ASSERT( className.type == TokenType::Letter );
	PARSER_ASSERT( className.text.size() > 0 );
	STRUCT_PRINT( "構造体名 : %s \n" , className.text.c_str() );
	TOKEN beginChunk = nextToken();
	PARSER_ASSERT( beginChunk.type == TokenType::BeginChunk );
	nextToken();
	// クラスシンボルは重複してはいけないので
	// とりあえず検索してみる
	SymbolInfo* symbol = m_currentScope->getSymbol( className.text );
	assert( !symbol );
	symbol = m_currentScope->addSymbol( className.text );
	STRUCT_PRINT( "構造体シンボル生成 : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );


	ParseParameter classParam;
	m_currentScope = m_currentScope->goToStructScope( symbol->Name() );
	int scopeLevel = m_currentScope->ScopeLevel();

	while( hasNext() ){
		_parse( &classParam );
		if( getToken().type == TokenType::EndChunk ){
			break;
		}
	}
	const vector<SymbolInfo*>& structFields = m_currentScope->getChildren();
	symbol->Addr(0);
	symbol->copyAndAddChildrenOfSymbol( structFields );
	//dumpSYMBOL( symbol , 1 );
	m_currentScope = m_currentScope->backToChildScope();
#undef STRUCT_PRINT
}


// チャンク処理
// 終了チャンクを発見するまでループする
void Parser::_parse_chunk( ParseParameter* param ){
//#define CHUNK_PRINT printf
#define CHUNK_PRINT VM_PRINT
	m_currentScope = m_currentScope->goToChildScope( "__chunk__" );

	CHUNK_PRINT( ">>チャンク 解析開始\n" );
	PARSER_ASSERT( getToken().type == TokenType::BeginChunk );
	nextToken();
	while( hasNext() ){
		_parse( param );
		if( getToken().type == TokenType::EndChunk ){
			nextToken();
			break;
		}
	}

	CHUNK_PRINT( ">>チャンク 解析終了 [現在のトークン:%s][発見シンボル数:%d][%d][%d][%d]\n" , 
		getToken().text.c_str() , 
		m_currentScope->getSymbolCountInScopeAttribute() , 
		m_currentScope->getParentSymbolCount( VariableLocal ) , 
		m_currentScope->getAllParentSymbolCount( VariableLocal ) ,
		m_currentScope->getAllSymbolCount( VariableLocal ) );
	m_currentScope = m_currentScope->backToChildScope();
#undef CHUNK_PRINT
}


// *************************************************************************************
// 構造体スコープの場合
// 現在の関数スコープは構造体メンバ関数、という属性として扱い
// 暗黙的にthisシンボルを定義する
// 関数名は"className##methodName"というように繋げる
// *************************************************************************************
static void _set_thisPointer( Scope* parentScope , Scope* currentScope ){
	if( !parentScope ){
		return;
	}
	//if( parentScope->isStructScope() ){
	//	throw VMError( new ERROR_INFO_3001( currentScope->ScopeName() ) );
	//}

	// 親スコープが構造体スコープである場合、構造体メソッドであることになるので通知
	// "this"という名前のシンボルを暗黙的に生成する
	// クラスの持つ同名のシンボルを取得し、クラスに設定する
	if( parentScope->isStructScope() ){
		currentScope->notifyStructMethodScope();
		SymbolInfo* thisSymbol = currentScope->addSymbol( "this" );
		SymbolInfo* classSymbol = parentScope->getSymbol( parentScope->ScopeName() );
		assert( classSymbol );
		thisSymbol->setClass( classSymbol );
		thisSymbol->SymbolType( VariableLocal );
		thisSymbol->IsReference( true );
		thisSymbol->copyAndAddChildrenOfSymbol( parentScope->getChildren() );
		thisSymbol->setupChildrenAddresToParentAddresOffset();
	}
}


// 関数内解析
void Parser::_parse_function( ParseParameter* param ){
//#define FUNC_PRINT printf
#define FUNC_PRINT  VM_PRINT
#define FUNC_ASSERT VM_ASSERT

	FUNC_PRINT( ">>関数 解析開始 [ScopeLevel %d]\n" , m_currentScope->ScopeLevel() );
	TOKEN funcName = nextToken();
	Scope* parentScope = m_currentScope;
	string scopeName = funcName.text;
	m_currentScope = m_currentScope->goToFunctionScope( scopeName );

	CBinaryWriter prev = m_writer;
	m_writer = CBinaryWriter( new BinaryWriter() );

	PARSER_ASSERT( funcName.type == TokenType::Letter );
	PARSER_ASSERT( funcName.text.size() > 0 );
	FUNC_PRINT( ">>関数名 : %s \n" , funcName.text.c_str() );
	TOKEN beginChunk = nextToken();
	PARSER_ASSERT( beginChunk.type == TokenType::Lparen );
	nextToken();

	// 構造体の場合"this"を定義
	_set_thisPointer( parentScope , m_currentScope );
	// "("から")"までの解析
	while( getToken().type != TokenType::Rparen ){
		_parse( NULL );
	}
	size_t args = m_currentScope->getSymbolCountMaxInAllScope( VariableLocal );
	for( unsigned int i = 0 ; i < args ; i++ ){
		m_writer->write( EMnemonic::Pop );
	}
	PARSER_ASSERT( getToken().type == TokenType::Rparen );
	nextToken();
	PARSER_ASSERT( getToken().type == TokenType::BeginChunk );
	nextToken();

	ParseParameter funcParam;
	funcParam.args = args;
	while( hasNext() ){
		_parse( &funcParam );
		if( getToken().type == TokenType::EndChunk ){
			m_writer->write( EMnemonic::EndFunc );
			break;
		}
	}
	FUNC_PRINT( ">>関数 解析終了 [ScopeLevel %d][SymbolCount %d]\n" , m_currentScope->ScopeLevel() , m_currentScope->getSymbolCountMaxInAllScope( VariableLocal ) );
	_entryFunction( scopeName , args );
	FUNC_PRINT( "\n" );	
	m_writer = prev;
	m_currentScope = m_currentScope->backToChildScope();
#undef FUNC_PRINT
}


// 関数内のアセンブリ情報をストリームに書き込む
void Parser::_entryFunction( string funcName , size_t args ){
//#define FUNC_PRINT
#define FUNC_PRINT VM_PRINT
//#define FUNC_PRINT printf
	if( m_currentScope->ScopeLevel() > 0 ) FUNC_PRINT( ">>関数登録 関数名[%s]\n" , funcName.c_str() );
	else                                   FUNC_PRINT( ">>グローバルスコープ\n" );
	ESymbolType scopeSymbolType = VariableLocal;
	if( m_currentScope->ScopeLevel() == 0 ){
		scopeSymbolType = VariableGlobal;
	}
	if( m_currentScope->getParentScope() ){
		if( m_currentScope->getParentScope()->isStructScope() ){
			funcName = m_currentScope->getParentScope()->ScopeName()+"##"+funcName;
		}
	}

	AssemblyInfo funcAssembly;
	funcAssembly.setName( funcName );
	funcAssembly.setArgs( args );
	funcAssembly.setStackFrame( m_currentScope->getSymbolCountMaxInAllScope( scopeSymbolType ) );
	CStream stream = m_writer->getStream();
	while( stream->hasNext() ){
		funcAssembly.pushByte( stream->getByte() );
	}
	m_assemblyCollection.assemblyInfo.push_back( funcAssembly );

	if( m_currentScope->ScopeLevel() > 0 ) FUNC_PRINT( ">>ローカル変数:%d\n" , funcAssembly.stackFrame() );
	else                                   FUNC_PRINT( ">>静的変数:%d\n"     , funcAssembly.stackFrame() );
#undef FUNC_PRINT
}

void Parser::_entryClass( string className ){
}


// '('から対応する')'が見つかるまでスキップ
void Parser::_skipParen(){
	int parenCount = 0;
	while( hasNext() ){
		const TOKEN& token = getToken();
		if( token.type == TokenType::Lparen ) parenCount++;
		if( token.type == TokenType::Rparen ) parenCount--;
		if( parenCount < 0 ){
			return;
		}
		nextToken();
	}
}


} // namespace Assembly
} // namespace SenchaVM
