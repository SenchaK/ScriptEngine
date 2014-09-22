#pragma once

#include "stream.h"

namespace Sencha{
namespace Util {

/*
 * �r�b�g�f�[�^�ǂݍ��݃N���X
 * �X�g���[�����R���X�g���N�^�����ɓn���A�r�b�g�P�ʂŃf�[�^���V�[�P���X���Ď擾����B
 * �擪�o�C�g����ǂݐi�߂邽�߁A�`���̓r�b�O�G���f�B�A���ɂ����Ή����Ă��Ȃ��B
 */
class BitReader{
private :
	CStream m_stream;
	int m_index;
public :
	BitReader( CStream stream );
};

} // namespace Util
} // namespace Sencha