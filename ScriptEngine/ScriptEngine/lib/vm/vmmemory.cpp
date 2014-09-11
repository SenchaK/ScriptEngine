#include "vmmemory.h"

namespace SenchaVM{
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


} // Assembly
} // SenchaVM