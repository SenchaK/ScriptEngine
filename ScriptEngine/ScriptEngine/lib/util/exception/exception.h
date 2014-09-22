#pragma once
#include<string>

namespace Sencha{
namespace Util{
using namespace std;

/*
 * ��O���
 * ��O�������s�������Ƃ��Ɏg���B
 */ 
class Exception {
protected :
	string m_message;
public :
	Exception(){
	}
	Exception( string message ){
		this->m_message = message;
	}
	virtual string getMessage(){
		return this->m_message;
	}
	virtual ~Exception(){
	}
};


/*
 * ���z�֐����������ɗ�O���΂������ꍇ�g�p����B
 */
class NotImplementException : public Exception {
public :
	NotImplementException() : Exception(){
		this->m_message = "NotImplementException";
	}
	NotImplementException( string message ) : Exception( message ){
		this->m_message = message;
	}
};

/*
 * �C���X�^���X��null�̂Ƃ��ɓ������O
 */
class NullPointerException : public Exception {
public :
};

} // namespace Util
} // namespace Sencha