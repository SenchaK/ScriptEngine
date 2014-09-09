#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <cassert>

namespace Sencha {
/*
 * class ConsoleLog
 */
void ConsoleLog::print( const char* formatString , va_list args ){
	vprintf( formatString , args );
}


/*
 * class TextFileLog
 */
TextFileLog::TextFileLog( const char* fileName ){
	fopen_s( &this->m_fp , fileName , "w" );
	assert( this->m_fp );
}

TextFileLog::~TextFileLog(){
	fclose( this->m_fp );
}

void TextFileLog::print( const char* formatString , va_list args ){
	vfprintf( this->m_fp , formatString , args );
}


}
