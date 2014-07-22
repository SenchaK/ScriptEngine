#include "vmassembly_info.h"

namespace SenchaVM{
namespace Assembly{

// ***********************************************************************
// class AssemblyInfo
// ***********************************************************************
bool AssemblyInfo::hasMore( int pc ){
	if( pc >= (int)m_assembleCommandList.size() ) return false;
	return true;
}
byte AssemblyInfo::getCommand( const int& pc ){
	if( pc >= 0 && pc < (int)m_assembleCommandList.size() ){
		return m_assembleCommandList[pc];
	}
	return 0;
}
unsigned __int8 AssemblyInfo::moveU8( int& pc ){
	unsigned __int8 result = getCommand( pc );
	pc++;
	return result;
}
unsigned __int16 AssemblyInfo::moveU16( int& pc ){
	unsigned __int16 result = Bin::byteToUInt16( m_assembleCommandList , (size_t)pc );
	pc += 2;
	return result;
}
unsigned __int32 AssemblyInfo::moveU32( int& pc ){
	unsigned __int32 result = Bin::byteToUInt32( m_assembleCommandList , (size_t)pc );
	pc += 4;
	return result;
}
double AssemblyInfo::moveDouble( int& pc ){
	double result = Bin::byteToDouble( m_assembleCommandList , (size_t)pc );
	pc += 8;
	return result;
}
string AssemblyInfo::moveString( int& pc ){
	string result = Bin::byteToString( m_assembleCommandList , (size_t)pc );
	pc += (int)result.length() + 1;
	return result;
}


} // namespace Assembly
} // namespace SenchaVM