#pragma once

#pragma warning( disable : 4996 )
#include<string>
#include<vector>
#include<cassert>
#include<list>
#include<iostream>
#include<sstream>
#include<memory>


#define TO_STRING( token ) #token

#define VM_DEBUG
#ifdef VM_DEBUG
	//#define VM_PRINT printf
	#define VM_PRINT(...)
	#define VM_ASSERT assert
#else
	#define VM_PRINT(...)
	#define VM_ASSERT(...)
#endif

#define SCOPE_LEVEL_GLOBAL ( 0 )


namespace SenchaVM{
namespace Assembly{

// ƒVƒ“ƒ{ƒ‹Ží—Þ
enum ESymbolType {
	Func           = 0x01 , 
	VariableField  = 0x02 ,
	VariableLocal  = 0x04 , 
	VariableGlobal = 0x08 ,
	Struct         = 0x10 ,
};


} // namespace Assembly
} // namespace SenchaVM