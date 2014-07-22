#define FOR_DEBUG
#ifdef FOR_DEBUG
	#define FOR_LOG    VM_PRINT
	#define FOR_ASSERT VM_ASSERT

//	#define FOR_LOG    printf
//	#define FOR_ASSERT assert
#else
	#define FOR_LOG(...)
	#define FOR_ASSERT(...)
#endif

#include "vmscope.h"
#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"


namespace SenchaVM {
namespace Assembly {

// **************************************************************************
// for�����
// 3�ӏ��̌v�Z���s��
//		1.�ŏ�����
//		2.�I������
//		3.���[�v�I��������
// �]�����ʂ�0�̏ꍇ�͏I���`�����N�܂ŃW�����v
// �����łȂ��ꍇ�̓W�����v���Ȃ�
// �`�����N�I���n�_�ɊJ�n�n�_�܂ł̃W�����v������
// **************************************************************************
void Parser::_parse_for( ParseParameter* param ){
	m_currentScope = m_currentScope->goToChildScope( "__for__" );

	FOR_LOG( ">>for Log\n" );
	FOR_ASSERT( getToken().type == TokenType::For );    nextToken();
	FOR_ASSERT( getToken().type == TokenType::Lparen ); nextToken();
	// �ŏ��̏���
	_parse( param );

	// ���[�v�I������
	int forContinuePos = m_writer->count();
	_expression( param );
	
	FOR_ASSERT( getToken().type == TokenType::Semicolon );
	// ��r���Z���ʂ�0�̏ꍇ�I���`�����N�܂ŃW�����v���閽�߂�ݒu
	m_writer->write( EMnemonic::JumpZero );
	int forJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );

	nextToken();

	// �Ō㏈��
	// �ʒu���L�����Ă����Ĕ�΂�
	int lastExpressionPos = this->m_pos;
	_skipParen();
	FOR_ASSERT( getToken().type == TokenType::Rparen );

	// ')'���΂�
	nextToken();
	// ���̂P���߂����(�`�����N������ꍇ�͏I���`�����N�܂ł��̂܂܉�͂���悤�ɂ���)
	ParseParameter forParam;

	FOR_LOG( ">>for Log ����������͊J�n\n" );
	_parse( &forParam );
	FOR_LOG( ">>for Log ����������͏I�� [%s]\n" , getToken().text.c_str() );

	int processEndPos = this->m_pos;

	// �ʒu��߂��čŌ�̌v�Z����]��
	this->m_pos = lastExpressionPos;
	int loopEndPos = m_writer->count();
	_expression( param );

	// �Ō�̌v�Z���I������猳�̈ʒu�ɖ߂�
	m_pos = processEndPos;

	// ���[�v�I���������ʒu�܂ŃW�����v
	m_writer->write( EMnemonic::Jmp );
	m_writer->writeInt32( forContinuePos );

	// ���[�v�I���ʒu����������
	m_writer->writeInt32( m_writer->count() , forJmpPos );

	m_currentScope = m_currentScope->backToChildScope();
	// continue,break������������W�����v�A�h���X��ݒ�
	// for���ł�continue���ɍŌ㏈�������Ă��烋�[�v�擪�ʒu�ɖ߂��_�ɒ���
	for( size_t i = 0 ; i < forParam.breakAddr.size() ; i++ ){ m_writer->writeInt32( m_writer->count() , forParam.breakAddr[i].codeAddr ); }
	for( size_t i = 0 ; i < forParam.continueAddr.size() ; i++ ){ m_writer->writeInt32( loopEndPos , forParam.continueAddr[i].codeAddr ); }
}

} // namespace Assembly
} // namespace SenchaVM