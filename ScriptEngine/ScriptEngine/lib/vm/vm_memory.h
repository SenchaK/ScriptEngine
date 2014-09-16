#pragma once
#include "vm_define.h"

namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;


class Memory {
public :
	double value;
	signed int address;
	signed int location;
	string value_string;

	Memory( double v , string s );
	Memory();
	virtual void setMemory( const Memory& m );
	virtual void setMemory( int address , int location );

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


/*
 * ���W�X�^�p������
 * setMemory�g�p���ɒʏ�̃������̓|�C���^�^�ł���ꍇ�͎Q�Ɛ�A�h���X�ɒl���������ނ��A
 * ���W�X�^�̏ꍇ�̓f�[�^�y�уA�h���X�R�s�[�����s��Ȃ��悤�ɂ���B
 */
class RMemory : public Memory {
};

} // namespace Assembly
} // namespace VM
} // namespace Sencha