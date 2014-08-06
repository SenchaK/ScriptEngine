#include "../../vm/parser/error/vmerror.h"
#include "vmsymbol.h"
#include "../parser/vmscope.h"


namespace SenchaVM{
namespace Assembly{

/* class SymbolInfo */
// �O����̐����͂������g�p����
SymbolInfo::SymbolInfo( string name , size_t arrayLength , ESymbolType symbolType , bool isReference , bool isStruct , int scopeLevel ){
	m_name = name;
	m_addr = 0;
	m_arrayLength = arrayLength;
	m_symbolType = symbolType;
	m_isReference = isReference;
	m_isStruct = isStruct;
	m_scopeLevel = scopeLevel;
	m_dataTypeId = 0;
	m_arrayIndexR = 0;
	if( m_scopeLevel == SCOPE_LEVEL_GLOBAL ){
		m_symbolType = VariableGlobal;
	}
	m_parent = NULL;
	m_type = NULL;
	m_isArray = false;
}

string SymbolInfo::DataTypeName(){
	if( !m_type ) return "var";
	return m_type->ScopeName();
}

int SymbolInfo::SizeOf(){
	if( !this->m_type ){
		return 1 * this->m_arrayLength;
	}
	return this->m_type->SizeOf() * this->m_arrayLength;
}

/*
 * private 
 * �|�C���^����V���{���R�s�[���s���B
 * �q�V���{�������Ɏg���֐��Ȃ̂Őe�ƃR�s�[�������݂��Ă��邱�Ƃ���O��
 * �O����͌ĂׂȂ�
 */
SymbolInfo::SymbolInfo( SymbolInfo* src , SymbolInfo* parent ){
	VM_ASSERT( src );
	VM_ASSERT( parent );
	this->m_name         = src->m_name;
	this->m_addr         = src->m_addr;
	this->m_arrayLength  = src->m_arrayLength;
	this->m_scopeLevel   = src->m_scopeLevel;
	this->m_symbolType   = parent->SymbolType();
	this->m_isReference  = src->m_isReference;
	this->m_isStruct     = src->m_isStruct;
	this->m_dataTypeId   = src->m_dataTypeId;
	this->m_parent       = parent;
	this->m_type         = src->m_type;
	this->m_arrayIndexR  = 0;
	this->m_isArray      = false;
}

/*
 * ��������q�V���{���͑S�č폜����B
 * �����̃V���{���̎Q�Ƃ��q�V���{���ɂ���Ƃ�����������delete���Ă΂��̂ł����������Ƃ͂��Ȃ�
 * (�V���{���P��ŊǗ����Ȃ��ƃ_��)
 */
SymbolInfo::~SymbolInfo(){
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		SymbolInfo* symbol = m_child[i];
		delete symbol;
	}
	m_child.clear();
	//printf( "SymbolInfo Finish %s \n" , m_name.c_str() );
}

/*
 * �ŏ�ʂɂ���V���{���A�h���X���擾����
 */
size_t SymbolInfo::TopSymbolAddr(){
	if( m_parent ){
		return m_parent->TopSymbolAddr();
	}
	return m_addr;
}

/*
 * �Q�ƌ^�ł��邩�ǂ���
 * ��{�I�ɂ͍\���̃����o�ɂ����K������Ȃ�
 */
int SymbolInfo::isReferenceMember(){
	if( this->m_parent ){
		if( m_parent->isReferenceMember() ){
			return 1;
		}
		if( this->m_parent->m_isReference ){
			return 1;
		}
	}
	return 0;
}


/*
 * ���݂̃V���{����ޔԍ�����
 * �O���[�o���ł���̂��A���[�J���ł���̂��̒��ԃR�[�h��Ԃ�
 * @throw �V���{����ނ��s���ȏꍇ�A�G���[�Ƃ���
 */
int SymbolInfo::toAssembleCode(){
	switch( m_symbolType ){
		case VariableGlobal : return EMnemonic::MEM_S;
		case VariableLocal  : return EMnemonic::MEM_L;
		case VariableField  : return EMnemonic::MEM_THIS_P;
	}
	throw VMError( new ERROR_INFO_4001( m_name , m_symbolType ) );
}

SymbolInfo* const SymbolInfo::addSymbol( string name ){
	int addr = m_child.size();
	SymbolInfo* symbol = NULL;
	m_child.push_back( new SymbolInfo( name , 1 , VariableField , false , false , 1 ) );
	symbol = m_child[m_child.size()-1];
	symbol->m_addr = addr;
	symbol->m_parent = this;
	return symbol;
}

/*
 * �Ώۂ̃V���{�������q�V���{���𕡐����Ēǉ�����
 * �Q�ƌ^�ł���ꍇ�q�V���{���̓|�C���^�����o�A�e�͎���(this)�Ƃ��Ĉ���
 */
void SymbolInfo::copyAndAddChildrenOfSymbol( SymbolInfo* symbol ){
	assert( symbol );
	copyAndAddChildrenOfSymbol( symbol->m_child );
}

/*
 * �q�V���{���𕡐����Ēǉ�����B
 */
void SymbolInfo::copyAndAddChildrenOfSymbol( const vector<SymbolInfo*>& children ){
	for( unsigned int i = 0 ; i < children.size() ; i++ ){
		VM_ASSERT( children[i] );
		SymbolInfo* symbol = new SymbolInfo( children[i] , this );
		symbol->copyAndAddChildrenOfSymbol( children[i] );
		this->m_child.push_back( symbol );
	}
}

/*
 * �q�K�w�ɑ��݂���V���{���̃A�h���X�ɑ΂��鏈���B
 * �e�A�h���X�̑��΃A�h���X�ɐ��K������
 * �Q�ƌ^�̓f�[�^�̍\��������m���Ă���Ηǂ��̂ł��̏����͕K�v�Ȃ�
 */
void SymbolInfo::setupChildrenAddresToParentAddresOffset(){
	int addrtop = 0;
	if( m_isReference ){
		return;
	}
	for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
		this->m_child[i]->m_addr = this->m_addr + addrtop;
		addrtop += m_child[i]->getSymbolCount();
	}
}

/*
 * ���O����V���{�����擾����
 */
SymbolInfo* const SymbolInfo::getSymbol( string name ){
	for( vector<SymbolInfo*>::iterator iter = m_child.begin() ; iter != m_child.end() ; iter++ ){
		if( name.compare( (*iter)->Name() ) == 0 ){
			return ( *iter );
		}
	}
	return NULL;
}

/*
 * �V���{�������擾
 * �z�񒷂��ƃV���{�����̂��̂����f�[�^�̃T�C�Y���l�����Čv�Z����
 */
int SymbolInfo::getSymbolCount(){
	if( m_isReference ){
		return 1;
	}
	return this->SizeOf();
}

/*
 * �Q�ƌ^�ł���ꍇ�̓f�[�^�T�C�Y�͕K��1(���Ԃ��������A�f�[�^�\�������킩���Ă���Ηǂ��̂�)
 * �q�V���{�������ꍇ�͎q�V���{���̃T�C�Y���܂߂��g�[�^���̃T�C�Y��Ԃ��B
 * �q�V���{���������Ȃ��ꍇ�͕K��1��Ԃ�
 */
size_t SymbolInfo::DataSize(){
	if( m_isReference ){
		return 1;
	}
	if( m_child.size() > 0 ){
		int childCount = 0;
		for( unsigned int i = 0 ; i < m_child.size() ; i++ ){
			childCount += m_child[i]->DataSize();
		}
		return childCount;
	}
	return 1;
}


/* class Symtable */
SymbolInfo* const Symtable::findSymbol( string symbolName ){
	for( vector<SymbolInfo*>::iterator iter = m_symbolList.begin() ; iter != m_symbolList.end() ; iter++ ){
		if( symbolName.compare( (*iter)->Name() ) == 0 ){
			return ( *iter );
		}
	}
	return NULL;
}

SymbolInfo* const Symtable::findSymbol( int addr ){
	for( vector<SymbolInfo*>::iterator iter = m_symbolList.begin() ; iter != m_symbolList.end() ; iter++ ){
		if( ( *iter )->Addr() == addr ){
			return ( *iter );
		}
	}
	return NULL;
}

SymbolInfo* const Symtable::getSymbol( int index ){
	assert( index >= 0 && index < (int)m_symbolList.size() );
	if( index < 0 || index >= (int)m_symbolList.size() ) return NULL;
	return m_symbolList[index];
}

SymbolInfo* const Symtable::peekSymbol(){
	size_t size = m_symbolList.size();
	int index = size - 1;
	if( size <= 0 ) return NULL;
	if( index < 0 ) return NULL;
	return m_symbolList[index];
}

void Symtable::entrySymbol( SymbolInfo* symbol ){
	symbol->Addr( m_symbolList.size() );
	m_symbolList.push_back( symbol );
}
#undef ENTITY_OF

// �e�V���{���̎��q�V���{���̐����v�Z����������Ԃ�
int Symtable::getSymbolTotal(){
	int result = 0;
	for( vector<SymbolInfo*>::iterator iter = m_symbolList.begin() ; iter != m_symbolList.end() ; iter++ ){
		int childCount = ( *iter )->ChildSymbolCount();
		if( childCount > 0 ){
			childCount--;
		}
		result += childCount;
	}
	result += m_symbolList.size();
	return result;
}

int Symtable::getSymbolCount( int symbolMask ){
	int result = 0;
	for( unsigned int i = 0 ; i < m_symbolList.size() ; i++ ){
		if( m_symbolList[i]->SymbolType() == symbolMask ){
			result += m_symbolList[i]->getSymbolCount();
		}
	}
	return result;
}

// �Ǘ����Ă�V���{���̑S�T�C�Y�擾
int Symtable::sizeOf(){
	int size = 0;
	for( unsigned int i = 0 ; i < m_symbolList.size() ; i++ ){
		size += m_symbolList[i]->SizeOf();
	}
	return size;
}

Symtable::~Symtable(){
	//VM_PRINT( "Symtable Destroy\n" );
	for( unsigned int i = 0 ; i < m_symbolList.size() ; i++ ){
		SymbolInfo* symbol = m_symbolList[i];
		delete symbol;
	}
	m_symbolList.clear();
}

} // namespace Assembly
} // namespace SenchaVM