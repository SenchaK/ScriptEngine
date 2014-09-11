#pragma once

#pragma warning( disable : 4996 )
#include<string>
#include<vector>
#include<cassert>
#include<list>
#include<iostream>
#include<sstream>
#include<memory>


#define SCOPE_LEVEL_GLOBAL ( 0 )


namespace SenchaVM{

// �g�[�N���R���e�i
// �g�[�N�i�C�U�[�ɊǗ������g�[�N���v�f�͂�����p������K�v������
class ITokenContainer {
};


// �g�[�N���C���^�[�t�F�[�X
// �\����͂��s���Ƃ��͂��̃C���^�[�t�F�[�X���Q�Ƃ���
// �����łǂ̂悤�Ɏ�������邩�͉B������悤�ɍ��
class ITokenizer {
public :
	// ��O�Ƀ|�W�V������߂�
	// @return ��O�̃g�[�N��
	virtual ITokenContainer* back() = 0;

	// �g�[�N������i�߂�
	// @return ���̃g�[�N��
	virtual ITokenContainer* next() = 0;

	// ���݂̃g�[�N����Ԃ�
	// @return ���݂̃g�[�N��
	virtual ITokenContainer* current() = 0;

	// ���݈ʒu����̃I�t�Z�b�g�l�����Z�����ʒu�̃g�[�N����Ԃ�
	// @return �w��ʒu�̃g�[�N��
	virtual ITokenContainer* offset( int ofs ) = 0;

	// ���̃g�[�N�������݂���̂�
	// @return
	// �E����ȏシ���߂��Ȃ��ꍇfalse
	// �E�i�߂���ꍇ��true
	virtual bool hasNext() = 0;
};

namespace Assembly{
class AsmInfo;

// �V���{�����
enum ESymbolType {
	Func           = 0x01 , 
	VariableField  = 0x02 ,
	VariableLocal  = 0x04 , 
	VariableGlobal = 0x08 ,
	Struct         = 0x10 ,
};

// �A�Z���u�����񋟃C���^�[�t�F�[�X
// �����R�[�h���擾����@�\��񋟂���
class IAssembleReader {
public :
	// �C���f�b�N�X�w��ŃA�Z���u�����擾
	virtual AsmInfo* getAssembly( int index ) = 0;
	// ���O�w��ŃA�Z���u�����擾
	virtual AsmInfo* getAssembly( std::string index ) = 0;
};


} // namespace Assembly
} // namespace SenchaVM