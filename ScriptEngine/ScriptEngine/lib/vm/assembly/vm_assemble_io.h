#pragma once
#include "../../util/logger.h"
#include "../../util/stream.h"
#include "../vm_define.h"
#include "../parser/vm_parser.h"
#include "vm_assembly_info.h"


namespace Sencha {
namespace VM {
namespace Assembly {
/*
 * ��͌��ʂ��o�C�i���`���̃t�@�C���ɏo�͂���B
 * �����R�[�h��ʃt�@�C���ɂ���p�r�Ŏg�p����B
 *
 * �e�A�Z���u���͈ȉ��̃t�H�[�}�b�g�`���ŕۑ������
 * [AssemblyName : string           ] �A�Z���u���̖��O
 * [StackFrame   : u32              ] �X�^�b�N�t���[��
 * [Address      : u32              ] �֐��A�h���X
 * [Args         : u32              ] �֐��p�����[�^��
 * [CodeSize     : u32              ] �R�[�h�̈�T�C�Y
 * [Code         : byte[ContentSize]] �R�[�h�̈�
 */
class VMAssembleOutput {
private :
	CStream m_stream;
public  :
	VMAssembleOutput( IAssembleReader* reader , const char* fileName );
};

/*
 * �X�g���[������A�Z���u�����쐬����
 */
class VMAssembleInput : public IAssembleReader {
private :
	VMAssembleCollection* m_asm;
public  :
	VMAssembleInput( CStream stream );
	virtual AsmInfo* getAssembly( int index ){
		if( this->m_asm ) return this->m_asm->indexAt( index );
		return NULL;
	}
	virtual AsmInfo* getAssembly( std::string name ){
		if( this->m_asm ) return this->m_asm->indexAt( this->m_asm->find( name ) );
		return NULL;
	}
};

} // namespace Assembly
} // namespace VM
} // namespace Sencha