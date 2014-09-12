#include "binary.h"

namespace Sencha{
namespace Util{
namespace Bin {


std::vector<unsigned char> int32ToByte( signed long int value ){
	std::vector<unsigned char> result;
	result.push_back( 0xFF & value );
	result.push_back( 0xFF & ( value >>  8 ) );
	result.push_back( 0xFF & ( value >> 16 ) );
	result.push_back( 0xFF & ( value >> 24 ) );
	return result;
}
std::vector<unsigned char> int16ToByte( signed short int value ){
	std::vector<unsigned char> result;
	result.push_back( 0xFF & value );
	result.push_back( 0xFF & ( value >>  8 ) );
	return result;
}
std::vector<unsigned char> uint32ToByte( unsigned long int value ){
	return int32ToByte( value );
}
std::vector<unsigned char> uint16ToByte( unsigned short int value ){
	return int16ToByte( value );
}


std::vector<unsigned char> singleToByte( float f32value ){
	union {
		unsigned __int32 u32bit;
		float f32value;
	}value;
	value.f32value = f32value;

	std::vector<unsigned char> result;
	result.push_back( 0xFF & value.u32bit );
	result.push_back( 0xFF & ( value.u32bit >>  8 ) );
	result.push_back( 0xFF & ( value.u32bit >> 16 ) );
	result.push_back( 0xFF & ( value.u32bit >> 24 ) );
	return result;
}
std::vector<unsigned char> doubleToByte( double f64value ){
	union {
		unsigned __int64 u64bit;
		double f64value;
	}value;
	value.f64value = f64value;

	std::vector<unsigned char> result;

#define __64( x ) ((unsigned __int64)x)
	result.push_back( 0xFF &   __64( value.u64bit ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >>  8 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 16 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 24 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 32 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 40 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 48 ) );
	result.push_back( 0xFF & ( __64( value.u64bit ) >> 56 ) );
#undef __64
	return result;
}

std::vector<unsigned char> stringToByte( std::string stringvalue ){
	std::vector<unsigned char> result;
	for( size_t index = 0 ; index < stringvalue.length() ; index++ ){
		result.push_back( stringvalue[index] );
	}
	result.push_back( 0 );
	return result;
}





signed long int byteToInt32( std::vector<unsigned char>& bytes  , size_t startIndex ){
	assert( startIndex >= 0 );
	assert( startIndex + 3 < bytes.size() );
	unsigned char byte_0 = bytes[startIndex+0];
	unsigned char byte_1 = bytes[startIndex+1];
	unsigned char byte_2 = bytes[startIndex+2];
	unsigned char byte_3 = bytes[startIndex+3];
	return ( byte_3 << 24 ) | ( byte_2 << 16 ) | ( byte_1 << 8 ) | byte_0;
}
signed short int byteToInt16( std::vector<unsigned char>& bytes  , size_t startIndex ){
	assert( startIndex >= 0 );
	assert( startIndex + 3 < bytes.size() );
	unsigned char byte_0 = bytes[startIndex+0];
	unsigned char byte_1 = bytes[startIndex+1];
	return ( byte_1 << 8 ) | byte_0;
}
unsigned long  int byteToUInt32( std::vector<unsigned char>& bytes , size_t startIndex ){
	return byteToInt32( bytes , startIndex );
}
unsigned short int byteToUInt16( std::vector<unsigned char>& bytes , size_t startIndex ){
	return byteToInt16( bytes , startIndex );
}

float byteToSingle( std::vector<unsigned char>& bytes , size_t startIndex ){
	assert( startIndex >= 0 );
	assert( startIndex + 3 < bytes.size() );
	unsigned char byte_0 = bytes[startIndex+0];
	unsigned char byte_1 = bytes[startIndex+1];
	unsigned char byte_2 = bytes[startIndex+2];
	unsigned char byte_3 = bytes[startIndex+3];
	union {
		unsigned __int32 u32bit;
		float f32value;
	}value;
	value.u32bit = ( byte_3 << 24 ) | ( byte_2 << 16 ) | ( byte_1 << 8 ) | byte_0;
	return value.f32value;
}
double byteToDouble( std::vector<unsigned char>& bytes , size_t startIndex ){
	assert( startIndex >= 0 );
	assert( startIndex + 7 < bytes.size() );
	unsigned char byte_0 = bytes[startIndex+0];
	unsigned char byte_1 = bytes[startIndex+1];
	unsigned char byte_2 = bytes[startIndex+2];
	unsigned char byte_3 = bytes[startIndex+3];
	unsigned char byte_4 = bytes[startIndex+4];
	unsigned char byte_5 = bytes[startIndex+5];
	unsigned char byte_6 = bytes[startIndex+6];
	unsigned char byte_7 = bytes[startIndex+7];

	union {
		unsigned __int64 u64bit;
		double f64value;
	}value;
#define __64( x ) ((unsigned __int64)x)
	value.u64bit = 
		( __64( byte_7 ) << 56 ) | 
		( __64( byte_6 ) << 48 ) | 
		( __64( byte_5 ) << 40 ) |
		( __64( byte_4 ) << 32 ) | 
		( __64( byte_3 ) << 24 ) | 
		( __64( byte_2 ) << 16 ) | 
		( __64( byte_1 ) <<  8 ) | 
		  __64( byte_0 );
#undef __64
	return value.f64value;
}

std::string byteToString( std::vector<unsigned char>& bytes , size_t startIndex ){
	std::string result = "";
	for( size_t i = startIndex ; i < bytes.size() ; i++ ){
		if( bytes[i] == 0 ){
			break;
		}
		result += (char)bytes[i];
	}
	return result;
}


} // namespace Bin
} // namespace Util
} // namespace Sencha
