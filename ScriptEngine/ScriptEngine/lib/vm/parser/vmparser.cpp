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
// ��O�̃g�[�N���ɖ߂�
const TOKEN& Parser::backToken(){
	m_pos--;
	return getToken();
}
// ���̃g�[�N���֐i�߂�
const TOKEN& Parser::nextToken(){
	m_pos++;
	return getToken();
}
// ���݂̃g�[�N�����擾����
const TOKEN& Parser::getToken(){
	if( m_pos >= m_tokens.size() ) return EndToken;
	return m_tokens[m_pos];
}
// ���݂̈ʒu����I�t�Z�b�g�l���v�Z�����ꏊ�ɂ���g�[�N�����擾
const TOKEN& Parser::getToken(int ofs){
	size_t pos = m_pos + ofs;
	if( pos >= m_tokens.size() ) return EndToken;
	return m_tokens[pos];
}
// ���̃g�[�N������H
bool Parser::hasNext(){
	if( m_pos >= m_tokens.size() ) return false;
	return true;
}
// �w�萔�g�[�N����i�߂�
void Parser::_consume( int consumeCount ){
	m_pos += consumeCount;
}
// �v�Z�Ɏg�p����V���{���A���e�����Ȃǂ��X�^�b�N�ɐς�
void Parser::_pushOperation( OperationStack item ){
	m_operationStack.push( item );
}
// �V���{���A���e�������ォ��߂�
OperationStack Parser::_popOperation(){
	PARSER_ASSERT( m_operationStack.size() > 0 );
	OperationStack result = m_operationStack.top();
	m_operationStack.pop();
	return result;
}
// ��͊J�n
void Parser::_execute(){
	try
	{
		while( hasNext() ){
			_parse( NULL );
		}
		//_entryFunction( "global" , 0 );
	}
	catch( VMError& error )
	{
		std::cout << error.getMessage() << std::endl;
		m_assemblyCollection.clear();
	}
}

/*
 * �ϐ����
 */
Parser::parse_variable::parse_variable( Parser* parser ) : Parser::interpreter( parser ){
	expression e( parser );
}

/*
 * as�ɂ��f�[�^�^�t��
 */
Parser::parse_as::parse_as( Parser* parser , varinfo& var ) : Parser::interpreter( parser ){
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		Type* t = this->getType();
		((SymbolInfo*)var)->setType( t );
	}
}

/*
 * �z��^�ł���ꍇ�]��
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
 * function����
 * "function"�̎��̃g�[�N������J�n����B
 * �֐������
 */
Parser::parse_function::parse_function( Parser* parser ) : Parser::interpreter( parser ){
	this->funcName = this->getTokenString();
	this->GoToFunction( funcName );
	this->ErrorCheckNextToken( TokenType::Lparen );
	this->Next();
	this->This();
	this->ParseWhile( TokenType::Rparen );
	this->ErrorCheckNextToken( TokenType::BeginChunk );
	this->Next();
	this->args = this->GetArgs();
	Context param;
	param.symbolArgs = args;
	this->ParseWhile( TokenType::EndChunk , &param );
	this->WhiteEndFunc();
	this->EntryFunction();
	this->GoToBack();
}

/*
 * �֐��o�^
 */
void Parser::parse_function::EntryFunction(){
	AssemblyInfo funcAssembly;
	funcAssembly.setName( funcName );
	funcAssembly.setArgs( args.size() );
	funcAssembly.setStackFrame( this->GetStackFrame() );
	funcAssembly.setBytes( this->GetCurrentStream() );
	this->EntryAssembly( funcAssembly );
}

/*
 * �\���̃X�R�[�v�̏ꍇ
 * ���݂̊֐��X�R�[�v�͍\���̃����o�֐��A�Ƃ��������Ƃ��Ĉ���
 * �ÖٓI��this�V���{�����`����
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
 * struct�錾���
 * "struct"�̎������͂��s��
 */
Parser::parse_struct::parse_struct( Parser* parser ) : Parser::interpreter( parser ){
	string name = this->getTokenString();
	this->ErrorCheckNextToken( TokenType::BeginChunk );
	SymbolInfo* symbol = this->addSymbol( name );
	Context param;
	Type* t = this->GoToStruct( name );
	this->Next();
	this->ParseWhile( TokenType::EndChunk , &param );
	symbol->setType( t );
	this->GoToBack();
}

/*
 * ���]��
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
 * �ϐ��V���{��
 */
Parser::expression_variable::expression_variable( expression* exp , Parser* parser ) : Parser::expression_base( exp , parser ) {
	this->expr = exp;
	this->type = NULL;
	this->exp();
}

Parser::expression_variable::expression_variable( expression* exp , Parser* parser , var_chain& v , Type* t ) : Parser::expression_base( exp , parser ) {
	this->var = v;
	this->expr = exp;
	this->type = t;
	this->exp();
}

void Parser::expression_variable::exp(){
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		string symbolName = this->getTokenString();
		SymbolInfo* symbol = this->getSymbolInScopeOrType( this->type , symbolName );
		if( !symbol ){
			if( this->type ){
				throw VMError( new ERROR_INFO_C2065( symbolName ) );
			}
			symbol = this->addSymbol( symbolName );
			printf( "�V���{������ : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );
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
	SymbolInfo* instSymbol = this->getSymbolInScopeOrType( this->type , symbolName );
	if( !instSymbol ){
		throw VMError( new ERROR_INFO_C2065( symbolName ) );
	}
	expression_variable( expr , m_parser , this->var , instSymbol->getType() );
}

void Parser::expression_variable::bracket( const string& symbolName ){
	expression_bracket( this->expr , this->m_parser , this->type , this->var );
}

void Parser::expression_variable::memberFunc( const string& symbolName ){
	if( !this->isExistSymbolInType( symbolName ) ){ throw VMError( new ERROR_INFO_C2065( symbolName ) ); }
	varinfo inst( this->getThis() );
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

Parser::expression_bracket::expression_bracket( expression* exp , Parser* parser , Type* type , var_chain& var ) : Parser::expression_base( exp , parser ) {
	expression1( exp , this->m_parser );
	this->ErrorCheckNextToken( TokenType::Rbracket );
	varinfo& current = var.peek();
	current.Index( exp->R-1 );	
	if( this->NextTokenIf( TokenType::Dot ) ){
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

// ��͏���
// �e�X�e�[�g�����g�̏������s��
void Parser::_parse( Context* param ){
	const TOKEN& Next = getToken();
	switch( Next.type ){
	case TokenType::Function :
		this->nextToken();
		parse_function( this );
		break;
// **************************************************************
// $xxx�Ȃǂ̒�`
// ����`�Ȃ�V���{���ǉ�����
// **************************************************************
	case TokenType::VariableSymbol :
	case TokenType::RefSymbol :
		this->backToken();
		parse_variable( this );
		break;
	case TokenType::Inc :
	case TokenType::Dec :
	case TokenType::AsString :
	case TokenType::AsInteger :
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
		this->nextToken();
		parse_struct( this );
		break;
	case TokenType::Namespace :
		break;
	case TokenType::Letter :
		break;
	case TokenType::BeginChunk :
		_parse_chunk( param );
		break;
	case TokenType::EndChunk : 
		break;
	}
	nextToken();
}

// continue��
// �Efor
// �Ewhile
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
void Parser::_parse_continue( Context* param ){
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

// break��
// �Efor
// �Ewhile
// �Eswitch
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
void Parser::_parse_break( Context* param ){
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

// return�����
// return�̎��̌v�Z����]�����A���̌��ʂ��i�[����
// ��{�I�ɂ�R0�Ԃɂ����߂�l�͓���Ȃ�
// return����艺�ɂ͍s���Ă͂����Ȃ��̂Ŕ��������炻���Ŋ֐��I��
void Parser::_parse_return( Context* param ){
	PARSER_ASSERT( param );
	PARSER_ASSERT( getToken().type == TokenType::Return );
	nextToken();
//	_expression( param );

	m_writer->write( EMnemonic::RET );
	m_writer->write( EMnemonic::REG );
	m_writer->writeInt32( this->m_R );
	m_writer->write( EMnemonic::EndFunc );

	PARSER_ASSERT( getToken().type  == TokenType::Semicolon );
	if( getToken().type  == TokenType::Semicolon ){
		nextToken();
	}
}


// �`�����N����
// �I���`�����N�𔭌�����܂Ń��[�v����
void Parser::_parse_chunk( Context* param ){
//#define CHUNK_PRINT printf
#define CHUNK_PRINT VM_PRINT
	m_currentScope = m_currentScope->goToChildScope( "__chunk__" );

	CHUNK_PRINT( ">>�`�����N ��͊J�n\n" );
	PARSER_ASSERT( getToken().type == TokenType::BeginChunk );
	nextToken();
	while( hasNext() ){
		_parse( param );
		if( getToken().type == TokenType::EndChunk ){
			nextToken();
			break;
		}
	}

	CHUNK_PRINT( ">>�`�����N ��͏I�� [���݂̃g�[�N��:%s][�����V���{����:%d][%d][%d][%d]\n" , 
		getToken().text.c_str() , 
		m_currentScope->getSymbolCountInScopeAttribute() , 
		m_currentScope->getParentSymbolCount( VariableLocal ) , 
		m_currentScope->getAllParentSymbolCount( VariableLocal ) ,
		m_currentScope->getAllSymbolCount( VariableLocal ) );
	m_currentScope = m_currentScope->backToChildScope();
#undef CHUNK_PRINT
}

// '('����Ή�����')'��������܂ŃX�L�b�v
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
