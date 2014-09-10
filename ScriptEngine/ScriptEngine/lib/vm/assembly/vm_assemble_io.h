#pragma once
#include "../../util/logger.h"
#include "../../util/util_stream.h"
#include "../vmdefine.h"
#include "../parser/vmparser.h"
#include "vmassembly_info.h"


namespace SenchaVM{
namespace Assembly{
/*
 * 解析結果をバイナリ形式のファイルに出力する。
 * 内部コードを別ファイルにする用途で使用する。
 *
 * 各アセンブリは以下のフォーマット形式で保存される
 * [AssemblyName : string           ] アセンブリの名前
 * [StackFrame   : u32              ] スタックフレーム
 * [Address      : u32              ] 関数アドレス
 * [Args         : u32              ] 関数パラメータ数
 * [CodeSize     : u32              ] コード領域サイズ
 * [Code         : byte[ContentSize]] コード領域
 */
class VMAssembleOutput {
private :
	CStream m_stream;
public  :
	VMAssembleOutput( IAssembleReader* reader , const char* fileName );
};

/*
 * ストリームからアセンブリを作成する
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

}
}