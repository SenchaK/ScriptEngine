#pragma once
#include "vmdefine.h"
#include "vmmemory.h"

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
	CalcMemory m_Calc;
	R_STACK();
private :
	static shared_ptr<R_STACK> instance;
	static shared_ptr<R_STACK> Instance();

// **************************************************************
// ���JAPI
// **************************************************************
public :
	static Memory& getMemory( int addres );
	static void setMemory( int addres , Memory value );
};

} // Assembly
} // SenchaVM