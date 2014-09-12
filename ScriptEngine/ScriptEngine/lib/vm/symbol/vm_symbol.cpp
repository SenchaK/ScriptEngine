#include "../../vm/parser/error/vm_error.h"
#include "vm_symbol.h"
#include "../parser/vm_scope.h"


namespace Sencha{
namespace VM {
namespace Assembly{

/* class SymbolInfo */
// �O����̐����͂������g�p����
SymbolInfo::SymbolInfo( string name , size_t arrayLength , ESymbolType symbolType , bool isReference , int scopeLevel ){
	m_name = name;
	m_addr = 0;
	m_arrayLength = arrayLength;
	m_symbolType = symbolType;
	m_isReference = isReference;
	m_scopeLevel = scopeLevel;
	m_dataTypeId = 0;
	if( m_scopeLevel == SCOPE_LEVEL_GLOBAL ){
		m_symbolType = VariableGlobal;
	}
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
 * ��������q�V���{���͑S�č폜����B
 * �����̃V���{���̎Q�Ƃ��q�V���{���ɂ���Ƃ�����������delete���Ă΂��̂ł����������Ƃ͂��Ȃ�
 * (�V���{���P��ŊǗ����Ȃ��ƃ_��)
 */
SymbolInfo::~SymbolInfo(){
}

/*
 * ���݂̃V���{����ޔԍ�����
 * �O���[�o���ł���̂��A���[�J���ł���̂��̒��ԃR�[�h��Ԃ�
 * @throw �V���{����ނ��s���ȏꍇ�A�G���[�Ƃ���
 */
int SymbolInfo::toCode(){
	switch( m_symbolType ){
		case VariableGlobal : return EMnemonic::MEM_S;
		case VariableLocal  : return EMnemonic::MEM_L;
		case VariableField  : return EMnemonic::MEM_THIS_P;
	}

	std::cout << "�s���ȃV���{���^�C�v [m_symbolType:" << m_symbolType << "][name:" << this->m_name << "]" << std::endl;
	abort();
	return -1;
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
	for( unsigned int i = 0 ; i < m_symbolList.size() ; i++ ){
		SymbolInfo* symbol = m_symbolList[i];
		delete symbol;
	}
	m_symbolList.clear();
}

} // namespace Assembly
} // namespace VM
} // namespace Sencha