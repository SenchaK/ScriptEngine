#define IF_DEBUG
#ifdef IF_DEBUG
	#define IF_LOG VM_PRINT
	#define IF_ASSERT VM_ASSERT
#else
	#define IF_LOG(...)
	#define IF_ASSERT(...)
#endif

#include "vmparser.h"
#include "..\lexer\vmlexer.h"
#include "..\vmassembler.h"
#include "..\symbol\vmsymbol.h"

namespace SenchaVM {
namespace Assembly {

// **************************************************************************
// if�����
// ���@�`�F�b�N���s���v�Z���]���B
// �]�����ʂ�0�̏ꍇ��else�������܂ŃW�����v
// �����łȂ��ꍇ�̓W�����v���Ȃ�
// �`�����N�I���n�_�ɏI���n�_�܂ł̃W�����v������
// **************************************************************************
void Parser::_parse_if( Context* param ){
	IF_LOG( "**if��\n" );
	IF_ASSERT( getToken().type == TokenType::If );     nextToken();
	IF_ASSERT( getToken().type == TokenType::Lparen ); nextToken();
//	_expression( param );
	IF_ASSERT( getToken().type == TokenType::Rparen );
	
	// ��r���Z���ʂ�0�̏ꍇelse�����ꏊ�܂ŃW�����v���閽�߂�ݒu
	// ���̒i�K�ł̓W�����v��A�h���X���킩��Ȃ��̂�0�����Ă����A�������݈ʒu���L�^���Ă����Č�ŏ㏑������
	m_writer->write( EMnemonic::JumpZero );
	int elseJmpPos = m_writer->count();
	m_writer->writeInt32( 0 );
	//
	// ���̂P���߂����(�`�����N������ꍇ�͏I���`�����N�܂ł��̂܂܉�͂���悤�ɂ���)
	nextToken();
	_parse( param );
	// 
	// �I���`�����N�܂ŃW�����v�����閽�߂�ݒu
	// ��������else������
	// else if�̏ꍇ��_parse_if���Ăяo��
	IF_LOG( "**if���I��\n" );
	m_writer->writeInt32( m_writer->count() , elseJmpPos );

	IF_LOG( "**else�T�� : %s \n" , getToken().text.c_str() );
	while( getToken().type == TokenType::Else ){
		// if���I������else������Ȃ�I���n�_�܂ŃW�����v�����閽�߂�����
		m_writer->write( EMnemonic::Jmp );
		int ifEndJmpPos = m_writer->count();
		m_writer->writeInt32( 0 );
		m_writer->writeInt32( m_writer->count() , elseJmpPos );

		// else�̎��Ɋ��߂ĉ��
		nextToken();
		_parse( param );

		// ���݂̈ʒu���W�����v��A�h���X�Ƃ���
		m_writer->writeInt32( m_writer->count() , ifEndJmpPos );
	}
	IF_LOG( "**else�T���I�� : %s \n" , getToken().text.c_str() );
}

} // namespace Assembly
} // namespace SenchaVM

