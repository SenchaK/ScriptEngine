#include "vmerror.h"

namespace SenchaVM {
namespace Assembly {

/*
 * �G���[�h�c�񋓒l�𕶎���ɂ��ĕԂ�
 * @return �Y������h�c�̖��O��Ԃ��B���݂��Ȃ��ꍇOTHER_ERROR��Ԃ�
 */
string ERR_ID_EX::toString( ERR_ID value ){
	switch( value ){
	case ERROR_1001 : return "ERROR_1001";
	case ERROR_1002 : return "ERROR_1002";
	case ERROR_2001 : return "ERROR_2001";
	case ERROR_2002 : return "ERROR_2002";
	case ERROR_2003 : return "ERROR_2003";
	case ERROR_3001 : return "ERROR_3001";
	case ERROR_3002 : return "ERROR_3002";
	case ERROR_4001 : return "ERROR_4001";
	case ERROR_C2065 : return "ERROR_C2065";
	case ERROR_C2143 : return "ERROR_C2143";
	}
	return "OTHER_ERROR";
}

/* 
 * �R���X�g���N�^
 * �G���[���ɍ��킹�ă��b�Z�[�W�𐶐�����
 * 
 * @param param 
 * �G���[���̊܂܂ꂽ�I�u�W�F�N�g�B
 * �L���X�g���Ďg�p����B
 * �K���p�����[�^�͊܂܂�Ă��Ȃ��Ƃ����Ȃ��B
 * �p�����[�^�͕K��new�Ő������邱��
 */
VMError::VMError( VMErrorParameter* param ){
	assert( param );

	ostringstream o;
	switch( param->errId ){
	case ERROR_3001 :
		{
			ERROR_INFO_3001* errinfo = reinterpret_cast<ERROR_INFO_3001*>( param );
			o << "�\���̃X�R�[�v���Ŋ֐���錾���邱�Ƃ͂ł��܂��� [funcName " << errinfo->funcName << "]";
		}
		break;
	case ERROR_4001 :
		{
			ERROR_INFO_4001* errinfo = reinterpret_cast<ERROR_INFO_4001*>( param );
			o << "�s���ȃV���{���^�C�v" << "[symbolName " << errinfo->symbolName << "][symbolType " << errinfo->symbolType << "]";
		}
		break;
	case ERROR_C2065 :
		{
			ERROR_INFO_C2065* errinfo = reinterpret_cast<ERROR_INFO_C2065*>( param );
			o << "\"" << errinfo->symbolName << "\":" << "��`����Ă��Ȃ����ʎq�ł��B";
		}
		break;
	case ERROR_C2143 :
		{
			ERROR_INFO_C2143* errinfo = reinterpret_cast<ERROR_INFO_C2143*>( param );
			o << "�\���G���[ : " << errinfo->tokenType << "����܂���B";
		}
		break;
	case ERROR_C2059 :
		{
			ERROR_INFO_C2059* errinfo = reinterpret_cast<ERROR_INFO_C2059*>( param );
			o << "�\���G���[ : \"" << errinfo->s << "\"";
		}
		break;
	}

	this->m_message = ERR_ID_EX::toString( param->errId ) + ":" + o.str();
	this->m_param = param;
}

/*
 * �f�X�g���N�^
 * �p�����[�^�C���X�^���X���������
 */
VMError::~VMError(){
	assert( m_param );
	if( m_param ){
		delete m_param;
	}
}


} // namespace Assembly
} // namespace SenchaVM