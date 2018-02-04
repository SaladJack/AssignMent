#include "GameObject.h"
#include "Renderer/D3D11RendererMaterial.h"
#include "Renderer/GemoetryRender.h"
#include "Basecode/DXFunction.h"
#include "Renderer/DeviceManager.h"
#include "Renderer/CommonStates.h"
#include "BaseCode/Texture/TextureMgr.h"
#include "Renderer/DeviceManager.h"
#include "MaterialMgr.h"
GameObject::GameObject()
{
	m_strName = "";
	m_bInitState = false;
	//	_CrtSetBreakAlloc(531); // �����ڵڼ����ڴ�����ʱ�������ִͣ�У����ظ��ֳ�

	//_CrtDumpMemoryLeaks(); // ��鵱ǰ����Щ�ڴ�û���ͷ�.
}

void GameObject::SetMaterial(std::shared_ptr<class D3D11RendererMaterial> material)
{
	m_MaterialPtr = material;
}

void GameObject::InitResource(GEOMETRY_TYPE type)
{
	RendererMaterialDesc desc;

	 m_MaterialPtr = g_objMaterial.GetShader("HLSL\\GameObject.hlsl");

	GemoetryRenderPtr = std::make_shared<GemoetryRender>();

	GeoGen::MeshData meshData;
	switch (type)
	{
	case GEOMETRY_TYPE_BOX:
	{
		GeoGen::CreateBox(1, 1, 1, meshData);

	}
	break;
	case GEOMETRY_TYPE_SPHERE:
	{
		GeoGen::CreateSphere(1, 50, 50, meshData);

	}
	break;
	case GEOMETRY_TYPE_GRID:
	{
		GeoGen::CreateGrid(20.0f, 20.0f, 20, 20, meshData);
		GeoGen::CreateGrid(20.0f, 20.0f, 1, 1, meshData);

	}
	break;
	case GEOMETRY_TYPE_BOX_EX:
	{
		GeoGen::CreateBox(10, 1, 1, meshData);
	}
	break;
	case GEOMETRY_TYPE_CYLINDER:
	{
		//GeoGen::CreateCylinder(0.02f, 0.06f, 1.6f, 10, 10, meshData);
		GeoGen::CreateCylinder(0.2f, 0.6f, 10.0f, 50, 50, meshData);
	}
	}
	bool bBuild = GemoetryRenderPtr->BuildBuffers(meshData);
	m_bInitState = true;


}

void GameObject::SetTexture(const char* pszName)
{
	m_strName = pszName;
}

void GameObject::Render(Matrix world, Matrix view, Matrix proj, bool bTest)
{
	ID3D11DeviceContext* pImmediateContext = g_objDeviecManager.GetImmediateContext();
	m_MaterialPtr->VSSetConstantBuffers("worldMatrix", &world);
	if (!bTest)
	{
		m_MaterialPtr->VSSetConstantBuffers("viewMatrix", &view);
		m_MaterialPtr->VSSetConstantBuffers("projectionMatrix", &proj);
	}

	CTexture* pTexture = g_objTextureMgr.GetTexture(m_strName);
	if (pTexture)
	{
		ID3D11ShaderResourceView* pSrv = pTexture->GetShaderResourceView();
		m_MaterialPtr->PSSetShaderResources(TU_DIFFUSE, pSrv);

		ID3D11SamplerState* LinearWrap = g_objStates.LinearWrap();
		pImmediateContext->PSSetSamplers(TU_DIFFUSE, 1, &LinearWrap);
		//pImmediateContext->OMSetDepthStencilState(g_objStates.DepthDefault(), 1);
		//pImmediateContext->RSSetState(g_objStates.CullNone());
		FLOAT BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };// 0xFFFFFFFF
		//pImmediateContext->OMSetBlendState(g_objStates.Opaque(), BlendFactor, 0xFFFFFFFF);
	}
	m_MaterialPtr->Apply();
	GemoetryRenderPtr->render(m_MaterialPtr.get());
}

void GameObject::Render()
{
	m_MaterialPtr->Apply();
	GemoetryRenderPtr->render(m_MaterialPtr.get());
}



void GameObject::SetLightBuffer(Vector3 lightPosition, Vector4 ambientColor, Vector4 diffuseColor)
{
	m_MaterialPtr->VSSetConstantBuffers("lightPosition", &lightPosition);
	m_MaterialPtr->PSSetConstantBuffers("ambientColor", &ambientColor);
	m_MaterialPtr->PSSetConstantBuffers("diffuseColor", &diffuseColor);

}

ID3D11ShaderResourceView* GameObject::GetTexture()
{
	CTexture* pTexture = g_objTextureMgr.GetTexture(m_strName);
	if (pTexture)
	{
		return pTexture->GetShaderResourceView();
	}
	return NULL;

}


