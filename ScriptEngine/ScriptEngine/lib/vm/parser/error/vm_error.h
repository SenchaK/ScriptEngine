#pragma once
#include "../../vm_define.h"


namespace Sencha {
namespace VM {
namespace Assembly {
using namespace std;

enum ERR_ID {
	ERROR_C2065 , // "symbolName" : ��`����Ă��Ȃ����ʎq�ł��B
	ERROR_C2059 , // �\���G���[ : 's'
	ERROR_C2143 , // �\���G���[ : ']' �� ';' �̑O�ɂ���܂���B
};

/*
 * �G���[�񋓌^�̊g���@�\
 */
class ERR_ID_EX{
public :
	/*
	 * �h�c�𕶎���ɂ���
	 */
	static string toString( ERR_ID value );
};

/*
 * �G���[���̃p�����[�^���
 * �G���[�̎�ނɂ���ė~�����p�����[�^���قȂ�̂Œ��ۓI�ɃI�u�W�F�N�g�̂ݗp�ӂ���
 */
class VMErrorParameter {
public :
	ERR_ID errId;
public :
	VMErrorParameter( ERR_ID errid ){
		this->errId = errid;
	}
	virtual ~VMErrorParameter(){
	}
};

class ERROR_INFO_C2065 : public VMErrorParameter {
public :
	string symbolName;
	ERROR_INFO_C2065( const string& symbolName ) : VMErrorParameter( ERROR_C2065 ){
		this->symbolName = symbolName;
	}
};

class ERROR_INFO_C2143 : public VMErrorParameter {
public :
	int tokenType;
	ERROR_INFO_C2143( int tokenType ) : VMErrorParameter( ERROR_C2143 ){
		this->tokenType = tokenType;
	}
};

class ERROR_INFO_C2059 : public VMErrorParameter {
public :
	string s;
	ERROR_INFO_C2059( string s ) : VMErrorParameter( ERROR_C2059 ){
		this->s = s;
	}
};

/*
 * ���z�@�B�֌W�ł̃G���[�����Ȃ�
 */
class VMError {
protected :
	string m_message;
	VMErrorParameter* m_param;
public :
	VMError( VMErrorParameter* param );
	virtual ~VMError();
	const string& getMessage(){
		return m_message;
	}
};


} // Assembly
} // VM
} // Sencha