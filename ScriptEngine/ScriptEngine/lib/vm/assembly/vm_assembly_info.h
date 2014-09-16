#pragma once
#include "../../util/stream.h"
#include "../lexer/vm_lexer.h"

namespace Sencha{
namespace VM {
namespace Assembly{
typedef Sencha::Util::byte vmbyte;
class VMDriver;
class Package;

class AsmInfo {
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
	void setAddress( size_t address ){
		m_addr = address;
	}
	void setStackFrame( size_t stackFrame ){
		m_stackFrame = stackFrame;
	}
	string& name(){
		return m_name;
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
	size_t CodeSize(){
		return m_code.size();
	}
	bool equal( string& name ){
		return this->m_name.compare( name ) == 0;
	}

	bool hasMore( int pc );
	vmbyte getCommand( const int& pc );
	unsigned __int8  moveU8( int& pc );
	unsigned __int16 moveU16( int& pc );
	unsigned __int32 moveU32( int& pc );
	double moveDouble( int& pc );
	string moveString( int& pc );
};



/*
 * 組み込み関数
 * パーザに登録する
 */
class VMBuiltInFunction {
private :
	string m_name;                        // マッピング関数名
	void (*function)( VMDriver* driver ); // コールバック関数
public :
	bool equal( string& src ){
		if( this->m_name.compare( src ) == 0 ){
			return true;
		}
		return false;
	}
	VMBuiltInFunction( string name , void (*built_in_function)( VMDriver* ) ){
		this->m_name = name;
		this->function = built_in_function;
	}
	void exec( VMDriver* driver ){
		if( this->function ){
			this->function( driver );
		}
	}
};

/*
 * 関数コールスタック要素
 */
struct VMCallStack {
	int funcAddr; // 実行途中のアセンブリアドレス
	int prog    ; // 実行途中のプログラムカウンタ
	VMCallStack(){
		funcAddr = 0;
		prog = 0;
	}
	void init( int _funcAddr , int _prog ){
		this->funcAddr = _funcAddr;
		this->prog = _prog;
	}
};


/*
 * 組み込み関数マネージャ
 */
class VMBuiltIn {
private :
	vector<VMBuiltInFunction*> built_in_function;
	vector<Package*> packages;
public :
	void entryFunction( VMBuiltInFunction* func );
	Package* insertPackage( string packageName );
	void clear();
	VMBuiltInFunction* indexAt( size_t index );
	int find( string& funcName );
	virtual ~VMBuiltIn();
};

class VMAssembleCollection {
private :
	vector<AsmInfo*> Asm;
public :
	~VMAssembleCollection(){
		this->clear();
	}
	void entryAssembly( AsmInfo* code ){
		assert( code );
		code->setAddress( this->Asm.size() );
		this->Asm.push_back( code );
	}
	size_t count(){
		return this->Asm.size();
	}
	AsmInfo* const indexAt( size_t index ){
		if( index >= this->count() ) return NULL;
		return this->Asm[index];
	}
	int find( string& funcName ){
		for( size_t i = 0 ; i < this->count() ; i++ ){
			if( this->indexAt(i)->equal( funcName ) ){
				return i;
			}
		}
		return -1;
	}
private :
	void clear(){
		for( size_t i = 0 ; i < this->Asm.size() ; i++ ){
			delete this->Asm[i];
		}
		this->Asm.clear();
	}
};


} // namespace Assembly
} // namespace VM
} // namespace Sencha