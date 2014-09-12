#include "vm_error.h"

namespace Sencha {
namespace VM {
namespace Assembly {

/*
 * エラーＩＤ列挙値を文字列にして返す
 * @return 該当するＩＤの名前を返す。存在しない場合OTHER_ERRORを返す
 */
string ERR_ID_EX::toString( ERR_ID value ){
	switch( value ){
	case ERROR_C2059 : return "ERROR_C2059";
	case ERROR_C2065 : return "ERROR_C2065";
	case ERROR_C2143 : return "ERROR_C2143";
	}
	return "OTHER_ERROR";
}

/* 
 * コンストラクタ
 * エラー情報に合わせてメッセージを生成する
 * 
 * @param param 
 * エラー情報の含まれたオブジェクト。
 * キャストして使用する。
 * 必ずパラメータは含まれていないといけない。
 * パラメータは必ずnewで生成すること
 */
VMError::VMError( VMErrorParameter* param ){
	assert( param );

	ostringstream o;
	switch( param->errId ){
	case ERROR_C2065 :
		{
			ERROR_INFO_C2065* errinfo = reinterpret_cast<ERROR_INFO_C2065*>( param );
			o << "\"" << errinfo->symbolName << "\":" << "定義されていない識別子です。";
		}
		break;
	case ERROR_C2143 :
		{
			ERROR_INFO_C2143* errinfo = reinterpret_cast<ERROR_INFO_C2143*>( param );
			o << "構文エラー : " << errinfo->tokenType << "ありません。";
		}
		break;
	case ERROR_C2059 :
		{
			ERROR_INFO_C2059* errinfo = reinterpret_cast<ERROR_INFO_C2059*>( param );
			o << "構文エラー : \"" << errinfo->s << "\"";
		}
		break;
	}

	this->m_message = ERR_ID_EX::toString( param->errId ) + ":" + o.str();
	this->m_param = param;
}

/*
 * デストラクタ
 * パラメータインスタンスを解放する
 */
VMError::~VMError(){
	assert( m_param );
	if( m_param ){
		delete m_param;
	}
}


} // namespace Assembly
} // namespace VM
} // namespace Sencha