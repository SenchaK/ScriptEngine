#pragma once
#include "vmdefine.h"
#include "vmmemory.h"
#include "assembly\vmassembly_info.h"
#include "assembly\vm_mnemonic_define.h"
#include <stack>



namespace SenchaVM{
namespace Assembly{
using namespace std;


struct memoryinfoS {
	string label;
	vmbyte type;
	vmbyte isArray;
	vmbyte isReferenceMember;
	vmbyte isReference;
	int index;
	int offsetAddres;
	double value;
	string value_string;
	memoryinfoS(){
		initialize();
	}
	memoryinfoS( vmbyte type , int index ){
		initialize();
		this->type = type;
		this->index = index;
	}
	void initialize(){
		this->type = 0;
		this->isArray = 0;
		this->isReferenceMember = 0;
		this->isReference = 0;
		this->index = 0;
		this->offsetAddres = 0;
		this->value = 0;
		this->value_string = "";
		this->label = "";
	}
};

// ソースとデスティネーションのペアを持つ構造体
struct valuepairS {
	memoryinfoS src;
	memoryinfoS dest;

	// デスティネーションはリテラルなのかメモリなのか判別する必要がある
	bool isMemory(){
		if( dest.type == EMnemonic::LIT_VALUE )  return false;
		if( dest.type == EMnemonic::LIT_STRING ) return false;
		return true;
	}
	void dump( int mnemonic , int stackp );
};


/* **************************************************
 * アセンブルコマンドを実行する
 * ************************************************** */
class VMStack;
class VMDriver;
class VMMainRoutineDriver;
class VMCoroutineDriver;
typedef shared_ptr<VMStack>             CVMStack;
typedef shared_ptr<VMDriver>            CVMDriver;
typedef shared_ptr<VMMainRoutineDriver> CVMMainRoutineDriver;
typedef shared_ptr<VMCoroutineDriver>   CVMCoroutineDriver;

class VMDriverInterface {
private :
	VMDriver* m_driver;
protected :
	Memory& popMemory();
	void Return( const double& value );
	void Return( const string& value );
	void Return( const double& value , const string& string_value );
	void VMSleep( int sleeptime );
public :
	VMDriverInterface();
	virtual void onInit( VMDriver* driver );
};


class VMSystemCallService : public VMDriverInterface {
private :
	VMDriver* m_driver;
	list<VMSystemCallService*> m_system;
protected :
	void addSystemCallService( VMSystemCallService* service );
public :
	~VMSystemCallService();
	virtual void onInit( VMDriver* driver );
	virtual void callFunction( string funcName );
};
typedef shared_ptr<VMSystemCallService> CVMSystemCallService;


class VMDriver {
protected :
	enum STATE {
		DRIVER_IDLE    ,
		DRIVER_EXECUTE , 
		DRIVER_SLEEP   ,
		DRIVER_END     ,
	};
	typedef enum {
		STK_SIZE = 2048 ,
	};
	VMCallStack m_callStack[STK_SIZE];
	CVMSystemCallService m_systemCall;
	int m_callStackIndex;
	int m_funcAddr;
	int m_prog;
	int m_push;
	int m_basePointer;
	int m_systemcall_push;
	int m_sleepcount;
	int m_sleeptime;
	STATE m_state;
public :
	VMDriver();
	virtual void setMemory( Memory& src , Memory& value ) = 0;
	virtual Memory& getMemory( const memoryinfoS& info ) = 0;
	virtual vmbyte getByte( int funcAddr , int pc ) = 0;
	virtual bool isProgramEnd() = 0;
	Memory& popMemory();
	void setSystemCallService( CVMSystemCallService systemcall );
	void VMSleep( int sleeptime );
protected :
};



class VMMainRoutineDriver : public VMDriver {
private :
	vector<AssemblyInfo> m_assemblyInfo;
	VMDriver* m_current;
	CVMCoroutineDriver m_activeCoroutine;
	size_t m_localAddr;
	size_t m_stacksize;
	size_t m_staticsize;
	size_t m_coroutinesize;
	CMemory m_local;
	CMemory m_static;
	CVMCoroutineDriver m_coroutine;
private :
	AssemblyInfo* currentAssembly();
	valuepairS createValuePair();
	void callSubroutine( int addres );
	void startCoroutine( int addres );
	void updateCoroutine();
	void _pmov();
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
	void _jumpzero();
	void _jumpnotzero();
	void _jmp();
	void _push();
	void _pop();
	void _call();
	void _endFunc();
	void _asString();
	void _arrayIndexSet();
	void _arrayIndexAdd();
	void _ret();

	Memory& getLocal( int addres );
	Memory& getStatic( int addres );
	void setLocal( int addres , Memory& m );
	void setStatic( int addres , Memory& m );
protected :
	virtual vmbyte getByte( int funcAddr , int pc ) override;
	virtual bool isProgramEnd() override;
	AssemblyInfo* findFunction( string name );
	int getFunctionAddr( string name );	
	void getFunction( string func );
	void vmsetup();
	void execute();
public :
	VMMainRoutineDriver();
	void executeFunction( string funcName );
	virtual void initialize( const VMAssembleCollection& context , size_t stacksize , size_t staticsize , size_t coroutinesize );
public :
	virtual void setMemory( Memory& src , Memory& value ) override;
	virtual Memory& getMemory( const memoryinfoS& info ) override;
	Memory& getMemoryAbsolute( int memoryScope , int addres );
};

class VMCoroutineDriver : public VMMainRoutineDriver {
private :
	CMemory m_local;
	VMDriver* m_Parent;
protected :
	virtual vmbyte getByte( int funcAddr , int pc ) override;
	virtual bool isProgramEnd() override;
public :
	virtual void initialize( VMDriver* parentRoutine , size_t stacksize );
	virtual void setMemory( Memory& src , Memory& value ) override;
	virtual Memory& getMemory( const memoryinfoS& info ) override;
};


} // namespace Assembly
} // namespace SenchaVM
