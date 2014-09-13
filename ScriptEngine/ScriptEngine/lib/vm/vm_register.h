#pragma once
#include "vm_define.h"
#include "vm_memory.h"

namespace Sencha {
namespace VM {
namespace Assembly {

/*
 * �h���C�o�Ŏg�p���郌�W�X�^�E�X�g�A�̈�
 * �ėp���W�X�^��p��
 * �ޔ�̈�������ɒ�`����B
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
// ���JAPI
// **************************************************************
public :
	VMR();
	~VMR();
	/*
	 * ���݂̃��W�X�^�̒��g���w��J�E���g�������X�g�A�̈�ɐςݏグ��B
	 * count��3�̏ꍇ
	 * STORE[0] ... R[0]
	 * STORE[1] ... R[1]
	 * STORE[2] ... R[2]
	 * �Ƃ����悤�ɐς܂��B
	 *
	 * @param count ... �ۑ�����̈搔
	 */
	void store( int count );

	/*
	 * �X�g�A�̈悩��w��J�E���g�������̏������W�X�^�ɓn���ď�ԏC�����s��
	 * R[0] ... STORE[0]
	 * R[1] ... STORE[1]
	 * R[2] ... STORE[2]
	 * �Ƃ������ԂŖ߂�悤�ɂ���B
	 * @param count ... �ǂ��܂Ŗ߂����̎��o����
	 */
	void load( int count );
	Memory& getMemory( int addres );
	void setMemory( int addres , Memory value );
	void getMemoryInfo( Memory* p , int* addr , int* location );
};

} // namespace Assembly
} // namespace VM
} // namespace Sencha