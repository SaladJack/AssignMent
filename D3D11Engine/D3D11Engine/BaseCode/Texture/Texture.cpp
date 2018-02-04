#include "Texture.h"
#include "TextureMgr.h"
#include <string>

#include "TextureMgr.h"

CTexture::CTexture(ID3D11ShaderResourceView* pShaderResource, int nWidth, int nHeight)
: m_pTexture(pShaderResource), m_nWidth(nWidth), m_nHeight(nHeight)
{
}

CTexture::~CTexture()
{
	SAFE_RELEASE(m_pTexture);
}
