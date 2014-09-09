#pragma once
#include "../vmdefine.h"
#include "vmassembly_info.h"
#include "../parser/vmparser.h"

namespace SenchaVM{
namespace Assembly{
class AssembleReader;
typedef shared_ptr<AssembleReader> CAssembleReader;


/*
 * ��͌��ʂ��e�L�X�g�`���̃t�@�C���ɏ�������
 * �A�Z���u�����߂��������o�͂���Ă��邩�m�F����p�r�Ŏg�p����B
 */
class VMTextFileWriter {
private :
	int m_pc;
	string m_fileName;
	FILE* m_fp;
	IAssembleReader* m_reader;
public  :
	VMTextFileWriter( IAssembleReader* reader , string fileName ){
		assert( reader );
		this->m_reader = reader;
		this->m_fileName = fileName;
		this->m_fp = NULL;
		this->m_pc = 0;
		fopen_s( &this->m_fp , this->m_fileName.c_str() , "w" );
		assert( this->m_fp );
		execute();
	}
private :
	void execute();
	void execAssemble( AsmInfo* assembly );
	void writeVarChain( AsmInfo* assembly );
};

/*
 * ��͌��ʂ��e�L�X�g�ŃR���\�[���ɏo�͂���B
 */
class VMTextConsoleWriter {
public  :
	VMTextConsoleWriter( IAssembleReader* reader );
};

/*
 * ��͌��ʂ��o�C�i���`���̃t�@�C���ɏ������ށB
 * �����R�[�h��ʃt�@�C���ɂ���p�r�Ŏg�p����B
 */
class VMBinaryFileWriter {
public  :
	VMBinaryFileWriter( IAssembleReader* reader );
};


} // namespace Assembly
} // namespace SenchaVM