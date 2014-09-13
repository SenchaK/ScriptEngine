#include "vm_memory.h"

namespace Sencha{
namespace VM {
namespace Assembly{

Memory::Memory( double v , string s ){
	this->value = v;
	this->value_string = s;
}
Memory::Memory(){
	this->value = 0;
	this->value_string = "";
}
void Memory::setMemory( const Memory& m ){
	this->value = m.value;
	this->value_string = m.value_string;
}
void Memory::setMemory( int address , int location ){
	this->value = 0;
	this->value_string = "";
	this->address = address;
	this->location = location;
}


} // namespace Assembly
} // namespace VM
} // namespace Sencha