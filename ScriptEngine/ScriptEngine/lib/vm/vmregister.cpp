#include "vmregister.h"
#include "../vm/assembly/vm_mnemonic_define.h"

#define REG_MASTER_DEBUG (0)
#define REG_UNIT_DEBUG (1)
#define REG_DEBUG (REG_MASTER_DEBUG)

#ifdef REG_DEBUG
	#if REG_DEBUG == REG_MASTER_DEBUG
		#define REG_LOG VM_PRINT
		#define REG_ASSERT VM_ASSERT
	#elif REG_DEBUG == REG_UNIT_DEBUG
		#define REG_LOG printf
		#define REG_ASSERT assert
	#endif
#else
	#define REG_LOG(...)
	#define REG_ASSERT(...)
#endif

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
	m_Ret       = CMemory( new Memory() );
	m_Index     = 0;
	m_CalcIndex = 0;
}

/* static */
Memory& R_STACK::getReturnMemory(){
	return *Instance()->m_Ret;
}

/* static */
void R_STACK::setReturnMemory( const Memory& value ){
	Instance()->m_Ret->setMemory( value );
}

/* static */
int R_STACK::getIndex(){
	return Instance()->m_Index;
}

/* static */
void R_STACK::setIndex( int value ){
	Instance()->m_Index = value;
}

/* static */
Memory& R_STACK::getMemory( int addres ){
	switch( addres ){
		case REG_INDEX_FUNC : return *Instance()->m_Ret.get();
	}
	if( addres < 0 )     { REG_LOG( "Register getMemory under flow!! %d\n" , addres ); }
	if( addres >= MAX_R ){ REG_LOG( "Register getMemory over  flow!! %d\n" , addres ); }
	REG_ASSERT( addres >= 0 && addres < MAX_R );
	return Instance()->m_Calc[Instance()->m_CalcIndex].m_Mem.get()[addres];
}

/* static */ 
void R_STACK::setMemory( int addres , Memory value ){
	REG_ASSERT( addres >= 0 && addres < MAX_R );
	Instance()->m_Calc[Instance()->m_CalcIndex].m_Mem.get()[addres].setMemory( value );
//	VM_PRINT( "R::setMemory!! addres[%d] , value = %0.2f , \"%s\" \n" , 
//		addres , 
//		Instance()->m_Mem.get()[addres].Value() , 
//		Instance()->m_Mem.get()[addres].ValueString().c_str() );
}

/* static */
void R_STACK::pushCalc(){
	Instance()->m_CalcIndex++;
	REG_ASSERT( Instance()->m_CalcIndex >= 0 );
	REG_ASSERT( Instance()->m_CalcIndex < MAX_R );
}

/* static */
void R_STACK::popCalc(){
	Instance()->m_CalcIndex--;
}

// ***********************************************************************




} // Assembly
} // SenchaVM