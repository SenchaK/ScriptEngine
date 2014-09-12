#include "vm_parser.h"
#include "vm_scope.h"
#include "..\lexer\vm_lexer.h"
#include "..\vm_assembler.h"
#include "..\symbol\vm_symbol.h"
#include "error\vm_error.h"

using namespace std;
#define PARSER_ASSERT VM_ASSERT


namespace Sencha {
namespace VM {
namespace Assembly {

static const Token EndToken( "END_TOKEN" , Token::Type::END_TOKEN );

Parser::Parser( ITokenizer* tokenizer , VMBuiltIn* built_in , Log* logger ){
	this->initialize( tokenizer , built_in , logger );
	this->execute();
}

Parser::Parser( ITokenizer* tokenizer , VMBuiltIn* built_in ){
	this->initialize( tokenizer , built_in , NULL );
	this->execute();
}

Parser::Parser( ITokenizer* tokenizer ){
	this->initialize( tokenizer , NULL , NULL );
	this->execute();
}

Parser::~Parser(){
	if( this->m_scope ){
		delete this->m_scope;
		this->m_scope = NULL;
	}
	if( this->m_asm ){
		delete this->m_asm;
		this->m_asm = NULL;
	}
}

void Parser::initialize( ITokenizer* tokenizer , VMBuiltIn* built_in , Log* logger ){
	this->m_scope = new Scope( "global" , SCOPE_LEVEL_GLOBAL );
	this->m_asm = new VMAssembleCollection();
	this->m_log = logger;
	this->m_built_in = built_in;
	this->m_token = tokenizer;
	this->m_currentScope = m_scope;
	this->m_writer = CBinaryWriter( new BinaryWriter() );
}

// 一個前のトークンに戻る
const Token& Parser::backToken(){
	return *reinterpret_cast<Token*>( this->m_token->back() );
}

// 次のトークンへ進める
const Token& Parser::nextToken(){
	return *reinterpret_cast<Token*>( this->m_token->next() );
}

// 現在のトークンを取得する
const Token& Parser::getToken(){
	return *reinterpret_cast<Token*>( this->m_token->current() );
}

// 現在の位置からオフセット値を計算した場所にあるトークンを取得
const Token& Parser::getToken(int ofs){
	return *reinterpret_cast<Token*>( this->m_token->offset( ofs ) );
}

// 次のトークンある？
bool Parser::hasNext(){
	return this->m_token->hasNext();
}

// 解析開始
void Parser::execute(){
	try
	{
		while( hasNext() ){
			parse( NULL );
		}
		//_entryFunction( "global" , 0 );
	}
	catch( VMError& error )
	{
		std::cout << error.getMessage() << std::endl;
		delete m_asm;
		m_asm = NULL;
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
	if( this->NextTokenIf( Token::Type::Letter ) ){
		this->Next();
		Type* t = this->getType();
		((SymbolInfo*)var)->setType( t );
	}
}

/*
 * 配列型である場合評価
 */
Parser::parse_array::parse_array( Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	assert( this->NextTokenIf( Token::Type::Lparen ) );
	this->Next();
	//if( this->NextTokenIf( Token::Type::Letter ) ){
	//	this->Next();
	//	as a( parser , symbol );
	//}
	//if( this->NextTokenIf( Token::Type::Digit ) ){
	//	this->Next();
	//	int arrayLength = this->getTokenInt();
	//	this->Next();
	//	assert( this->TokenIf( Token::Type::Rparen ) );
	//	this->Next();
	//	assert( this->TokenIf( Token::Type::Semicolon ) );
	//	this->Next();
	//	symbol->ArrayLength( arrayLength );
	//}
}

/*
 * function発見
 * "function"の次のトークンから開始する。
 * 関数内解析
 */
Parser::parse_function::parse_function( Parser* parser ) : Parser::interpreter( parser ){
	this->funcName = this->getTokenString();
	this->GoToFunction( funcName );
	this->ErrorCheckNextToken( Token::Type::Lparen );
	this->Next();
	this->This();
	this->ParseWhile( Token::Type::Rparen );
	this->ErrorCheckNextToken( Token::Type::BeginChunk );
	this->Next();
	this->args = this->GetArgs();
	this->ParseWhile( Token::Type::EndChunk );
	this->WriteEndFunc();
	this->EntryFunction();
	this->GoToBack();
}

/*
 * 関数登録
 */
void Parser::parse_function::EntryFunction(){
	AsmInfo* funcAssembly = new AsmInfo();
	funcAssembly->setName( funcName );
	funcAssembly->setArgs( args.size() );
	funcAssembly->setStackFrame( this->GetStackFrame() );
	funcAssembly->setBytes( this->GetCurrentStream() );
	this->EntryAssembly( funcAssembly );
}

/*
 * 構造体スコープの場合
 * 現在の関数スコープは構造体メンバ関数、という属性として扱い
 * 暗黙的にthisシンボルを定義する
 */
void Parser::parse_function::This(){
	if( !this->isStructScope() ){
		return;
	}
	string symbolName = "this";
	this->NotifyStructMethodScope();
	SymbolInfo* thisSymbol = this->addSymbol( symbolName );
	Type* t = this->getType();
	thisSymbol->setType( t );
	thisSymbol->Location( VariableLocal );
	thisSymbol->IsReference( true );
}

/*
 * struct宣言解析
 * "struct"の次から解析を行う
 */
Parser::parse_struct::parse_struct( Parser* parser ) : Parser::interpreter( parser ){
	string name = this->getTokenString();
	this->ErrorCheckNextToken( Token::Type::BeginChunk );
	SymbolInfo* symbol = this->addSymbol( name );
	Type* t = this->GoToStruct( name );
	this->Next();
	this->ParseWhile( Token::Type::EndChunk );
	symbol->setType( t );
	this->GoToBack();
}

/*
 * チャンク構造解析
 * チャンクの次から解析を始める
 */
Parser::parse_chunk::parse_chunk( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	this->GoToChunk();
	this->ParseWhile( Token::Type::EndChunk , args );
	this->GoToBack();
}

/*
 * return文解析
 * "return"から解析開始
 * returnの次の計算式を評価し、その結果を格納する
 * 基本的にはR0番にしか戻り値は入らない
 * return文より下には行ってはいけないので発見したらそこで関数終了
 */
Parser::parse_return::parse_return( Parser* parser ) : Parser::interpreter( parser ){
	expression e( parser );
	e.R--;
	this->WriteReturn( e.R );
}


/*
 * if文解析
 * 文法チェックを行い計算式評価。
 * 評価結果が0の場合はelse文発見までジャンプ
 * そうでない場合はジャンプしない
 * チャンク終了地点に終了地点までのジャンプを入れる
 * elseを発見した場合
 * ・ifを通ってきた場合は終了地点までジャンプ
 * ・ifで評価されなかった場合はifと同じ評価を行う
 * （else ifの場合はifで引っかかり、elseの場合はその処理が評価される）
 */
Parser::parse_if::parse_if( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	this->ErrorCheckToken( Token::Type::Lparen );
	expression e( parser );
	int IF_JumpPos = this->WriteJZ();
	this->ErrorCheckNextToken( Token::Type::Rparen );
	this->Next();
	this->Parse( args );
	this->WriteJmpPos( IF_JumpPos );
	if( !this->NextTokenIf( Token::Type::Else ) ){
	}
	while( this->NextTokenIf( Token::Type::Else ) ){
		this->Next();
		int IF_EndJumpPos = this->WriteJ();
		this->WriteJmpPos( IF_JumpPos );
		this->Next();
		this->Parse( args );
		this->WriteJmpPos( IF_EndJumpPos );
	}
}

Parser::parse_switch::parse_switch( Parser* parser ) : Parser::interpreter( parser ){
};

Parser::parse_for::parse_for( Parser* parser ) : Parser::interpreter( parser ){
	this->GoToChunk();
	this->ErrorCheckToken( Token::Type::Lparen );
	expression first_expr( parser );
	this->ErrorCheckNextToken( Token::Type::Semicolon );
	
	int continuePos = this->GetWritePos();
	expression loop_end_expr( parser );
	this->ErrorCheckNextToken( Token::Type::Semicolon );
	int forJmpPos = this->WriteJZ();
	CBinaryWriter old = this->CreateNewWriter();
	expression last_expr( parser );

	this->ErrorCheckNextToken( Token::Type::Rparen );
	this->Next();
	CBinaryWriter lastExprCode = this->SetWriter( old );
	
	Args args( parser );
	this->Parse( &args );
	this->AppendWriter( lastExprCode );
	this->WriteJ( continuePos );
	this->WriteJmpPos( forJmpPos );
	this->GoToBack();
	int breakPos = this->GetWritePos();
	args.WriteBreak( breakPos );
	args.WriteContinue( continuePos );
};

Parser::parse_while::parse_while( Parser* parser ) : Parser::interpreter( parser ){
	Args args( parser );
	this->ErrorCheckToken( Token::Type::Lparen );
	int continuePos = this->GetWritePos();
	expression e( parser );
	int whileJmpPos = this->WriteJZ();
	this->ErrorCheckNextToken( Token::Type::Rparen );
	this->Next();
	this->Parse( &args );
	this->WriteJ( continuePos );
	this->WriteJmpPos( whileJmpPos );
	int breakPos = this->GetWritePos();
	args.WriteBreak( breakPos );
	args.WriteContinue( continuePos );
};

// continueは
// ・for
// ・while
// 文内でしか使用できない文であるため、必ずパラメータがやってくる（来ない場合は処理上おかしい）
// そのため、まず最初にparamの有効チェックを行う。
// パラメータがある場合、そのポインタに現在の書き込み位置をプッシュする
Parser::parse_continue::parse_continue( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	assert( args );
	JumpInfo jumpInfo;
	jumpInfo.pos = this->WriteJ();
	args->Continue.push_back( jumpInfo );
	this->ErrorCheckToken( Token::Type::Semicolon );
};

// breakは
// ・for
// ・while
// ・switch
// 文内でしか使用できない文であるため、必ずパラメータがやってくる（来ない場合は処理上おかしい）
// そのため、まず最初にparamの有効チェックを行う。
// パラメータがある場合、そのポインタに現在の書き込み位置をプッシュする
Parser::parse_break::parse_break( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	assert( args );
	JumpInfo jumpInfo;
	jumpInfo.pos = this->WriteJ();
	args->Break.push_back( jumpInfo );
	this->ErrorCheckToken( Token::Type::Semicolon );
};


/*
 * 式評価
 */
Parser::expression::expression( Parser* parser ) : Parser::interpreter( parser ){
	R = 0;
	expression4( this , parser );
	expression3( this , parser );
	expression2( this , parser );
	expression1( this , parser );
}

Parser::expression::expression( Parser* parser , expression* e ) : Parser::interpreter( parser ){
	R = e->R;
	expression4( this , parser );
	expression3( this , parser );
	expression2( this , parser );
	expression1( this , parser );
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
	if( this->NextTokenIf( Token::Type::Assign ) )    isExpression = true;
	if( this->NextTokenIf( Token::Type::AddAssign ) ) isExpression = true;
	if( this->NextTokenIf( Token::Type::SubAssign ) ) isExpression = true;
	if( this->NextTokenIf( Token::Type::MulAssign ) ) isExpression = true;
	if( this->NextTokenIf( Token::Type::RemAssign ) ) isExpression = true;
	if( isExpression ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression4( exp , parser );
		exp->Assign( var , opetype );
	}
}

// 評価1
// ||
Parser::expression1::expression1( expression* exp , Parser* parser ) : expression_base( exp , parser ) {
	if( this->NextTokenIf( Token::Type::LogicalOr ) ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression4( exp , parser );
		expression3( exp , parser );
		exp->CalcStack( opetype );
		expression1( exp , parser );
		expression2( exp , parser );
	}
}

// 評価2
// &&
Parser::expression2::expression2( expression* exp , Parser* parser ) : expression_base( exp , parser ) {
	if( this->NextTokenIf( Token::Type::LogicalAnd ) ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression4( exp , parser );
		expression3( exp , parser );
		exp->CalcStack( opetype );
		expression2( exp , parser );
		expression1( exp , parser );
	}
}

// 評価3
// !=
// ==
// >=
// <=
// >
// <
Parser::expression3::expression3( expression* exp , Parser* parser ) : expression_base( exp , parser ) {
	bool isExpression = false;
	if( this->NextTokenIf( Token::Type::Equal ) )    isExpression = true;
	if( this->NextTokenIf( Token::Type::NotEqual ) ) isExpression = true;
	if( this->NextTokenIf( Token::Type::GEq ) )      isExpression = true;
	if( this->NextTokenIf( Token::Type::Greater ) )  isExpression = true;
	if( this->NextTokenIf( Token::Type::LEq ) )      isExpression = true;
	if( this->NextTokenIf( Token::Type::Lesser ) )   isExpression = true;
	if( isExpression ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression4( exp , parser );
		exp->CalcStack( opetype );
	}
}

/*
 * +
 * -
 */
Parser::expression4::expression4( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	expression5 expr( exp , parser );
	while( this->NextTokenIf( Token::Type::Add ) || this->NextTokenIf( Token::Type::Sub ) ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression5( exp , parser );
		exp->CalcStack( opetype );
	}
}

/*
 * *
 * /
 * %
 */
Parser::expression5::expression5( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	expression6 expr( exp , parser );
	while( this->NextTokenIf( Token::Type::Mul ) || this->NextTokenIf( Token::Type::Div ) || this->NextTokenIf( Token::Type::Rem ) ){
		this->Next();
		const int& opetype = this->getTokenType();
		expression6( exp , parser );
		exp->CalcStack( opetype );
	}
}

/*
 * Token
 */
Parser::expression6::expression6( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	if( this->NextTokenIf( Token::Type::VariableSymbol ) ){
		this->Next();
		expression_variable( exp , parser );
	}
	else if( this->NextTokenIf( Token::Type::RefSymbol ) ){
		this->Next();
		expression_variable( exp , parser );
	}
	else if( this->NextTokenIf( Token::Type::Digit ) ){
		this->Next();
		this->ExprPushData( this->getTokenDouble() );
	}
	else if( this->NextTokenIf( Token::Type::String ) ){
		this->Next();
		this->ExprPushData( this->getTokenString() );
	}
	else if( this->NextTokenIf( Token::Type::Letter ) ){
		this->Next();
		expression_func( exp , parser );
	}
	else if( this->NextTokenIf( Token::Type::Lbracket ) ){
		this->Next();
	}
	else if( this->NextTokenIf( Token::Type::Lparen ) ){
		this->Next();
		expression e( parser , exp );
		exp->Clone( &e );
		if( this->NextTokenIf( Token::Type::Rparen ) ){
			this->Next();
		}
	}
	else if( this->NextTokenIf( Token::Type::Not ) ){
		this->Next();
		expression6(exp,parser);
		exp->WriteNot();
	}
	else if( this->NextTokenIf( Token::Type::Sub ) ){
		this->Next();
		expression6(exp,parser);
		exp->WriteMinus();
	}
}

/* 
 * 変数シンボル
 */
Parser::expression_variable::expression_variable( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	this->expr = exp;
	this->type = NULL;
	this->exp();
}

/*
 * 構造体変数の場合
 */
Parser::expression_variable::expression_variable( expression* exp , Parser* parser , var_chain& v , Type* t ) : Parser::expression_base( exp , parser ) {
	this->var = v;
	this->expr = exp;
	this->type = t;
	this->exp();
}

void Parser::expression_variable::exp(){
	if( this->NextTokenIf( Token::Type::Letter ) ){
		this->Next();
		string symbolName = this->getTokenString();
		SymbolInfo* symbol = this->getSymbolInScopeOrType( this->type , symbolName );
		if( !symbol ){
			if( this->type ){
				throw VMError( new ERROR_INFO_C2065( symbolName ) );
			}
			symbol = this->addSymbol( symbolName );
			this->Log( "シンボル生成 : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );
		}
		varinfo current( symbol );
		this->var.push( current );
		if( this->NextTokenIf( Token::Type::Dot ) ){
			this->Next();
			this->dot( symbolName );
		}
		else if( this->NextTokenIf( Token::Type::Lbracket ) ){
			this->Next();
			this->bracket( symbolName );
		}
		else if( this->NextTokenIf( Token::Type::Lparen ) ){
			this->Next();
			this->memberFunc( symbolName );
		}
		else if( this->NextTokenIf( Token::Type::As ) ){
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
	SymbolInfo* instSymbol = this->getSymbolInScopeOrType( this->type , symbolName );
	if( !instSymbol ){
		throw VMError( new ERROR_INFO_C2065( symbolName ) );
	}
	expression_variable( expr , m_parser , this->var , instSymbol->getType() );
}

void Parser::expression_variable::bracket( const string& symbolName ){
	expression_bracket( this->expr , this->m_parser , this->type , this->var );
}

void Parser::expression_variable::memberFunc( string& symbolName ){
	if( !this->isExistSymbolInType( symbolName ) ){ throw VMError( new ERROR_INFO_C2065( symbolName ) ); }
	varinfo inst( this->getThis() );
	this->expr->PushThis( inst );
	while( !this->NextTokenIf( Token::Type::Rparen ) ){
		expression4( this->expr , m_parser );
		if( this->NextTokenIf( Token::Type::Comma ) ){
			this->Next();
		}
		this->expr->Push();
	}
	this->Next();
	this->expr->CallFunction( symbolName );
}

Parser::expression_bracket::expression_bracket( expression* exp , Parser* parser , Type* type , var_chain& var ) : Parser::expression_base( exp , parser ) {
	expression4( exp , this->m_parser );
	this->ErrorCheckNextToken( Token::Type::Rbracket );
	varinfo& current = var.peek();
	current.Index( exp->R-1 );	
	if( this->NextTokenIf( Token::Type::Dot ) ){
		this->Next();
		SymbolInfo* instSymbol = this->getSymbolInScopeOrType( type , ((SymbolInfo*)current)->Name() );
		expression_variable( exp , this->m_parser , var , type );
	}
	else {
		expression0( exp , m_parser , var );
		exp->ExprPushData( var );
	}
}

Parser::expression_func::expression_func( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ){
	string funcName = this->getTokenString();
	if( !this->NextTokenIf( Token::Type::Lparen ) ){
		this->Next();
		throw VMError( new ERROR_INFO_C2059( this->getTokenString() ) );
	}

	this->Next();
	while( !this->NextTokenIf( Token::Type::Rparen ) ){
		expression4( exp , parser );
		expression3( exp , parser );
		expression2( exp , parser );
		expression1( exp , parser );
		if( this->NextTokenIf( Token::Type::Comma ) ){
			this->Next();
		}
		exp->Push();
	}
	this->Next();
	exp->CallFunction( funcName );
}


// 解析処理
// 各ステートメントの処理を行う
void Parser::parse( Args* args ){
	const Token& Next = getToken();
	switch( Next.type ){
	case Token::Type::Function :
		{
			this->nextToken();
			parse_function( this );
		}
		break;
	case Token::Type::VariableSymbol :
	case Token::Type::RefSymbol :
		{
			this->backToken();
			parse_variable( this );
		}
		break;
	case Token::Type::Letter :
		{
			this->backToken();
			expression e( this );
		}
		break;
	case Token::Type::Inc :
	case Token::Type::Dec :
		break;
	case Token::Type::Switch :
		//_parse_switch( param );
		break;
	case Token::Type::For :
		this->nextToken();
		parse_for( this );
		break;
	case Token::Type::While :
		this->nextToken();
		parse_while( this );
		break;
	case Token::Type::If :
		this->nextToken();
		parse_if( this , args );
		break;
	case Token::Type::Continue :
		this->nextToken();
		parse_continue( this , args );
		break;
	case Token::Type::Break :
		this->nextToken();
		parse_break( this , args );
		break;
	case Token::Type::YIELD :
		break;
	case Token::Type::Return :
		parse_return( this );
		break;
	case Token::Type::Struct :
		this->nextToken();
		parse_struct( this );
		break;
	case Token::Type::Namespace :
		break;
	case Token::Type::BeginChunk :
		this->nextToken();
		parse_chunk( this , args );
		break;
	case Token::Type::EndChunk : 
		break;
	}
	nextToken();
}

int Parser::getFuncAddres( string& funcName ){
	struct funcinfoS{
		unsigned int address : 24;
		unsigned int type    :  8;
	};
	union {
		funcinfoS info;
		int int_value;
	} result;

	result.info.address = 0;
	result.info.type    = 0;
	int addr = this->m_asm->find( funcName );
	if( addr < 0 ){
		if( this->m_built_in ){
			addr = this->m_built_in->find( funcName );
			result.info.type = 1;
		}
		if( addr < 0 ){
			throw VMError( new ERROR_INFO_C2065( funcName ) );
		}
	}

	result.info.address = addr;
	return result.int_value;
}

} // namespace Assembly
} // namespace VM
} // namespace Sencha
