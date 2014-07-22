#pragma once
#include <string>

namespace SenchaVM{
namespace Assembly{
using namespace std;

#define REG_INDEX_FUNC ( 100 )


//	**************************************************
//	ニーモニック表
//	**************************************************
class EMnemonic {
public :
	typedef enum {
		/*
		 * 四則演算
		 */
		PMov,
		Mov ,
		Add , 
		Sub ,
		Mul , 
		Div ,
		Rem ,

		/*
		 * 関数呼び出し
		 */
		Push ,
		Pop ,
		SetStackPointer ,
		Call , 
		EndFunc ,

		/*
		 * 比較命令
		 */
		CmpGeq , 
		CmpG   , 
		CmpLeq , 
		CmpL   , 
		CmpEq  , 
		CmpNEq , 
		LogOr  ,
		LogAnd ,
		Inc    ,
		Dec    ,

		/*
		 * ジャンプ命令
		 */
		Jne  ,
		Jnge , 
		Jng  , 
		Jnle , 
		Jnl  ,
		Je   ,
		Jge  , 
		Jg   , 
		Jle  , 
		Jl   ,
		Jmp  ,
		JumpZero , 
		JumpNotZero ,

		ArrayIndexSet , 
		ArrayIndexAdd ,

		RET        ,
		REG        ,

		LIT_VALUE  ,
		LIT_STRING ,
		MEM_L      ,
		MEM_S      ,
		MEM_THIS_P ,

		AsString   ,
		AsInteger  ,
	};
public :
static string toString( int mnemonic );
};



} // namespace SenchaVM
} // namespace Assembly
