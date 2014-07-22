#pragma once
#include "vmdefine.h"
#include "vmmemory.h"


#define MAX_CALC_STACK ( 1024 )


namespace SenchaVM{
namespace Assembly{
class R_STACK {
private :
	// �v�Z�@������
	class CalcMemory {
	public :
		CMemory m_Mem; 
		CalcMemory();
	};
private :
	CalcMemory m_Calc[MAX_CALC_STACK] ; // �v�Z�@�������X�^�b�N
	CMemory m_Ret            ; // �߂�l������
	int m_Index              ; // �C���f�b�N�X������
	int m_CalcIndex          ; // �v�Z�@�ԍ�
	R_STACK();
private :
	static shared_ptr<R_STACK> instance;
	static shared_ptr<R_STACK> Instance();

// **************************************************************
// ���JAPI
// **************************************************************
public :
	static int getIndex();
	static void setIndex( int value );
	static Memory& getMemory( int addres );
	static Memory& getReturnMemory();
	static void setReturnMemory( const Memory& value );
	static void setMemory( int addres , Memory value );
	static void pushCalc();
	static void popCalc();
};

} // Assembly
} // SenchaVM