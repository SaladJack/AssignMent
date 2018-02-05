#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include <string>
#include "CommonDefs.h"

class TextureMgr;
// Class that wraps information about a texture. This class 
// won't be used directly by the users. Instead, they will
// manipulate the CImage class.
class   CTexture
{
	friend class TextureMgr;
public:
	ID3D11ShaderResourceView* GetShaderResourceView(){ return m_pTexture; }

	int  GetWidth(){ return m_nWidth; }
	int  GetHegith(){ return m_nHeight; }

protected:
	// Constructor which takes the filename as argument.
	// It loads the file and throw an exception if the load
	// failed.
	CTexture(ID3D11ShaderResourceView* pShaderResource,int nWidth,int nHeight);
	~CTexture();
private:
	ID3D11ShaderResourceView* m_pTexture;               //纹理指针
	ID3D11Device			*m_d3dDevice;				//D3D11设备
	ID3D11DeviceContext		*m_deviceContext;			//设备上下文
	int m_nWidth;
	int m_nHeight;
};

#endif  // _TEXTURE_H_