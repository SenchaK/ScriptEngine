#pragma once
#include <string>

namespace Sencha{
namespace VM {
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
		MovPtr ,
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
		PushPtr ,
		Pop ,
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
		Not    ,
		Minus  ,
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
		RET        ,
		REG        ,

		LIT_VALUE  ,
		LIT_STRING ,
		MEM_L      ,
		MEM_S      ,
		MEM_THIS_P ,
		ST , 
		LD ,
	};
public :
static string toString( int mnemonic );
};



} // namespace Assembly
} // namespace VM
} // namespace Sencha
