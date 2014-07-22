#pragma once
#include "vmdefine.h"
#include "vmmemory.h"


#define MAX_CALC_STACK ( 1024 )


namespace SenchaVM{
namespace Assembly{
class R_STACK {
private :
	// 計算機メモリ
	class CalcMemory {
	public :
		CMemory m_Mem; 
		CalcMemory();
	};
private :
	CalcMemory m_Calc[MAX_CALC_STACK] ; // 計算機メモリスタック
	CMemory m_Ret            ; // 戻り値メモリ
	int m_Index              ; // インデックスメモリ
	int m_CalcIndex          ; // 計算機番号
	R_STACK();
private :
	static shared_ptr<R_STACK> instance;
	static shared_ptr<R_STACK> Instance();

// **************************************************************
// 公開API
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