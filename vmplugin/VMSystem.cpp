#include "VMSystem.h"

VMSystem::VMSystem() : SenchaVM::Assembly::VMSystemCallService() {
}

/* virtual */
void VMSystem::callFunction( string funcName ){
	if( funcName == "Log" ){
		Log( popMemory().ValueString() );
	}
	else if( funcName == "str_len" ){
		str_len( popMemory().ValueString() );
	}
	else if( funcName == "str_replace" ){
		const string& to   = popMemory().ValueString();
		const string& from = popMemory().ValueString();
		const string& text = popMemory().ValueString();
		str_replace( text , from , to );
	}
	else if( funcName == "number_format" ){
		const int BUF_SIZE = 512;
		const int& number = (const int)popMemory().Value();
		const string& format = popMemory().ValueString();
		char buffer[BUF_SIZE];
		sprintf_s<BUF_SIZE>( buffer , format.c_str() , number );
		string result = buffer;
		Return( result );
	}
	else if( funcName == "str_getc" ){
		const unsigned int& index = (unsigned int)popMemory().Value();
		const string& text = popMemory().ValueString();
		str_getc( text , index );
	}
	else if( funcName == "sleep" ){
		const int& sleeptime = (unsigned int)popMemory().Value();
		sleep( sleeptime );
	}
}


void VMSystem::Log( const string& message ){
	printf( "%s\n" , message.c_str() );
}
void VMSystem::sleep( const int sleeptime ){
	VMSleep( sleeptime );
}

void VMSystem::str_len( const string& string_value ){
	Return( string_value.length() );
}
void VMSystem::str_replace( const string& string_value , string from , string to ){
	string ret = string_value;

	std::string::size_type pos = 0;
	pos = ret.find( from , pos );
	while( pos != std::string::npos ){
		ret.replace( pos , from.length() , to );
		pos = ret.find( from , pos );
	}
	Return( ret );
}
void VMSystem::str_getc( const string& string_value , const int& index ){
	string ret;
	ret += string_value[index];
	Return( ret );
}