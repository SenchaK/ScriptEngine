#pragma once
#include<string>

namespace Sencha{
namespace Util{
using namespace std;

/*
 * 例外基底
 * 例外処理を行いたいときに使う。
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
 * 仮想関数未実装時に例外を飛ばしたい場合使用する。
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
 * インスタンスがnullのときに投げる例外
 */
class NullPointerException : public Exception {
public :
};

} // namespace Util
} // namespace Sencha