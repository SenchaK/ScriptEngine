#pragma once
#include "../../util/logger.h"
#include "../vmdefine.h"
#include "../parser/vmparser.h"
#include "vmassembly_info.h"


namespace SenchaVM{
namespace Assembly{
class AssembleReader;
typedef shared_ptr<AssembleReader> CAssembleReader;


/*
 * ��͌��ʂ��e�L�X�g�`���̃t�@�C���ɏ�������
 * �A�Z���u�����߂��������o�͂���Ă��邩�m�F����p�r�Ŏg�p����B
 */
class VMAssembleLog {
private :
	int m_pc;
	IAssembleReader* m_reader;
	CLog m_log;
public  :
	VMAssembleLog( IAssembleReader* reader , CLog log ){
		assert( reader );
		assert( log.get() );
		this->m_log = log;
		this->m_reader = reader;
		this->m_pc = 0;
		execute();
	}
	void print( const char* formatString , ... ){
		va_list args;
		va_start( args , formatString );
		this->m_log->print( formatString , args );
		va_end( args );
	}
private :
	void execute();
	void execAssemble( AsmInfo* assembly );
	void writeVarChain( AsmInfo* assembly );
	void writeCalc( const char* name , AsmInfo* assembly );
	void writeJmp( const char* name , AsmInfo* assembly );
protected :
	void call( AsmInfo* assembly );
	void st( AsmInfo* assembly );
	void ld( AsmInfo* assembly );
	void push( AsmInfo* assembly );
	void end( AsmInfo* assembly );
	void ret( AsmInfo* assembly );
};



} // namespace Assembly
} // namespace SenchaVM