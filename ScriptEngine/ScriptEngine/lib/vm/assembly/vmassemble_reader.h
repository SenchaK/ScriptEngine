#pragma once
#include "../vmdefine.h"
#include "vmassembly_info.h"
#include "../parser/vmparser.h"

namespace SenchaVM{
namespace Assembly{
class AssembleReader;
typedef shared_ptr<AssembleReader> CAssembleReader;


/*
 * 解析結果をテキスト形式のファイルに書き込む
 * アセンブル命令が正しく出力されているか確認する用途で使用する。
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
 * 解析結果をテキストでコンソールに出力する。
 */
class VMTextConsoleWriter {
public  :
	VMTextConsoleWriter( IAssembleReader* reader );
};

/*
 * 解析結果をバイナリ形式のファイルに書き込む。
 * 内部コードを別ファイルにする用途で使用する。
 */
class VMBinaryFileWriter {
public  :
	VMBinaryFileWriter( IAssembleReader* reader );
};


} // namespace Assembly
} // namespace SenchaVM