#pragma once
#include "..\lexer\vmlexer.h"
#include "..\symbol\vmsymbol.h"
#include "..\assembly\vmassembly_info.h"
#include <stack>

namespace SenchaVM {
namespace Assembly {

class Scope;
class Symtable;
class SymbolInfo;
typedef shared_ptr<Symtable> CSymtable;
typedef shared_ptr<Scope>    CScope;


class MethodInfo;
class Type;

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
	Scope*         m_parent     ; // �e�X�R�[�v
	CSymtable      m_symtable   ; // �V���{���e�[�u��
	int            m_scopeLevel ; // �X�R�[�v���x��
	EScopeType     m_scopeType  ; // �X�R�[�v��ށy�֐��E�O���[�o���E�\���̓����z
	string         m_scopeName  ; // �X�R�[�v��
private :
	int getSearchSymbolType();
	SymbolInfo* const _addSymbol( string symbolName );
	Scope* getTopParent();
	
public :
	void notifyStructMethodScope(){ m_scopeType = StructMethod; }
	bool isStructScope(){ return m_scopeType == Struct; }
	bool isStructMethodScope(){ return m_scopeType == StructMethod; }
	string ScopeName(){ return m_scopeName; }
	int ScopeLevel(){ return m_scopeLevel; }
	Scope* const getParentScope(){ return m_parent; }
	Scope* const backToChildScope();
	Scope* const goToChildScope( string name );
	Type* const goToStructScope( string name );
	MethodInfo* const goToFunctionScope( string name );
	Scope* const findScope( string scopeName );
	SymbolInfo* const addSymbol( string symbolName );
	SymbolInfo* const getSymbol( string name );
	SymbolInfo* const getSymbol( int index );

	// ���̃X�R�[�v���Ǘ�����V���{�����X�g��Ԃ�
	const vector<SymbolInfo*>& getSymbols(){
		return m_symtable->getSymbols();
	}
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

	const vector<SymbolInfo*>& getChildren();
	const vector<Scope*>& getScopeChildren(){
		return m_child;
	}
	bool hasContainSymbol( string name );
	Scope( string scopeName , int scopeLevel );
	~Scope();
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


}
}
