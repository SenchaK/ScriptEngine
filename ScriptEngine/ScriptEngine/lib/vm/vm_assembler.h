#pragma once
#include "vm_define.h"
#include "vm_memory.h"
#include "assembly\vm_assembly_info.h"
#include "assembly\vm_mnemonic_define.h"
#include <stack>
#include <list>



namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;

/* **************************************************
 * アセンブルコマンドを実行する
 * ************************************************** */
class VMDriver;
class VMR;
class Subroutine;
class Coroutine;
typedef shared_ptr<VMDriver> CVMDriver;


class VMDriver {
	friend class Coroutine;
	friend class Subroutine;
	friend class Coroutine;
private :
	enum {
		CALL_STACK_SIZE = 1024 ,
	};
	enum STATE {
		STATE_IDLE  , 
		STATE_RUN   ,
		STATE_SLEEP ,
		STATE_BREAK ,
	};

	STATE m_state;
	VMCallStack* m_callStack;
	VMCallStack m_breakPoint;
	int m_sleepcount;
	int m_callStackIndex;
	int m_pc;
	int m_push;
	int m_base;
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
	void setLocal( int addres , Memory& m );
	void setMemory( Memory& src , Memory& value );
	void setMemory( Memory& src , int addr , int location );
private :
	void getMemoryInfo( Memory* p , int* addr , int* location );
	void getLocalInfo( Memory* p , int* addr , int* location );
	AsmInfo* findFunction( string name );
	int getFunctionAddr( string name );	
	void getFunction( string func );
	void vmsetup();
	void initialize( IAssembleReader* reader , VMBuiltIn* built_in , size_t stacksize , size_t staticsize );
	bool isBreak();
protected :
	virtual Memory& getStatic( int addres );
	virtual void setStatic( int addres , Memory& m );
	virtual void getStaticInfo( Memory* p , int* addr , int* location );
public :
	VMDriver();
	VMDriver( IAssembleReader* reader , VMBuiltIn* built_in );
	virtual ~VMDriver();
	void executeFunction( string funcName );
	void Return( Memory& m );
	virtual void Invoke( string& funcName );
	virtual void Sleep( int sleepTime );
	virtual void OnUpdate(){}
	Memory& popMemory();
	Memory& getMemory( int location , int address );
	void execute();
	int baseP(){ return this->m_base; }
	int sp(){ return this->m_localAddr; }
	int pushCount(){ return this->m_push; }
	void setBreakPoint( string funcName , int pc );
	void setBreakPoint( int funcAddress , int pc );
public :
};


/*
 * サブルーチン部
 * コルーチンを持ち、必要に応じて稼働させる
 */
class Subroutine : public VMDriver {
private :
	enum {
		MAX_COROUTINE =  32 ,
		STACK_SIZE    = 256 , 
	};
	Coroutine* m_coroutine;
	list<Coroutine*> m_activeList;
	list<Coroutine*> m_freeList;
public :
	Subroutine( IAssembleReader* reader , VMBuiltIn* built_in );
	~Subroutine();
	virtual void Invoke( string& funcName );
	virtual void OnUpdate();
};

/*
 * コルーチン
 * 静的領域は親となるドライバのほうを優先するように作る
 */
class Coroutine : public VMDriver {
	friend class Subroutine;
private :
	VMDriver* m_parent;
	void initialize( IAssembleReader* reader , VMBuiltIn* built_in , size_t stacksize , VMDriver* parent );
protected :
	virtual Memory& getStatic( int addres );
	virtual void setStatic( int addres , Memory& m );
	virtual void getStaticInfo( Memory* p , int* addr , int* location );
public  :
	Coroutine();
};


} // namespace Assembly
} // namespace VM
} // namespace Sencha
