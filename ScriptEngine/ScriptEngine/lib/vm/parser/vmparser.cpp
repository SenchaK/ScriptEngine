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

// ��͊J�n
void Parser::_execute(){
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
	this->ParseWhile( TokenType::EndChunk );
	this->WriteEndFunc();
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
	Type* t = this->GoToStruct( name );
	this->Next();
	this->ParseWhile( TokenType::EndChunk );
	symbol->setType( t );
	this->GoToBack();
}

/*
 * �`�����N�\�����
 * �`�����N�̎������͂��n�߂�
 */
Parser::parse_chunk::parse_chunk( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	this->GoToChunk();
	this->ParseWhile( TokenType::EndChunk , args );
	this->GoToBack();
}

/*
 * return�����
 * "return"�����͊J�n
 * return�̎��̌v�Z����]�����A���̌��ʂ��i�[����
 * ��{�I�ɂ�R0�Ԃɂ����߂�l�͓���Ȃ�
 * return����艺�ɂ͍s���Ă͂����Ȃ��̂Ŕ��������炻���Ŋ֐��I��
 */
Parser::parse_return::parse_return( Parser* parser ) : Parser::interpreter( parser ){
	expression e( parser );
	this->WriteReturn( e.R - 1 );
}


/*
 * if�����
 * ���@�`�F�b�N���s���v�Z���]���B
 * �]�����ʂ�0�̏ꍇ��else�������܂ŃW�����v
 * �����łȂ��ꍇ�̓W�����v���Ȃ�
 * �`�����N�I���n�_�ɏI���n�_�܂ł̃W�����v������
 * else�𔭌������ꍇ
 * �Eif��ʂ��Ă����ꍇ�͏I���n�_�܂ŃW�����v
 * �Eif�ŕ]������Ȃ������ꍇ��if�Ɠ����]�����s��
 * �ielse if�̏ꍇ��if�ň���������Aelse�̏ꍇ�͂��̏������]�������j
 */
Parser::parse_if::parse_if( Parser* parser ) : Parser::interpreter( parser ){
	this->ErrorCheckToken( TokenType::Lparen );
	this->Next();
	expression e( parser );
	int IF_JumpPos = this->WriteJZ();

	printf( "jz @IF\n" );
	this->ErrorCheckNextToken( TokenType::Rparen );
	this->Next();
	this->Parse();
	this->WriteJmpPos( IF_JumpPos );
	if( !this->NextTokenIf( TokenType::Else ) ){
		printf( ":IF\n" );
	}
	while( this->NextTokenIf( TokenType::Else ) ){
		this->Next();
		int IF_EndJumpPos = this->WriteJ();
		this->WriteJmpPos( IF_JumpPos );
		printf( "jmp @IF_END\n" );
		printf( ":IF\n" );
		this->Next();
		this->Parse();
		this->WriteJmpPos( IF_EndJumpPos );
		printf( ":IF_END\n" );
	}
}

Parser::parse_switch::parse_switch( Parser* parser ) : Parser::interpreter( parser ){
};

Parser::parse_for::parse_for( Parser* parser ) : Parser::interpreter( parser ){
	this->GoToChunk();
	this->ErrorCheckToken( TokenType::Lparen );
	expression first_expr( parser );
	this->ErrorCheckNextToken( TokenType::Semicolon );
	printf( ":CONTINUE\n" );
	expression loop_end_expr( parser );
	this->ErrorCheckNextToken( TokenType::Semicolon );
	int forJmpPos = this->WriteJZ();
	printf( "jz @FOR\n" );
	CBinaryWriter old = this->CreateNewWriter();
	printf( "--last \n");
	expression last_expr( parser );
	printf( "--\n");

	this->ErrorCheckNextToken( TokenType::Rparen );
	this->Next();
	CBinaryWriter lastExprCode = this->SetWriter( old );
	int continuePos = this->GetWritePos();
	Args args( parser );
	this->Parse( &args );
	this->AppendWriter( lastExprCode );
	this->WriteJ( continuePos );
	printf( "jmp CONTINUE\n" );
	this->WriteJmpPos( forJmpPos );
	printf( ":FOR\n" );
	this->GoToBack();
	int breakPos = this->GetWritePos();
	args.WriteBreak( breakPos );
	args.WriteContinue( continuePos );
};

Parser::parse_while::parse_while( Parser* parser ) : Parser::interpreter( parser ){
	Args args( parser );
	this->ErrorCheckToken( TokenType::Lparen );
	int continuePos = this->GetWritePos();
	printf( ":CONTINUE\n" );
	expression e( parser );
	int whileJmpPos = this->WriteJZ();
	printf( "jz @WHILE\n" );
	this->ErrorCheckNextToken( TokenType::Rparen );
	this->Next();
	this->Parse( &args );
	this->WriteJ( continuePos );
	printf( "jmp CONTINUE\n" );
	this->WriteJmpPos( whileJmpPos );
	printf( ":WHILE\n" );
	int breakPos = this->GetWritePos();
	args.WriteBreak( breakPos );
	args.WriteContinue( continuePos );
};

// continue��
// �Efor
// �Ewhile
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
Parser::parse_continue::parse_continue( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	assert( args );
	JumpInfo jumpInfo;
	jumpInfo.pos = this->WriteJ();
	args->Continue.push_back( jumpInfo );
	this->ErrorCheckToken( TokenType::Semicolon );
};

// break��
// �Efor
// �Ewhile
// �Eswitch
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
Parser::parse_break::parse_break( Parser* parser , Args* args ) : Parser::interpreter( parser ){
	assert( args );
	JumpInfo jumpInfo;
	jumpInfo.pos = this->WriteJ();
	args->Break.push_back( jumpInfo );
	this->ErrorCheckToken( TokenType::Semicolon );
};


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

/*
 * �\���̕ϐ��̏ꍇ
 */
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
void Parser::parse( Args* args ){
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
		//_parse_switch( param );
		break;
	case TokenType::For :
		this->nextToken();
		parse_for( this );
		break;
	case TokenType::While :
		this->nextToken();
		parse_while( this );
		break;
	case TokenType::If :
		this->nextToken();
		parse_if( this );
		break;
	case TokenType::Continue :
		this->nextToken();
		parse_continue( this , args );
		break;
	case TokenType::Break :
		this->nextToken();
		parse_break( this , args );
		break;
	case TokenType::YIELD :
		break;
	case TokenType::Return :
		parse_return( this );
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
		this->nextToken();
		parse_chunk( this , args );
		break;
	case TokenType::EndChunk : 
		break;
	}
	nextToken();
}

} // namespace Assembly
} // namespace SenchaVM
