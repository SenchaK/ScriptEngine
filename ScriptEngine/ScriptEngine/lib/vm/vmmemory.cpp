#include "vmmemory.h"

namespace SenchaVM{
namespace Assembly{

Memory::Memory( double v , string s ){
	this->value = v;
	this->value_string = s;
	this->isBindObjectData = false;
}
Memory::Memory(){
	this->value = 0;
	this->value_string = "";
	this->isBindObjectData = false;
}
void Memory::setMemory( const Memory& m ){
	this->value = m.value;
	this->value_string = m.value_string;
	this->isBindObjectData = false;
}
void Memory::setPointer( int pointer_addres , int memory_scope ){
	this->value = 0;
	this->value_string = "";
	this->pointer_addres = pointer_addres;
	this->memory_scope = memory_scope;
	this->isBindObjectData = false;
}


} // Assembly
} // SenchaVM