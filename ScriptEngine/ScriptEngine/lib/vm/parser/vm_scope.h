#pragma once
#include "..\lexer\vm_lexer.h"
#include "..\symbol\vm_symbol.h"
#include "..\assembly\vm_assembly_info.h"
#include <stack>

namespace Sencha {
namespace VM{
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


class MethodInfo;
class Type;
class VMDriver;

// �X�R�[�v�Ǘ��N���X
class Scope{
protected :
	enum EScopeType {
		Global         ,
		Function       , 
		Struct         , 
		StructMethod   ,
	};
	vector<Scope*> m_child      ; // �q�X�R�[�v 
	Scope* m_parent             ; // �e�X�R�[�v
	CSymtable m_symtable        ; // �V���{���e�[�u��
	int m_scopeLevel            ; // �X�R�[�v���x��
	EScopeType m_scopeType      ; // �X�R�[�v��ށy�֐��E�O���[�o���E�\���̓����z
	string m_scopeName          ; // �X�R�[�v��
private :
	int getSearchSymbolType();
	SymbolInfo* const _addSymbol( string symbolName );
	Scope* getTopParent();	
public :
	// �R���X�g���N�^
	// ���O�ƃX�R�[�v���x���������l�Ƃ��ăZ�b�g����
	Scope( string scopeName , int scopeLevel );

	// �f�X�g���N�^
	// �Ǘ����Ă���q�X�R�[�v���폜����B
	virtual ~Scope();

	// ���̃X�R�[�v�͍\���̃X�R�[�v�ł���ƒʒm
	void notifyStructMethodScope(){ m_scopeType = StructMethod; }

	// �\���̃X�R�[�v���ǂ���
	bool isStructScope(){ return m_scopeType == Struct; }

	// �\���̃��\�b�h�X�R�[�v�ł��邩�ǂ���
	bool isStructMethodScope(){ return m_scopeType == StructMethod; }

	// �X�R�[�v��
	string ScopeName(){ return m_scopeName; }

	// �X�R�[�v���x��
	int ScopeLevel(){ return m_scopeLevel; }

	// ���̐e�X�R�[�v���擾
	Scope* const getParentScope(){ return m_parent; }

	// ��O�̃X�R�[�v�ɖ߂�
	Scope* const backToChildScope();

	// �q�X�R�[�v�֐i��
	// �i�߂��q�X�R�[�v�̎Q�Ƃ�Ԃ�
	Scope* const goToChildScope( string name );

	// �\���̃X�R�[�v�֐i��
	// �i�߂��q�X�R�[�v�̎Q�Ƃ�Ԃ�
	Type* const goToStructScope( string name );

	// �֐��X�R�[�v�֐i��
	// �i�߂��q�X�R�[�v�̎Q�Ƃ�Ԃ�
	MethodInfo* const goToFunctionScope( string name );

	// ���O����X�R�[�v��T��
	Scope* const findScope( string scopeName );

	// �V���{����o�^
	// �������ꂽ�V���{����Ԃ�
	SymbolInfo* const addSymbol( string symbolName );

	// �V���{���𖼑O���猟�����Ď擾����
	// �Ώۂ̖��O�V���{����Ԃ�
	SymbolInfo* const getSymbol( string name );

	// �e�[�u���C���f�b�N�X����V���{�����擾
	// �Ώۂ̃V���{����Ԃ�
	SymbolInfo* const getSymbol( int index );

	// �X�R�[�v�̃t���l�[�����擾����B
	// �e�X�R�[�v��.�q�X�R�[�v���ƘA�����ꂽ�����񂪃t���l�[���ƂȂ�
	string toFullName( const string& funcName );

	// ���̃X�R�[�v���Ǘ�����V���{�����X�g��Ԃ�
	const vector<SymbolInfo*>& getSymbols(){
		return m_symtable->getSymbols();
	}

	// ��ԏ�̃X�R�[�v����Ώۂ̖��O�̃X�R�[�v����������B
	Scope* findScopeFromTop( string scopeName );

	// ���݂̃X�R�[�v�����q�X�R�[�v�̐����擾����
	// ���X�R�[�v�̓J�E���g���Ȃ�
	int getChildScopeCount(){ return m_child.size(); }

	// �Ώۂ̃V���{���^�C�v�������������A���̃X�R�[�v�ɑ��݂���V���{���̐����擾����
	// ���K�w�A�e�K�w�ɑ��݂���V���{�����͊܂߂Ȃ�
	int getSymbolCount( int symbolMask );

	// ���݂̃X�R�[�v�ɂ��킹���V���{�������擾����
	// �O���[�o���X�R�[�v:�O���[�o�������̃V���{���̂�
	// �֐��X�R�[�v      :���[�J�������̃V���{���̂�
	// �\���̃X�R�[�v    :�\���̑����̃V���{���̂�
	int getSymbolCountInScopeAttribute();

	// ���݂̃X�R�[�v���猩����S�Ă̎q�X�R�[�v�̐e�K�w���܂߂��V���{���������擾���A
	// �ł������V���{��������Ԃ�
	int getSymbolCountMaxInAllScope( ESymbolType symbolType );

	// �e�X�R�[�v�̎��V���{������Ԃ�
	int getParentSymbolCount( ESymbolType symbolType );

	// �ˑ��֌W�ɂ���S�Ă̐e�X�R�[�v��H��A
	// �����Ă���V���{���������擾
	int getAllParentSymbolCount( ESymbolType symbolType );

	// �ˑ��֌W�ɂ���S�Ă̐e�X�R�[�v�̎��V���{�������y��
	// ���݂̃X�R�[�v�̏�������V���{���������ׂĂ̍��v�l���擾����
	int getAllSymbolCount( ESymbolType symbolType );

	// �Ώۂ̖��O�̃V���{�������̃X�R�[�v�A�������͐e�K�w���Ǘ����Ă��邩�ǂ���
	bool hasContainSymbol( string name );
};

// �\���̂Ȃǂ̌^���̏ꍇ
class Type : public Scope {
public :
	Type( string scopeName , int scopeLevel ) : Scope( scopeName , scopeLevel ){
	}
	string Name(){
		return this->ScopeName();
	}
	int SizeOf();
};


// �֐��X�R�[�v
class MethodInfo : public Scope {
public :
	MethodInfo( string scopeName , int scopeLevel ) : Scope( scopeName , scopeLevel ){
	}
};

// ���O��ԃX�R�[�v
class Package : public Scope {
private :
	VMBuiltIn* m_built_in;
public :
	Package( string scopeName , int scopeLevel );
	virtual ~Package();
	void insertMethod( string methodName , void(*function)(VMDriver*) );
};


} // namespace Assembly
} // namespace VM
} // namespace Sencha
