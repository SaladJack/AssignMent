#include "Sample.h"
#include "SampleBase.h"

Sample::Sample(HINSTANCE hInstance, int nWidth /*= 1024*/, int nHeight /*= 600*/):
D3D11App(hInstance)
{
	mClientWidth = nWidth;
	mClientHeight = nHeight;
	pitch_ = yaw_ = 0.0f;
	m_pTextureSampler = NULL;
	m_pPointSampler = NULL;
	m_pWriteStencilDSState = NULL;
	m_pTestStencilDSState = NULL;
	bKeyDown = false;

	m_bBlend = false;
	m_bTurnOn = false;
	m_CullMode = MAX_CULLMODES;
}


Sample::~Sample()
{
	SAFE_RELEASE(m_pTextureSampler);
	SAFE_RELEASE(m_pPointSampler);
	SAFE_RELEASE(m_pWriteStencilDSState);
	SAFE_RELEASE(m_pTestStencilDSState);

}

void Sample::InitResource()
{
	DeviceCreated();
	SkyBoxPtr = std::make_shared<SkyBox>();
	SkyBoxPtr->InitResource();
	gameObject.InitResource(GEOMETRY_TYPE_GRID);
	gameObject.SetTexture("Data\\Texture\\metal001.dds");
	gameSphereObject.InitResource(GEOMETRY_TYPE_SPHERE);
	gameSphereObject.SetTexture("Data\\Texture\\Snow2.dds");

	CreateScene();

}

void Sample::CreateScene()
{
	scene_ = std::make_shared<Scene>();
	cameraNode_ = scene_->CreateChild("Camera");
	cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -30.0f));
	pitch_ = -5.0f;
	Quaternion q = Quaternion::CreateFromEulerAngles(pitch_, 0.0f, 0.0f);
	cameraNode_->SetRotation(q);
	float fieldOfView = (float)XM_PI / 4.0f;
	float AspectHByW = (float)mClientWidth / (float)mClientHeight;
	cameraMain = cameraNode_->CreateComponent<Camera>();
	cameraMain->ProjParams(fieldOfView, AspectHByW, 1.0f, 1000.0f);
}


void Sample::UpdateScene(float fTotalTime, float fDeltaTime)
{
	const float MOVE_SPEED = 20.0f;
	float dt = fDeltaTime / 1000.0f;
	if (::GetFocus())
	{
		if (::GetAsyncKeyState('W') & 0x8000)
			cameraNode_->Translate(Vector3::Backward * MOVE_SPEED * dt);
		if (::GetAsyncKeyState('S') & 0x8000)
			cameraNode_->Translate(Vector3::Forward * MOVE_SPEED * dt);
		if (::GetAsyncKeyState('A') & 0x8000)
			cameraNode_->Translate(Vector3::Left * MOVE_SPEED * dt);
		if (::GetAsyncKeyState('D') & 0x8000)
			cameraNode_->Translate(Vector3::Right * MOVE_SPEED * dt);
	}
}

void Sample::RenderSample()
{
	TurnZBufferOn();
	Matrix mWorld;
	Matrix mView;
	Matrix mProj;
	if (cameraMain)
	{
		mView = cameraMain->GetView();
		mProj = cameraMain->GetProjection();
	}
	return;
	gameObject.Render(Matrix::CreateScale(5, 5, 5), mView, mProj);
	gameSphereObject.Render(Matrix::CreateScale(3, 3, 3), mView, mProj);


	Vector3 eyePos = cameraNode_->GetWorldPosition();
	mWorld = Matrix::CreateTranslation(eyePos.x, eyePos.y, eyePos.z);
	SkyBoxPtr->Render(mWorld*mView*mProj);
}

void Sample::DrawScene()
{
	SwapChainPtr->Begin();
	RenderSample();
	SwapChainPtr->Flip();
}

void Sample::OnMouseUp(WPARAM btnState, int x, int y)
{
	D3D11App::OnMouseUp(btnState, x, y);
}

void Sample::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (bMouseDown)
	{
		if (mouseLast.bLeftDown)
		{
			int nX = x - mouseLast.X;
			int nY = y - mouseLast.Y;
			// Mouse sensitivity as degrees per pixel
			const float MOUSE_SENSITIVITY = 0.1f;
			yaw_ += MOUSE_SENSITIVITY * nX;
			pitch_ += MOUSE_SENSITIVITY * nY;
			pitch_ = MathHelper::Clamp(pitch_, -90.0f, 90.0f);
			Quaternion q = Quaternion::CreateFromEulerAngles(pitch_, yaw_, 0.0f);
			cameraNode_->SetRotation(q);
		}
		else
		{

		}
	}
	D3D11App::OnMouseMove(btnState, x, y);
}

HRESULT Sample::DeviceCreated()
{
	HRESULT hr;

	// Create Depth Stencil State
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		dsDesc.DepthEnable = false;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		dsDesc.StencilEnable = true;
		dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.BackFace = dsDesc.FrontFace;
		V_RETURN(m_d3dDevice->CreateDepthStencilState(&dsDesc, &m_pWriteStencilDSState));
	}

	// Create Samplers
	{
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = sampDesc.AddressV = sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = FLT_MAX;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		V_RETURN(m_d3dDevice->CreateSamplerState(&sampDesc, &m_pTextureSampler));
	}

	return S_OK;

}

void Sample::AlphaBlend()
{
	if (!m_bBlend)
	{
		m_bBlend = true;
		FLOAT BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };// 0xFFFFFFFF
		m_deviceContext->OMSetBlendState(g_objStates.AlphaBlend(), BlendFactor, 0xFFFFFFFF);
	}
}

void Sample::Opaque()
{
	if (m_bBlend)
	{
		m_bBlend = false;
		FLOAT BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };// 0xFFFFFFFF
		m_deviceContext->OMSetBlendState(g_objStates.Opaque(), BlendFactor, 0xFFFFFFFF);
	}
}

void Sample::TurnZBufferOn()
{
	if (!m_bTurnOn)
	{
		ID3D11DeviceContext*  d3dcontext = g_objDeviecManager.GetImmediateContext();
		d3dcontext->OMSetDepthStencilState(g_objStates.DepthDefault(), 1);
		m_bTurnOn = true;
	}
}

void Sample::TurnZBufferOff()
{
	if (m_bTurnOn)
	{
		ID3D11DeviceContext*  d3dcontext = g_objDeviecManager.GetImmediateContext();
		d3dcontext->OMSetDepthStencilState(g_objStates.DepthNone(), 1);
		m_bTurnOn = false;
	}
}

void Sample::SetState(CullMode cullMode_)
{
	if (m_CullMode != cullMode_)
	{
		m_CullMode = cullMode_;
		switch (cullMode_)
		{
		case  CULL_NONE:
		{
			m_deviceContext->RSSetState(g_objStates.CullNone());
		}
		break;
		case  CULL_CCW:
		{
			m_deviceContext->RSSetState(g_objStates.CullCounterClockwise());
		}
		break;
		case  CULL_CW:
		{
			m_deviceContext->RSSetState(g_objStates.CullClockwise());
		}
		break;
		}
	}
}
