#include "vmassembly_info.h"

namespace SenchaVM{
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


} // namespace Assembly
} // namespace SenchaVM