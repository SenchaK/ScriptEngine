#pragma once
#include "../../util/util_stream.h"
#include "../lexer/vmlexer.h"

namespace SenchaVM{
namespace Assembly{
typedef Sencha::Util::byte vmbyte;


class AssemblyInfo {
private :
	string m_name;
	size_t m_stackFrame;
	size_t m_addr;
	size_t m_args;
	vector<vmbyte> m_code;
public :
	void clearBytes(){
		m_code.clear();
	}
	void pushByte( vmbyte byteData ){
		m_code.push_back( byteData );
	}
	void setBytes( CStream stream ){
		while( stream->hasNext() ){
			m_code.push_back( stream->getByte() );
		}
	}
	void setArgs( size_t args ){
		m_args = args;
	}
	void setName( const string& name ){
		m_name = name;
	}
	string name(){
		return m_name;
	}
	void setStackFrame( size_t stackFrame ){
		m_stackFrame = stackFrame;
	}
	size_t Args(){
		return m_args;
	}
	size_t stackFrame(){
		return m_stackFrame;
	}
	size_t addr(){
		return m_addr;
	}

	bool hasMore( int pc );
	vmbyte getCommand( const int& pc );
	unsigned __int8  moveU8    ( int& pc );
	unsigned __int16 moveU16   ( int& pc );
	unsigned __int32 moveU32   ( int& pc );
	double           moveDouble( int& pc );
	string           moveString( int& pc );
};


struct VMCallStack {
	int funcAddr;
	int prog;	
};

struct VMAssembleCollection {
	vector<AssemblyInfo> assemblyInfo;
	void clear(){
		assemblyInfo.clear();
	}
};


} // namespace Assembly
} // namespace SenchaVM