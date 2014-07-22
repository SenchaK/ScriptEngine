#pragma once
#include <string>

namespace SenchaVM{
namespace Assembly{
using namespace std;

#define REG_INDEX_FUNC ( 100 )


//	**************************************************
//	�j�[���j�b�N�\
//	**************************************************
class EMnemonic {
public :
	typedef enum {
		/*
		 * �l�����Z
		 */
		PMov,
		Mov ,
		Add , 
		Sub ,
		Mul , 
		Div ,
		Rem ,

		/*
		 * �֐��Ăяo��
		 */
		Push ,
		Pop ,
		SetStackPointer ,
		Call , 
		EndFunc ,

		/*
		 * ��r����
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
		 * �W�����v����
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
