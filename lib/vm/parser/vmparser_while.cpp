#define WHILE_DEBUG
#ifdef WHILE_DEBUG
	#define WHILE_LOG VM_PRINT
	#define WHILE_ASSERT VM_ASSERT

//	#define WHILE_LOG printf
//	#define WHILE_ASSERT VM_ASSERT
#else
	#define WHILE_LOG(...)
	#define WHILE_ASSERT(...)
#endif


#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"


namespace SenchaVM {
namespace Assembly {
// **************************************************************************
// while�����
// ���@�`�F�b�N���s���v�Z���]���B
// �]�����ʂ�0�̏ꍇ�͏I���`�����N�܂ŃW�����v
// �����łȂ��ꍇ�̓W�����v���Ȃ�
// �`�����N�I���n�_�ɏI���n�_�܂ł̃W�����v������
// **************************************************************************
void Parser::_parse_while( ParseParameter* param ){
	WHILE_LOG( ">>while Log\n" );
	WHILE_ASSERT( getToken().type == TokenType::While );  nextToken();
	WHILE_ASSERT( getToken().type == TokenType::Lparen ); nextToken();


	// ���[�v�I������
	int whileContinuePos = m_writer->count();
	_expression( param );

	WHILE_ASSERT( getToken().type == TokenType::Rparen );
	WHILE_LOG( ">>while Log �I��\n" );
	
	// ��r���Z���ʂ�0�̏ꍇ�I���`�����N�܂ŃW�����v���閽�߂�ݒu
	m_writer->write( EMnemonic::JumpZero );
	int whileJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );

	nextToken();
	ParseParameter whileParam;

	WHILE_LOG( ">>while Log ����������͂�...\n" );
	_parse( &whileParam );
	WHILE_LOG( ">>while Log ����������͏I�� %s \n" , getToken().text.c_str() );

	m_writer->write( EMnemonic::Jmp );
	m_writer->writeInt32( whileContinuePos );

	// ���[�v�I���ʒu����������
	m_writer->writeInt32( m_writer->count() , whileJmpPos );

	// continue,break������������W�����v�A�h���X��ݒ�
	for( size_t i = 0 ; i < whileParam.breakAddr.size() ; i++ ){ m_writer->writeInt32( m_writer->count() , whileParam.breakAddr[i].codeAddr ); }
	for( size_t i = 0 ; i < whileParam.continueAddr.size() ; i++ ){ m_writer->writeInt32( whileContinuePos , whileParam.continueAddr[i].codeAddr ); }
}


} // namespace Assembly
} // namespace SenchaVM
