#pragma once
#include "vm_define.h"
#include "vm_memory.h"
#include "assembly\vm_assembly_info.h"
#include "assembly\vm_mnemonic_define.h"
#include <stack>



namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;

/* **************************************************
 * アセンブルコマンドを実行する
 * ************************************************** */
class VMDriver;
class VMR;
typedef shared_ptr<VMDriver> CVMDriver;

class VMDriver {
private :
	enum {
		CALL_STACK_SIZE = 1024 , 
	};

	VMCallStack* m_callStack;
	int m_callStackIndex;
	int m_pc;
	int m_push;
	int m_funcAddr;
	size_t m_localAddr;
	size_t m_stacksize;
	size_t m_staticsize;
	Memory* m_local;
	Memory* m_static;
	VMR* R;
	IAssembleReader* m_reader;
	VMBuiltIn* m_built_in;
private :
	void callSubroutine( int addres );
	void startCoroutine( int addres );
	void updateCoroutine();
	void _mov_ptr();
	void _mov();
	void _add();
	void _sub();
	void _mul();
	void _div();
	void _rem();
	void _inc();
	void _dec();
	void _cmp( int cmpType );
	void _log( int logType );
	void _not();
	void _minus();
	void _jumpzero();
	void _jumpnotzero();
	void _jmp();
	void _push();
	void _push_ptr();
	void _pop();
	void _call();
	void _st();
	void _ld();
	void _endFunc();
	void _ret();
private :
	bool isActive();
	unsigned char getByte( int funcAddr , int pc );
	AsmInfo* currentAssembly();
	Memory& createOrGetMemory();
	Memory& getRefMemory( int location , int address , size_t i , size_t size );
	Memory& getLocal( int addres );
	Memory& getStatic( int addres );
	Memory& getMemory( int location , int address );
	void setLocal( int addres , Memory& m );
	void setStatic( int addres , Memory& m );
	void setMemory( Memory& src , Memory& value );
	void setMemory( Memory& src , int addr , int location );
private :
	void getMemoryInfo( Memory* p , int* addr , int* location );
	AsmInfo* findFunction( string name );
	int getFunctionAddr( string name );	
	void getFunction( string func );
	void vmsetup();
	void execute();
	void initialize( IAssembleReader* reader , VMBuiltIn* built_in , size_t stacksize , size_t staticsize );
public :
	VMDriver( IAssembleReader* reader , VMBuiltIn* built_in );
	virtual ~VMDriver();
	void executeFunction( string funcName );
	void Return( Memory& m );
	Memory& popMemory();
public :
};



} // namespace Assembly
} // namespace VM
} // namespace Sencha
