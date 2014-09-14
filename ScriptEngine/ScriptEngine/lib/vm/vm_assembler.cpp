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
 * �������̎擾�Ɏg�p����B
 * �擪1�o�C�g�ɂ̓�������ނ��܂܂�Ă���B
 * �E�������e����(double�^)
 * �E�����񃊃e����(string�^)
 * �E���W�X�^(Memory�^)
 * �E���[�J���������͐ÓI�̈�(Memory�^)
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
 * �������
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
 * �����Z
 */
void VMDriver::_add(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src + dest );
}

/*
 * �����Z
 */
void VMDriver::_sub(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src - dest );
}

/*
 * �|���Z
 */
void VMDriver::_mul(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src * dest );
}

/*
 * ����Z
 */
void VMDriver::_div(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src / dest );
}

/*
 * �]�Z
 */
void VMDriver::_rem(){
	Memory& src = this->createOrGetMemory();
	Memory& dest = this->createOrGetMemory();
	this->setMemory( src , src % dest );
}

/*
 * �C���N�������g����
 */
void VMDriver::_inc(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , ++src );
}

/*
 * �f�N�������g����
 */
void VMDriver::_dec(){
	Memory& src = this->createOrGetMemory();
	this->setMemory( src , --src );
}

/* 
 * ��r����
 * cmpType�ɊY�������r���߂��s���A�e��r�����ɍ����Ă���ΐ^��Ԃ��B
 * @param cmpType ... ��r���ߎ��
 *
 * geq ... src��dest�����傫���������͓�����
 * g   ... src��dest�����傫��
 * leq ... src��dest�����������������͓�����
 * l   ... src��dest����������
 * eq  ... src��dest�͓�����
 * neq ... src��dest�͓������Ȃ�
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
 * �_�����Z && , || 
 * log��Logic Operation(�_�����Z)����
 * �]���l src��dest��AND��������OR�̉��Z���s���B
 * AND ... src��dest���o���U�łȂ��Ȃ�ΐ^
 * OR  ... src��dest�ǂ��炩���U�łȂ��Ȃ�ΐ^
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
 * �ے艉�Z
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
 * �������]
 */
void VMDriver::_minus(){
	Memory& src = this->createOrGetMemory();
	setMemory( src , Memory( src.value * -1 , "" ) );
}


/*
 * jmp����
 * �w��̃A�h���X�Ƀv���O�����J�E���^���ړ�������B
 */
void VMDriver::_jmp(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	this->m_pc = jmpaddr;
}

/* 
 * jz����
 * ���ʒl��0�ł���ꍇ�A�w��̃A�h���X�Ƀv���O�����J�E���^���ړ�������B
 */
void VMDriver::_jumpzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R->getMemory( 0 );
	if( r0 == 0 ){
		this->m_pc = jmpaddr;
	}
}

/*
 * jnz����
 * ���ʒl��0�ł͂Ȃ��ꍇ�A�w��̃A�h���X�Ƀv���O�����J�E���^���ړ�������B
 */
void VMDriver::_jumpnotzero(){
	int jmpaddr = currentAssembly()->moveU32( this->m_pc );
	Memory r0 = R->getMemory( 0 );
	if( r0 != 0 ){
		this->m_pc = jmpaddr;
	}
}

/*
 * push����
 * �X�^�b�N�t���[�� + ���݂̃X�^�b�N�|�C���^ + �v�b�V���񐔕��������炵���ʒu�Ƀ�������z�u����B
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
 * pop����
 * �X�^�b�N��1�߂�
 */
void VMDriver::_pop(){
	m_push--;
}

/*
 * �T�u���[�`���Ăяo������
 * ����24�r�b�g�̓A�h���X
 * ��� 8�r�b�g�͎�ނƂȂ��Ă���B
 * 0 ... �X�N���v�g���̃A�Z���u��
 * 1 ... �g�ݍ��݊֐�
 * �ƂȂ�
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
 * �X�g�A����
 * �ꎞ�v�Z��������ʗ̈�ɑޔ�������
 */ 
void VMDriver::_st(){
	int UsedRCount = this->currentAssembly()->moveU8( this->m_pc );
	this->R->store( UsedRCount );
}

/*
 * ���[�h����
 * �ꎞ�ޔ��������v�Z�����������ɖ߂��B
 */
void VMDriver::_ld(){
	int UsedRCount = this->currentAssembly()->moveU8( this->m_pc );
	this->R->load( UsedRCount );
}

/*
 * ret����
 * �߂�l��0�ԃ��W�X�^�ɔz�u����
 */
void VMDriver::_ret(){
	Memory& m = this->createOrGetMemory();
	this->R->setMemory( 0 , m );
}


/*
 * end����
 * �֐��I�����ɌĂ΂�A�R�[���X�^�b�N��1�O�̏�Ԃɖ߂��B
 * ����ȏ�O�̏�Ԃ��Ȃ��ꍇ�͂������G���g���[�|�C���g�̏I���n�_�Ȃ̂ł����ŏI���Ƃ���B
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
 * �֐����Ăяo�����s����
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
 * �v�b�V������Ă��郁����������o���B
 * �g�ݍ��݊֐��Ɉ�����n���Ƃ��Ɏg�p����B
 */
Memory& VMDriver::popMemory(){
	this->_pop();
	return this->getLocal( m_localAddr + m_push );
}

/*
 * �߂�l�Z�b�g
 */
void VMDriver::Return( Memory& m ){
	this->R->setMemory( 0 , m );
}

/*
 * �R���[�`���𑖂点��
 */ 
void VMDriver::Invoke( string& funcName ){
}

/*
 * �ꎞ������~�X�e�[�g�ɕύX
 */ 
void VMDriver::Sleep( int sleepTime ){
	this->m_state = STATE_SLEEP;
	this->m_sleepcount = sleepTime + 1;
}

} // namespace Assembly
} // namespace VM
} // namespace Sencha