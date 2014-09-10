#pragma once
#include <cstdio>
#include <memory>

namespace Sencha {
class Log{
public :
	virtual void print( const char* formatString , va_list args ){
	}
};

class ConsoleLog : public Log {
public :
	virtual void print( const char* formatString , va_list args );
};

class TextFileLog : public Log {
private :
	FILE* m_fp;
public :
	TextFileLog( const char* fileName );
	~TextFileLog();
	virtual void print( const char* formatString , va_list args );
};
typedef std::shared_ptr<Log> CLog;

}