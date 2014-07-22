#pragma once
#include "vmdefine.h"

namespace SenchaVM{
namespace Assembly{
using namespace std;


class Memory {
private :
	union {
		struct{
			signed int pointer_addres : 32;
			signed int memory_scope   : 32;
		};
		struct{
			void* bindObjectDataAddres;
			signed int bindObjectDataType : 32;
		};
		double value;
	};
	string value_string;
	bool isBindObjectData;
public :
	Memory( double v , string s );
	Memory();
	void setPointer( int pointer_addres , int memory_scope );
	void setMemory( const Memory& m );
	const double& Value(){ return value; }
	const string& ValueString(){ return value_string; }
	int PointerAddres(){ return pointer_addres; }
	int MemoryScope(){ return memory_scope; }
};
typedef shared_ptr<Memory> CMemory;

} // Assembly
} // SenchaVM