#include "vm_assembly_info.h"
#include "../parser/vm_scope.h"

namespace Sencha{
namespace VM {
namespace Assembly{

// ***********************************************************************
// class AsmInfo
// ***********************************************************************
bool AsmInfo::hasMore( int pc ){
	if( pc >= (int)m_code.size() ) return false;
	return true;
}
byte AsmInfo::getCommand( const int& pc ){
	if( pc >= 0 && pc < (int)m_code.size() ){
		return m_code[pc];
	}
	return 0;
}
unsigned __int8 AsmInfo::moveU8( int& pc ){
	unsigned __int8 result = getCommand( pc );
	pc++;
	return result;
}
unsigned __int16 AsmInfo::moveU16( int& pc ){
	unsigned __int16 result = Bin::byteToUInt16( m_code , (size_t)pc );
	pc += 2;
	return result;
}
unsigned __int32 AsmInfo::moveU32( int& pc ){
	unsigned __int32 result = Bin::byteToUInt32( m_code , (size_t)pc );
	pc += 4;
	return result;
}
double AsmInfo::moveDouble( int& pc ){
	double result = Bin::byteToDouble( m_code , (size_t)pc );
	pc += 8;
	return result;
}
string AsmInfo::moveString( int& pc ){
	string result = Bin::byteToString( m_code , (size_t)pc );
	pc += (int)result.length() + 1;
	return result;
}


// class VMBuiltIn
void VMBuiltIn::entryFunction( VMBuiltInFunction* func ){
	this->built_in_function.push_back( func );
}

void VMBuiltIn::clear(){
	for( size_t i = 0 ; i < this->built_in_function.size() ; i++ ){
		delete this->built_in_function[i];
	}
	this->built_in_function.clear();
	for( size_t i = 0 ; i < this->packages.size() ; i++ ){
		delete this->packages[i];
	}
	this->packages.clear();
}

VMBuiltInFunction* VMBuiltIn::indexAt( size_t index ){
	if( index >= this->built_in_function.size() ) return NULL;
	return this->built_in_function[index];
}

Package* VMBuiltIn::insertPackage( string packageName ){
	Package* package = new Package( packageName , SCOPE_LEVEL_GLOBAL );
	this->packages.push_back( package );
	return package;
}

int VMBuiltIn::find( string& funcName ){
	for( size_t i = 0 ; i < this->built_in_function.size() ; i++ ){
		if( this->built_in_function[i]->equal( funcName ) ){
			return i;
		}
	}
	return -1;
}

// virtual
VMBuiltIn::~VMBuiltIn(){
	this->clear();
}

} // namespace Assembly
} // namespace VM
} // namespace Sencha