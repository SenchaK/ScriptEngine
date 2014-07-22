#pragma once
#include <vector>
#include <cassert>
#include <string>

namespace Sencha{
namespace Util{
namespace Bin {
	std::vector<unsigned char> int32ToByte( signed long int value );
	std::vector<unsigned char> int16ToByte( signed short int value );
	std::vector<unsigned char> uint32ToByte( unsigned long int value );
	std::vector<unsigned char> uint16ToByte( unsigned short int value );
	std::vector<unsigned char> singleToByte( float f32value );
	std::vector<unsigned char> doubleToByte( double f64value );
	std::vector<unsigned char> stringToByte( std::string stringvalue );
	signed   long  int byteToInt32( std::vector<unsigned char>& bytes  , size_t startIndex );
	signed   short int byteToInt16( std::vector<unsigned char>& bytes  , size_t startIndex );
	unsigned long  int byteToUInt32( std::vector<unsigned char>& bytes , size_t startIndex );
	unsigned short int byteToUInt16( std::vector<unsigned char>& bytes , size_t startIndex );
	float byteToSingle( std::vector<unsigned char>& bytes , size_t startIndex );
	double byteToDouble( std::vector<unsigned char>& bytes , size_t startIndex );
	std::string byteToString( std::vector<unsigned char>& bytes , size_t startIndex );

} // namespace Bin
} // namespace Util
} // namespace Sencha