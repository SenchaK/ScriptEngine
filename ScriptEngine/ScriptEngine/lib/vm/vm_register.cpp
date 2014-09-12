#include "vm_register.h"
#include "../vm/assembly/vm_mnemonic_define.h"

namespace Sencha{
namespace VM {
namespace Assembly{
VMR::VMR(){
	this->R = new Memory[R_MAX];
	this->STORE = new Memory[STORE_SIZE];
	this->store_p = 0;
}

VMR::~VMR(){
	delete[] this->R;
	delete[] this->STORE;
	this->store_p = 0;
}

void VMR::store( int count ){
	for( int i = 0 ; i < count ; i++ ){
		this->STORE[this->store_p++] = this->R[i];
	}
}

void VMR::load( int count ){
	int store_peek_p = this->store_p;
	for( int i = 0 ; i < count ; i++ ){
		int store_index = ( store_peek_p - count ) + i;
		this->R[i] = this->STORE[store_index];
		this->store_p--;
	}
}

Memory& VMR::getMemory( int addres ){
	if( addres < 0 )     { printf( "Register getMemory under flow!! %d\n" , addres ); }
	if( addres >= R_MAX ){ printf( "Register getMemory over  flow!! %d\n" , addres ); }
	assert( addres >= 0 && addres < R_MAX );
	return this->R[addres];
}

void VMR::setMemory( int addres , Memory value ){
	assert( addres >= 0 && addres < R_MAX );
	this->R[addres].setMemory( value );
}

// ***********************************************************************




} // namespace Assembly
} // namespace VM
} // namespace Sencha