#pragma once
#include "HpD3D9Type.h"

ID3D11ShaderResourceView* D3DX11CreateShaderResourceViewFromFile(
	ID3D11Device*  pDevice,
	const char*   strFileName);

//��ȡ����2D�ľ���
XMMATRIX GetShow2DMatrix(int nWidth, int nHegith);