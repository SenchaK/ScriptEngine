#pragma once
#include "vm_define.h"
#include "vm_memory.h"

namespace Sencha {
namespace VM {
namespace Assembly {

/*
 * ドライバで使用するレジスタ・ストア領域
 * 汎用レジスタを用意
 * 退避領域もここに定義する。
 */
class VMR {
private :
	typedef enum {
		R_MAX      =   32 , 
		STORE_SIZE = 1028 , 
	};
	Memory* R;
	Memory* STORE;
	int store_p;
// **************************************************************
// 公開API
// **************************************************************
public :
	VMR();
	~VMR();
	/*
	 * 現在のレジスタの中身を指定カウント分だけストア領域に積み上げる。
	 * countが3の場合
	 * STORE[0] ... R[0]
	 * STORE[1] ... R[1]
	 * STORE[2] ... R[2]
	 * というように積まれる。
	 *
	 * @param count ... 保存する領域数
	 */
	void store( int count );

	/*
	 * ストア領域から指定カウント分だけの情報をレジスタに渡して状態修復を行う
	 * R[0] ... STORE[0]
	 * R[1] ... STORE[1]
	 * R[2] ... STORE[2]
	 * という順番で戻るようにする。
	 * @param count ... どこまで戻すかの取り出し数
	 */
	void load( int count );
	Memory& getMemory( int addres );
	void setMemory( int addres , Memory value );
	void getMemoryInfo( Memory* p , int* addr , int* location );
};

} // namespace Assembly
} // namespace VM
} // namespace Sencha