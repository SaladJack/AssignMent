#pragma once
#include "CommonDefs.h"
/************************************************************************/
/* D3D�豸�����ӿ�                                                                     */
/************************************************************************/
class  DeviceManager
{
public:
	static DeviceManager* GetInstancePtr();
	static DeviceManager& GetInstance();

	ID3D11Device* GetDevice()
	{
		return m_pd3dDevice;
	}

	IDXGIFactory* GetDXGIFactory()
	{
		return m_pDXGIFactory;
	}

	ID3D11DeviceContext* GetImmediateContext()
	{
		return m_pImmediateContext;
	}
	HRESULT CreateInputLayout(LayoutVector vecLayout, const void *pBuffer, int nSize,ID3D11InputLayout **ppInputLayout);

private:
	DeviceManager();
	~DeviceManager();
private:
	ID3D11Device*           m_pd3dDevice;
	ID3D11DeviceContext*    m_pImmediateContext;

	IDXGIFactory*          m_pDXGIFactory;

};

#define g_objDeviecManager  DeviceManager::GetInstance()