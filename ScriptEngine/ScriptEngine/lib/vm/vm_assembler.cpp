#include "vm_assembler.h"
#include "vm_register.h"


namespace Sencha{
namespace VM {
namespace Assembly{

/*
 * class Subroutine
 */
Subroutine::Subroutine( IAssembleReader* reader , VMBuiltIn* built_in ) : VMDriver( reader , built_in ){
	this->m_coroutine = new Coroutine[MAX_COROUTINE];
	for( int i = 0 ; i < MAX_COROUTINE ; i++ ){
		this->m_coroutine[i].initialize( reader , built_in , 2048 , 1024 );
		this->m_freeList.push_back( &this->m_coroutine[i] );
	}
}

Subroutine::~Subroutine(){
	delete[] this->m_coroutine;
}

// virtual
void Subroutine::Invoke( string& funcName ){
	Coroutine* c = this->m_freeList.back();
	m_freeList.pop_back();
	assert( c );
	this->m_activeList.push_back( c );
	c->executeFunction( funcName );
	if( c->m_state == STATE_IDLE ){
		this->m_activeList.remove( c );
		this->m_freeList.push_back( c );
	}
}

// virtual
void Subroutine::OnUpdate(){
	list<Coroutine*>::iterator iter = this->m_activeList.begin();
	while( iter != this->m_activeList.end() ){
		Coroutine* c = *iter;
		c->execute();
		if( c->m_state == STATE_IDLE ){
			iter++;
			this->m_activeList.remove( c );
			this->m_freeList.push_back( c );
		printf( "func end : active %d , free %d\n" , this->m_activeList.size() , this->m_freeList.size() );
			continue;
		}
		iter++;
	}
}





AsmInfo* VMDriver::currentAssembly(){
	return this->m_reader->getAssembly( m_funcAddr );
}

VMDriver::VMDriver( IAssembleReader* reader , VMBuiltIn* built_in ){
	this->initialize( reader , built_in , 2048 , 1024 );
}

VMDriver::VMDriver(){
}

VMDriver::~VMDriver(){
	delete this->R;
	delete[] this->m_local;
	delete[] this->m_static;
	delete[] this->m_callStack;
}

void VMDriver::initialize( IAssembleReader* reader , VMBuiltIn* built_in , size_t stacksize , size_t staticsize ){
	assert( stacksize > 0 );
	assert( staticsize > 0 );
	assert( reader );
	this->m_state = STATE_IDLE;
	this->m_sleepcount = 0;
	this->m_funcAddr = 0;
	this->m_pc = 0;
	this->m_stacksize = 0;
	this->m_staticsize = 0;
	this->m_localAddr = 0;
	this->m_callStackIndex = 0;
	this->m_push = 0;
	this->m_base = 0;
	this->m_reader = reader;
	this->m_built_in = built_in;
	this->m_stacksize = stacksize;
	this->m_staticsize = staticsize;
	this->m_local = new Memory[stacksize];
	this->m_static = new Memory[staticsize];
	this->m_callStack = new VMCallStack[CALL_STACK_SIZE];
	this->R = new VMR();
}

void VMDriver::getMemoryInfo( Memory* p , int* addr , int* location ){
	*addr = 0;
	*location = 0;

	for( size_t i = 0 ; i < m_stacksize ; i++ ){
		Memory* m = &this->m_local[i];
		if( m == p ){
			*addr = i;
			*location = EMnemonic::MEM_L;
			return;
		}
	}
	for( size_t i = 0 ; i < m_staticsize ; i++ ){
		Memory* m = &this->m_static[i];
		if( m == p ){
			*addr = i;
			*location = EMnemonic::MEM_S;
			return;
		}
	}
}

Memory& VMDriver::getLocal( int addres ){
	assert( addres >= 0 && addres < (int)m_stacksize );
	return m_local[addres];
}
Memory& VMDriver::getStatic( int addres ){
	assert( addres >= 0 && addres < (int)m_staticsize );
	return m_static[addres];
}
void VMDriver::setLocal( int addres , Memory& m ){
	assert( addres >= 0 && addres < (int)m_stacksize );
	Memory& src = m_local[addres];
	src.setMemory( m );
}
void VMDriver::setStatic( int addres , Memory& m ){
	assert( addres >= 0 && addres < (int)m_staticsize );
	m_static[addres].setMemory( m );
}
void VMDriver::setMemory( Memory& src , Memory& value ){
	src.setMemory( value );
}

void VMDriver::setMemory( Memory& src , int addr , int location ){
	src.setMemory( addr , location );
}

Memory& VMDriver::getMemory( int location , int address ){
	static Memory Null;
	switch( location ){
	case EMnemonic::MEM_L :
		return this->getLocal( address + this->m_localAddr + this->m_base );
	case EMnemonic::MEM_S :
		return this->getStatic( address );
	}
	return Null;
}

unsigned char VMDriver::getByte( int funcAddr , int pc ){
	AsmInfo* assembly = this->m_reader->getAssembly( funcAddr );
	assert( assembly );
	return assembly->getCommand( pc );
}

bool VMDriver::isActive(){
	if( this->m_state == STATE_IDLE ){
		return false;
	}
	if( this->m_state == STATE_SLEEP ){
		this->m_sleepcount--;
		if( this->m_sleepcount <= 0 ){
			this->m_state = STATE_RUN;
			return true;
		}
		return false;
	}
	if( !this->currentAssembly() ) return false;
	if( this->currentAssembly()->hasMore( this->m_pc ) ) return true;
	if( this->m_funcAddr >= 0 ) return true;
	return false;
}

void VMDriver::execute(){
	if( this->m_state == STATE_IDLE ){
		printf( "Error: state is idle. \n");
		return;
	}

	assert( this->currentAssembly() );
	while( this->isActive() ){
		unsigned char content = this->getByte( m_funcAddr , m_pc );
		m_pc++;
		switch( content ){
			case EMnemonic::MovPtr :
				_mov_ptr();
				break;
			case EMnemonic::Mov :
				_mov();
				break;
			case EMnemonic::Add :
				_add();
				break;
			case EMnemonic::Sub :
				_sub();
				break;
			case EMnemonic::Mul :
				_mul();
				break;
			case EMnemonic::Div :
				_div();
				break;
			case EMnemonic::Rem :
				_rem();
				break;
			case EMnemonic::Inc :
				_inc();
				break;
			case EMnemonic::Dec :
				_dec();
				break;
			case EMnemonic::Push :
				_push();
				break;
			case EMnemonic::PushPtr :
				_push_ptr();
				break;
			case EMnemonic::Pop :
				_pop();
				break;
			case EMnemonic::Call :
				_call();
				break;
			case EMnemonic::ST :
				_st();
				break;
			case EMnemonic::LD :
				_ld();
				break;
			case EMnemonic::EndFunc :
				_endFunc();
				break;

			case EMnemonic::CmpGeq : 
			case EMnemonic::CmpG :
			case EMnemonic::CmpLeq : 
			case EMnemonic::CmpL :
			case EMnemonic::CmpEq : 
			case EMnemonic::CmpNEq :
				_cmp( content );
				break;
			case EMnemonic::Not :
				_not();
				break;
			case EMnemonic::Minus :
				_minus();
				break;
			case EMnemonic::LogOr :
			case EMnemonic::LogAnd :
				_log( content );
				break;
			case EMnemonic::Jmp :
				_jmp();
				break;
			case EMnemonic::JumpZero :
				_jumpzero();
				break;
			case EMnemonic::JumpNotZero :
				_jumpnotzero();
				break;
			case EMnemonic::RET :
				_ret();
				break;
		}
	}
}

AsmInfo* VMDriver::findFunction( string name ){
	return this->m_reader->getAssembly( name );
}

int VMDriver::getFunctionAddr( string name ){
	AsmInfo* assembly = this->findFunction( name );
	if( assembly ){
		return assembly->addr();
	}
	printf( "not found function [%s]\n" , name.c_str() );
	return -1;
}

void VMDriver::getFunction( string func ){
	m_funcAddr = getFunctionAddr( func );
}

void VMDriver::vmsetup(){
	this->m_state = STATE_RUN;
	this->m_callStackIndex = 0;
	this->m_pc = 0;
}

/*
 * メモリの取得に使用する。
 * 先頭1バイトにはメモリ種類が含まれている。
 * ・整数リテラル(double型)
 * ・文字列リテラル(string型)
 * ・レジスタ(Memory型)
 * ・ローカルもしくは静的領域(Memory型)
 */
Memory& VMDriver::createOrGetMemory(){
	static Memory literalMemory;

	int location = this->currentAssembly()->moveU8( this->m_pc );
	if( location == EMnemonic::LIT_VALUE ){
		literalMemory.setMemory( Memory( this->currentAssembly()->moveDouble( this->m_pc ) , "" ) );
		return literalMemory;
	}
	if( location == EMnemonic::LIT_STRING ){
		literalMemory.setMemory( Memory( 0 , this->currentAssembly()->moveString( this->m_pc ) ) );
		return literalMemory;
	}
	if( location == EMnemonic::REG ){
		return R->getMemory( this->currentAssembly()->moveU8( this->m_pc ) );
	}

	int address = 0;
	size_t size = this->currentAssembly()->moveU32( this->m_pc );
	bool isVariable = false;
	if( location == EMnemonic::MEM_S ) isVariable = true;
	if( location == EMnemonic::MEM_L ) isVariable = true;
	if( isVariable ){
		for( size_t i = 0 ; i < size ; i++ ){
			int isArray = this->currentAssembly()->moveU8( this->m_pc );
			int isRef = this->currentAssembly()->moveU8( this->m_pc );
			int addr = this->currentAssembly()->moveU32( this->m_pc );

			if( isArray ){
				int sizeOf = this->currentAssembly()->moveU32( this->m_pc );
				int RIndex = this->currentAssembly()->moveU32( this->m_pc );
				addr += sizeOf * ((int)R->getMemory( RIndex ).value);
			}
			if( isRef ){
				Memory& m = this->getMemory( location , addr );
				return this->getRefMemory( m.location , m.address , ++i , size );
			}
			address += addr;
		}
	}
	return this->getMemory( location , address );
}

Memory& VMDriver::getRefMemory( int location , int address , size_t i , size_t size ){
	for( ; i < size ; i++ ){
		int isArray = this->currentAssembly()->moveU8( this->m_pc );
		int isRef = this->currentAssembly()->moveU8( this->m_pc );
		int addr = this->currentAssembly()->moveU32( this->m_pc );
		if( isArray ){
			int sizeOf = this->currentAssembly()->moveU32( this->m_pc );
			int RIndex = this->currentAssembly()->moveU32( this->m_pc );
			addr += sizeOf * ((int)R->getMemory( RIndex ).value);
		}
		if( isRef ){
			Memory& m = this->getMemory( location , addr );
			location = m.location;
			addr     = m.address;
			return this->getRefMemory( location , addr , i , size );
		}
		address += addr;
	}

	switch( location ){
		case EMnemonic::MEM_L : return this->getLocal( address );
		case EMnemonic::MEM_S : return this->getStatic( address );
	}
	printf( "unknown location %d \n" , location );
	abort();
	return R->getMemory(0);
}


void VMDriver::_mov_ptr(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	int addr = 0;
	int location = 0;
	this->getMemoryInfo( &dest , &addr , &location );
	this->setMemory( src , addr , location );
}

/*
 * 代入命令
 */
void VMDriver::_mov(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	int addr = 0;
	int location = 0;
	getMemoryInfo( &src , &addr , &location );
	this->setMemory( src , dest );
}

/*
 * 足し算
 */
void VMDriver::_add(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src + dest );
}

/*
 * 引き算
 */
void VMDriver::_sub(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src - dest );
}

/*
 * 掛け算
 */
void VMDriver::_mul(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src * dest );
}

/*
 * 割り算
 */
void VMDriver::_div(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src / dest );
}

/*
 * 余算
 */
void VMDriver::_rem(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src % dest );
}

/*
 * インクリメント命令
 */
void VMDriver::_inc(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , ++src );
}

/*
 * デクリメント命令
 */
void VMDriver::_dec(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , --src );
}

/* 
 * 比較命令
 * cmpTypeに該当する比較命令を行い、各比較条件に合っていれば真を返す。
 * @param cmpType ... 比較命令種類
 *
 * geq ... srcがdestよりも大きいもしくは等しい
 * g   ... srcがdestよりも大きい
 * leq ... srcがdestよりも小さいもしくは等しい
 * l   ... srcがdestよりも小さい
 * eq  ... srcとdestは等しい
 * neq ... srcとdestは等しくない
 */
void VMDriver::_cmp( int cmpType ){
	bool result = 0;
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	switch( cmpType ){
		case EMnemonic::CmpGeq : result = src >= dest; break;
		case EMnemonic::CmpG   : result = src >  dest; break;
		case EMnemonic::CmpLeq : result = src <= dest; break;
		case EMnemonic::CmpL   : result = src <  dest; break;
		case EMnemonic::CmpEq  : result = src == dest; break;
		case EMnemonic::CmpNEq : result = src != dest; break;
	}
	setMemory( src , Memory( result , "" ) );
}

/*
 * 論理演算 && , || 
 * logはLogic Operation(論理演算)から
 * 評価値 srcとdestのANDもしくはORの演算を行う。
 * AND ... srcとdestが双方偽でないならば真
 * OR  ... srcとdestどちらかが偽でないならば真
 */
void VMDriver::_log( int logType ){
	bool result = 0;
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	switch( logType ){
		case EMnemonic::LogOr  : result = ( ( src != 0 ) || ( dest != 0 ) ); break;
		case EMnemonic::LogAnd : result = ( ( src != 0 ) && ( dest != 0 ) ); break;
	}
	setMemory( src , Memory( result , "" ) );
}

/*
 * 否定演算
 */
void VMDriver::_not(){
	bool result = false;
	Memory& src = this->createOrGetMemory();
	if( src.value == 0 ){
		result = true;
	}
	setMemory( src , Memory( result , "" ) );
}

/*
 * 符号反転
 */
void VMDriver::_minus(){
	Memory& src = this->createOrGetMemory();
	setMemory( src , Memory( src.value * -1 , "" ) );
}


/*
 * jmp命令
 * 指定のアドレスにプログラムカウンタを移動させる。
 */
void VMDriver::_jmp(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	this->m_pc = jmpaddr;
}

/* 
 * jz命令
 * 結果値が0である場合、指定のアドレスにプログラムカウンタを移動させる。
 */
void VMDriver::_jumpzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R->getMemory( 0 );
	if( r0 == 0 ){
		this->m_pc = jmpaddr;
	}
}

/*
 * jnz命令
 * 結果値が0ではない場合、指定のアドレスにプログラムカウンタを移動させる。
 */
void VMDriver::_jumpnotzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R->getMemory( 0 );
	if( r0 != 0 ){
		this->m_pc = jmpaddr;
	}
}

/*
 * push命令
 * スタックフレーム + 現在のスタックポインタ + プッシュ回数分だけずらした位置にメモリを配置する。
 */
void VMDriver::_push(){
	size_t stackFrame = currentAssembly()->stackFrame();
	Memory& m = this->createOrGetMemory();
	Memory& set = getLocal( stackFrame + m_push + m_localAddr );
	m_push++;
	setMemory( set , m );
}

void VMDriver::_push_ptr(){
	size_t stackFrame = currentAssembly()->stackFrame();
	Memory& m = this->createOrGetMemory();
	Memory& set = getLocal( stackFrame + m_push + m_localAddr );
	m_push++;
	int addr = m.address;
	int location = m.location;
	this->setMemory( set , addr , location );
}

/*
 * pop命令
 * スタックを1つ戻す
 */
void VMDriver::_pop(){
	m_push--;
}

/*
 * サブルーチン呼び出し命令
 * 下位24ビットはアドレス
 * 上位 8ビットは種類となっている。
 * 0 ... スクリプト内のアセンブリ
 * 1 ... 組み込み関数
 * となる
 */
void VMDriver::_call(){
	assert( this->m_callStackIndex >= 0 );
	assert( this->m_callStackIndex < CALL_STACK_SIZE );
	struct funcinfoS{
		unsigned int address : 24;
		unsigned int type    :  8;
	};
	union {
		funcinfoS info;
		int int_value;
	} func;
	func.int_value = currentAssembly()->moveU32( this->m_pc );
	this->m_callStack[m_callStackIndex].funcAddr = this->m_funcAddr;
	this->m_callStack[m_callStackIndex].prog     = this->m_pc;
	this->m_callStackIndex++;
	this->m_localAddr += this->currentAssembly()->stackFrame();
	if( func.info.type == 1 ){
		assert( this->m_built_in );
		this->m_built_in->indexAt( func.info.address )->exec( this );
		this->_endFunc();
		return;
	}
	this->m_push -= this->m_reader->getAssembly( func.info.address )->Args();
	this->m_base = this->m_push;
	this->m_funcAddr = func.info.address;
	this->m_pc = 0;
}

/*
 * ストア命令
 * 一時計算メモリを別領域に退避させる
 */ 
void VMDriver::_st(){
	int UsedRCount = this->currentAssembly()->moveU8( this->m_pc );
	this->R->store( UsedRCount );
}

/*
 * ロード命令
 * 一時退避させた計算メモリを元に戻す。
 */
void VMDriver::_ld(){
	int UsedRCount = this->currentAssembly()->moveU8( this->m_pc );
	this->R->load( UsedRCount );
}

/*
 * ret命令
 * 戻り値を0番レジスタに配置する
 */
void VMDriver::_ret(){
	Memory& m = this->createOrGetMemory();
	this->R->setMemory( 0 , m );
}


/*
 * end命令
 * 関数終了時に呼ばれ、コールスタックを1つ前の状態に戻す。
 * それ以上前の状態がない場合はそこがエントリーポイントの終了地点なのでそこで終了とする。
 */
void VMDriver::_endFunc(){
	m_callStackIndex--;
	if( m_callStackIndex < 0 ){
		m_state = STATE_IDLE;
		m_funcAddr = -1;
		return;
	}
	assert( m_callStackIndex < CALL_STACK_SIZE );
	this->m_funcAddr   = m_callStack[m_callStackIndex].funcAddr;
	this->m_pc         = m_callStack[m_callStackIndex].prog;
	this->m_localAddr -= currentAssembly()->stackFrame();
}


/* ****************************************************************************** *
 * public
 * ****************************************************************************** */ 

/*
 * 関数を呼び出し実行する
 */
void VMDriver::executeFunction( string funcName ){
	if( this->m_state == STATE_SLEEP ){
		this->execute();
		return;
	}
	this->getFunction( funcName );
	this->vmsetup();
	this->execute();
}

/*
 * プッシュされているメモリを一つ取り出す。
 * 組み込み関数に引数を渡すときに使用する。
 */
Memory& VMDriver::popMemory(){
	this->_pop();
	return this->getLocal( m_localAddr + m_push );
}

/*
 * 戻り値セット
 */
void VMDriver::Return( Memory& m ){
	this->R->setMemory( 0 , m );
}

/*
 * コルーチンを走らせる
 */ 
void VMDriver::Invoke( string& funcName ){
}

/*
 * 一時処理停止ステートに変更
 */ 
void VMDriver::Sleep( int sleepTime ){
	this->m_state = STATE_SLEEP;
	this->m_sleepcount = sleepTime + 1;
}

} // namespace Assembly
} // namespace VM
} // namespace Sencha