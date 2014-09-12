#pragma once
#include "vm_define.h"

namespace SenchaVM{
namespace Assembly{
using namespace std;


class Memory {
public :
	Memory( double v , string s );
	Memory();
	void setMemory( const Memory& m );
	double value;
	string value_string;

	Memory& operator+( Memory& src ){
		this->value += src.value;
		this->value_string += src.value_string;
		return *this;
	}
	Memory& operator-( Memory& src ){
		this->value -= src.value;
		return *this;
	}
	Memory& operator*( Memory& src ){
		this->value *= src.value;
		return *this;
	}
	Memory& operator/( Memory& src ){
		this->value /= src.value;
		return *this;
	}
	Memory& operator%( Memory& src ){
		int v = (int)this->value;
		v %= (int)src.value;
		this->value = v;
		return *this;
	}
	Memory& operator++(){
		this->value += 1;
		return *this;
	}
	Memory& operator--(){
		this->value -= 1;
		return *this;
	}
	Memory operator++(int){
		Memory result = *this;
		this->value += 1;
		return result;
	}
	Memory operator--(int){
		Memory result = *this;
		this->value -= 1;
		return result;
	}


	bool operator>=( Memory& compareTo ){
		return this->value >= compareTo.value;
	}
	bool operator>( Memory& compareTo ){
		return this->value > compareTo.value;
	}
	bool operator<=( Memory& compareTo ){
		return this->value <= compareTo.value;
	}
	bool operator<( Memory& compareTo ){
		return this->value < compareTo.value;
	}
	bool operator==( Memory& compareTo ){
		return ((long long)this->value) == ((long long)compareTo.value);
	}
	bool operator!=( Memory& compareTo ){
		return ((long long)this->value) != ((long long)compareTo.value);
	}
	bool operator==( int compareTo ){
		return ((long long)this->value) == ((long long)compareTo);
	}
	bool operator!=( int compareTo ){
		return ((long long)this->value) != ((long long)compareTo);
	}
};
typedef shared_ptr<Memory> CMemory;

} // Assembly
} // SenchaVM