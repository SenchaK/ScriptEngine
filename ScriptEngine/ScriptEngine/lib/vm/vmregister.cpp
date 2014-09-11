#include "vmregister.h"
#include "../vm/assembly/vm_mnemonic_define.h"

#define MAX_R ( 32 )


namespace SenchaVM{
namespace Assembly{

R_STACK::CalcMemory::CalcMemory(){
	m_Mem = CMemory( new Memory[MAX_R] , std::default_delete<Memory[]>() );
}


// ***********************************************************************
// class R
/* static */
shared_ptr<R_STACK> R_STACK::instance;
/* static */
shared_ptr<R_STACK> R_STACK::Instance(){
	if( instance.get() == NULL ){
		instance = shared_ptr<R_STACK>( new R_STACK() );
	}
	return instance;
}

R_STACK::R_STACK(){
}

/* static */
Memory& R_STACK::getMemory( int addres ){
	if( addres < 0 )     { printf( "Register getMemory under flow!! %d\n" , addres ); }
	if( addres >= MAX_R ){ printf( "Register getMemory over  flow!! %d\n" , addres ); }
	assert( addres >= 0 && addres < MAX_R );
	return Instance()->m_Calc.m_Mem.get()[addres];
}

/* static */ 
void R_STACK::setMemory( int addres , Memory value ){
	assert( addres >= 0 && addres < MAX_R );
	Instance()->m_Calc.m_Mem.get()[addres].setMemory( value );
}

// ***********************************************************************




} // Assembly
} // SenchaVM