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
	if( ( ofs + m_pos ) >= ( ofs + m_tokens.size() ) ) return EndToken;
	return m_tokens[ofs+m_pos];
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
		_entryFunction( "global" , 0 );
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
Parser::variable::variable( Parser* parser ) : Parser::interpreter( parser ){
	bool isReference = this->TokenIf( TokenType::RefSymbol );
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		SymbolInfo* symbol = this->getSymbol();
		symbol->IsReference( isReference );
		if( this->NextTokenIf( TokenType::As ) ){
			this->Next();
			as a( parser , symbol );
		}
		expression expr( parser , symbol );
	}
}

/*
 * as�ɂ��f�[�^�^�t��
 */
Parser::as::as( Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	if( this->NextTokenIf( TokenType::Array ) ){
		this->Next();
		parse_array ary( parser , symbol );
	}
	else if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		SymbolInfo* dataSymbol = this->getSymbol();
		symbol->setClass( dataSymbol );
		symbol->copyAndAddChildrenOfSymbol( dataSymbol );
		symbol->setupChildrenAddresToParentAddresOffset();
		this->Next();
	}
	else{
	}
}

/*
 * �z��^�ł���ꍇ�]��
 */
Parser::parse_array::parse_array( Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	assert( this->NextTokenIf( TokenType::Lparen ) );
	this->Next();
	if( this->NextTokenIf( TokenType::Letter ) ){
		this->Next();
		as a( parser , symbol );
	}
	if( this->NextTokenIf( TokenType::Digit ) ){
		this->Next();
		int arrayLength = this->getTokenInt();
		this->Next();
		assert( this->TokenIf( TokenType::Rparen ) );
		this->Next();
		assert( this->TokenIf( TokenType::Semicolon ) );
		this->Next();
		symbol->ArrayLength( arrayLength );
	}
}

/*
 * ���]��
 */
Parser::expression::expression( Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	R = 0;
	expression0 expr( this , parser , symbol );
}

Parser::expression::expression( expression* prev , Parser* parser , SymbolInfo* symbol ) : Parser::interpreter( parser ){
	if( prev ){
		R = prev->R;
	}
	expression0 expr( this , parser , symbol );
}


/*
 * =
 * +=
 * -=
 * *=
 * /=
 * %=
 */
Parser::expression0::expression0( expression* exp , Parser* parser , SymbolInfo* symbol ) : Parser::expression_base( exp , parser ) {
	bool isAssignExpression = false;
	if( this->NextTokenIf( TokenType::Assign ) )    isAssignExpression = true;
	if( this->NextTokenIf( TokenType::AddAssign ) ) isAssignExpression = true;
	if( this->NextTokenIf( TokenType::SubAssign ) ) isAssignExpression = true;
	if( this->NextTokenIf( TokenType::MulAssign ) ) isAssignExpression = true;
	if( this->NextTokenIf( TokenType::RemAssign ) ) isAssignExpression = true;
	if( isAssignExpression ){
		this->Next();
		const TOKEN_TYPE& opetype = this->getTokenType();
		expression1 expr( exp , parser , symbol );
	}
}

/*
 * +
 * -
 */
Parser::expression1::expression1( expression* exp , Parser* parser , SymbolInfo* symbol ) : Parser::expression_base( exp , parser ) {
	expression2 expr( exp , parser , symbol );
}

/*
 * *
 * /
 * %
 */
Parser::expression2::expression2( expression* exp , Parser* parser , SymbolInfo* symbol ) : Parser::expression_base( exp , parser ) {
	expression3 expr( exp , parser , symbol );
}

/*
 * TOKEN
 */
Parser::expression3::expression3( expression* exp , Parser* parser , SymbolInfo* symbol ) : Parser::expression_base( exp , parser ) {
	if( this->NextTokenIf( TokenType::VariableSymbol ) ){
		this->Next();
	}
	else if( this->NextTokenIf( TokenType::RefSymbol ) ){
		this->Next();
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
	}
	else if( this->NextTokenIf( TokenType::AsString ) ){
		this->Next();
	}
	else if( this->NextTokenIf( TokenType::AsInteger ) ){
		this->Next();
	}
}


// ��͏���
// �e�X�e�[�g�����g�̏������s��
void Parser::_parse( ParseParameter* param ){
	const TOKEN& token = getToken();

	switch( token.type ){
	case TokenType::Function :
		_parse_function( param );
		break;
// **************************************************************
// $xxx�Ȃǂ̒�`
// ����`�Ȃ�V���{���ǉ�����
// **************************************************************
	case TokenType::VariableSymbol :
	case TokenType::RefSymbol :
		variable( this );
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
}

// continue��
// �Efor
// �Ewhile
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
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

// break��
// �Efor
// �Ewhile
// �Eswitch
// �����ł����g�p�ł��Ȃ����ł��邽�߁A�K���p�����[�^������Ă���i���Ȃ��ꍇ�͏����エ�������j
// ���̂��߁A�܂��ŏ���param�̗L���`�F�b�N���s���B
// �p�����[�^������ꍇ�A���̃|�C���^�Ɍ��݂̏������݈ʒu���v�b�V������
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

// return�����
// return�̎��̌v�Z����]�����A���̌��ʂ��i�[����
// ��{�I�ɂ�R0�Ԃɂ����߂�l�͓���Ȃ�
// return����艺�ɂ͍s���Ă͂����Ȃ��̂Ŕ��������炻���Ŋ֐��I��
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

// �ϐ��錾�E�ϐ��ւ̑�����߂Ȃ�
// $�����������ꍇ�A�������� ','�ɑ����ĕϐ������݂���ꍇ�Ă΂�� 
void Parser::_parse_variable( ParseParameter* param ){
	using namespace Assembly;
	const TOKEN& t_0 = getToken(0);
	const TOKEN& t_1 = getToken(1);
	const TOKEN& t_2 = getToken(2);
	const TOKEN& t_3 = getToken(3);

	//VM_PRINT( "���݉�͒��̃X�R�[�v�ɕϐ�[%s]�V���{����ǉ�\n" , t_1.text.c_str() );
	// $VariableName
	//	 			 = | 
	//				+= | 
	//				-= |
	//				*= | 
	//				/= | 
	//				%=
	if( _isVariable( t_0 , t_1 ) ){
		// m_currentScope ... ����͌��݂̃X�R�[�v�C���X�^���X
		// �X�R�[�v�͓���q�ɂȂ�̂ŃX�R�[�v�N���X�ɂ͎q�X�R�[�v�̃|�C���^����������
		// ���݂̃X�R�[�v����X�Ɏq�K�w�̃X�R�[�v�ֈړ�������ꍇ�́A�q�X�R�[�v�𐶐����J�����g�X�R�[�v�̎Q�Ƃ��ڂ�(�߂��Ă����Ƃ��Ɍ��̈ʒu�ɖ߂��悤�ɃX�^�b�N�ŕۑ�����)
		// �q�X�R�[�v�͐e�X�R�[�v�̎Q�Ƃ������A�V���{���������͐e�X�R�[�v���猟������
		// �e�X�R�[�v�ɓ����̃V���{�������݂���ꍇ�A�����D��I�Ɉ���
		// if,while,switch,for�͒�`���ꂽ�^�C�~���O�ŃX�R�[�v���ڂ�

		// getSymbol�̓V���{�������w�肵�A�Y������V���{���C���X�^���X��ԋp������ɂ���
		SymbolInfo* symbol = m_currentScope->getSymbol( t_1.text );

		// ���݂��Ȃ��V���{���̏ꍇ�͂��̏�Ő���
		if( !symbol ){
			symbol = m_currentScope->addSymbol( t_1.text );
			VM_PRINT( "�V���{������ : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );
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


// �f�[�^�^�t������
// �Ώۂ̃V���{���Ɏw��̃f�[�^�^�V���{����V�K�ɍ쐬���Ă���
// �e�V���{���ɂ͐擪���琔�����A�h���X�����蓖�Ă�
// todo:: �֐����������ق����ǂ�
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
	nextToken(); // "$"��΂�
	nextToken(); // �V���{����΂�
	nextToken(); // "as"��΂�

	// *************************************************
	// as array�Ŕz��Ƃ��ď�������
	// *************************************************
	if( getToken().type == TokenType::Array ){
		nextToken(); // array��΂�
		AS_ASSERT( getToken().type == TokenType::Lparen );
		nextToken(); // '('��΂�

		// *************************************************
		// �f�[�^�^�ł���ꍇ
		// *************************************************
		if( getToken().type == TokenType::Letter ){
			string dataTypeName = getToken().text;
			SymbolInfo* dataSymbol = m_currentScope->getSymbol( dataTypeName );
			AS_ASSERT( dataSymbol );
			symbol->setClass( dataSymbol );
			symbol->copyAndAddChildrenOfSymbol( dataSymbol );
			symbol->setupChildrenAddresToParentAddresOffset();

			nextToken(); // LETTER��΂�
			AS_ASSERT( getToken().type == TokenType::Comma );
			nextToken(); // ','��΂�
		}

		// *************************************************
		// �z��ԍ�
		// *************************************************
		AS_ASSERT( getToken().type == TokenType::Digit );
		int arrayLength = atoi( getToken().text.c_str() );
		nextToken(); // ������΂�
		AS_ASSERT( getToken().type == TokenType::Rparen );
		nextToken(); // ')'��΂�
		AS_ASSERT( getToken().type == TokenType::Semicolon );
		nextToken(); // ';'��΂�

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


// �����m�F
// �֐��ł���ꍇ�A�]�����s��
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
//	�\���̉��
//	--------------------------------------------------------
void Parser::_parse_struct( ParseParameter* param ){
//#define CLASS_PRINT(...)
#define CLASS_PRINT VM_PRINT
//#define CLASS_PRINT printf
	CLASS_PRINT( "�\���� ��͊J�n\n" );
	TOKEN className = nextToken();
	PARSER_ASSERT( className.type == TokenType::Letter );
	PARSER_ASSERT( className.text.size() > 0 );
	CLASS_PRINT( "�\���̖� : %s \n" , className.text.c_str() );
	TOKEN beginChunk = nextToken();
	PARSER_ASSERT( beginChunk.type == TokenType::BeginChunk );
	nextToken();
	// �N���X�V���{���͏d�����Ă͂����Ȃ��̂�
	// �Ƃ肠�����������Ă݂�
	SymbolInfo* symbol = m_currentScope->getSymbol( className.text );
	assert( !symbol );
	symbol = m_currentScope->addSymbol( className.text );
	CLASS_PRINT( "�\���̃V���{������ : %s [addr %d]\n" , symbol->Name().c_str() , symbol->Addr() );


	ParseParameter classParam;
	m_currentScope = m_currentScope->goToStructScope( symbol->Name() );
	int scopeLevel = m_currentScope->ScopeLevel();

	while( hasNext() ){
		_parse( &classParam );
		if( getToken().type == TokenType::EndChunk ){
			nextToken();
			break;
		}
	}
	const vector<SymbolInfo*>& structFields = m_currentScope->getChildren();
	symbol->Addr(0);
	symbol->copyAndAddChildrenOfSymbol( structFields );
	//dumpSYMBOL( symbol , 1 );
	m_currentScope = m_currentScope->backToChildScope();

#undef CLASS_PRINT
}


// �`�����N����
// �I���`�����N�𔭌�����܂Ń��[�v����
void Parser::_parse_chunk( ParseParameter* param ){
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


// *************************************************************************************
// �\���̃X�R�[�v�̏ꍇ
// ���݂̊֐��X�R�[�v�͍\���̃����o�֐��A�Ƃ��������Ƃ��Ĉ���
// �ÖٓI��this�V���{�����`����
// �֐�����"className##methodName"�Ƃ����悤�Ɍq����
// *************************************************************************************
static void _set_thisPointer( Scope* parentScope , Scope* currentScope ){
	if( !parentScope ){
		return;
	}
	if( parentScope->isStructScope() ){
		throw VMError( new ERROR_INFO_3001( currentScope->ScopeName() ) );
	}
/*
	// �e�X�R�[�v���\���̃X�R�[�v�ł���ꍇ�A�\���̃��\�b�h�ł��邱�ƂɂȂ�̂Œʒm
	// "this"�Ƃ������O�̃V���{�����ÖٓI�ɐ�������
	// �N���X�̎������̃V���{�����擾���A�N���X�ɐݒ肷��
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
*/
}


// �֐������
void Parser::_parse_function( ParseParameter* param ){
//#define FUNC_PRINT printf
#define FUNC_PRINT  VM_PRINT
#define FUNC_ASSERT VM_ASSERT

	FUNC_PRINT( ">>�֐� ��͊J�n [ScopeLevel %d]\n" , m_currentScope->ScopeLevel() );
	TOKEN funcName = nextToken();
	Scope* parentScope = m_currentScope;
	string scopeName = funcName.text;
	m_currentScope = m_currentScope->goToFunctionScope( scopeName );

	CBinaryWriter prev = m_writer;
	m_writer = CBinaryWriter( new BinaryWriter() );

	PARSER_ASSERT( funcName.type == TokenType::Letter );
	PARSER_ASSERT( funcName.text.size() > 0 );
	FUNC_PRINT( ">>�֐��� : %s \n" , funcName.text.c_str() );
	TOKEN beginChunk = nextToken();
	PARSER_ASSERT( beginChunk.type == TokenType::Lparen );
	nextToken();

	// �\���̂̏ꍇ"this"���`
	_set_thisPointer( parentScope , m_currentScope );
	// "("����")"�܂ł̉��
	while( getToken().type != TokenType::Rparen ){
		_parse( NULL );
	}
	size_t args = m_currentScope->getSymbolCountMaxInAllScope( VariableLocal );
	for( unsigned int i = 0 ; i < args ; i++ ){
		m_writer->write( EMnemonic::Pop );
	}
	PARSER_ASSERT( getToken().type == TokenType::Rparen )		;	nextToken();
	PARSER_ASSERT( getToken().type == TokenType::BeginChunk )	;	nextToken();
	ParseParameter funcParam;
	funcParam.args = args;
	while( hasNext() ){
		_parse( &funcParam );
		if( getToken().type == TokenType::EndChunk ){
			m_writer->write( EMnemonic::EndFunc );
			nextToken();
			break;
		}
	}
	FUNC_PRINT( ">>�֐� ��͏I�� [ScopeLevel %d][SymbolCount %d]\n" , m_currentScope->ScopeLevel() , m_currentScope->getSymbolCountMaxInAllScope( VariableLocal ) );
	_entryFunction( scopeName , args );
	FUNC_PRINT( "\n" );	
	m_writer = prev;

	m_currentScope = m_currentScope->backToChildScope();
#undef FUNC_PRINT
}


// �֐����̃A�Z���u�������X�g���[���ɏ�������
void Parser::_entryFunction( string funcName , size_t args ){
//#define FUNC_PRINT
#define FUNC_PRINT VM_PRINT
//#define FUNC_PRINT printf
	if( m_currentScope->ScopeLevel() > 0 ) FUNC_PRINT( ">>�֐��o�^ �֐���[%s]\n" , funcName.c_str() );
	else                                   FUNC_PRINT( ">>�O���[�o���X�R�[�v\n" );
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

	if( m_currentScope->ScopeLevel() > 0 ) FUNC_PRINT( ">>���[�J���ϐ�:%d\n" , funcAssembly.stackFrame() );
	else                                   FUNC_PRINT( ">>�ÓI�ϐ�:%d\n"     , funcAssembly.stackFrame() );
#undef FUNC_PRINT
}

void Parser::_entryClass( string className ){
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
