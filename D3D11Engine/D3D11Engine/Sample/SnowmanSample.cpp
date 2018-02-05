#include "SnowmanSample.h"
#include "SampleBase.h"
#include "BaseCode/Texture/TextureMgr.h"

SAMPLE_CPP(SnowmanSample)
{
	m_fxaaEnabled = true;
}

SnowmanSample::~SnowmanSample()
{
}

void SnowmanSample::InitResource()
{
	Sample::InitResource();
	snowSphereGO.InitResource(GEOMETRY_TYPE_SPHERE);
	snowSphereGO.SetTexture("Data\\Texture\\Snow2.dds");
	blackSphereGO.InitResource(GEOMETRY_TYPE_SPHERE);
	blackSphereGO.SetTexture("Data\\Texture\\Stone.dds");
	boxGO.InitResource(GEOMETRY_TYPE_BOX);
	boxGO.SetTexture("Data\\Texture\\Box.dds");
	handGO.InitResource(GEOMETRY_TYPE_CYLINDER);
	handGO.SetTexture("Data\\Texture\\Wood.dds");

	cameraNode_->SetPosition(Vector3(0.0f, 30.0f, -45.0f));
	pitch_ = 30.0f;
	Quaternion q = Quaternion::CreateFromEulerAngles(pitch_, 0.0f, 0.0f);
	cameraNode_->SetRotation(q);
	cameraScreenNode_ = scene_->CreateChild("cameraScreenNode");
	
	Vector3 eyePos(-30.0f, 40.0f, 2.0f);
	cameraScreenNode_->SetPosition(eyePos);
	GameNode* pNode = cameraScreenNode_->CreateChild("CameraNode");
	pNode->SetRotation(Quaternion::CreateFromEulerAngles(30, 0, 0));
	float fieldOfView = (float)XM_PI / 3.0f;
	float AspectHByW = (float)mClientWidth / (float)mClientHeight;
	cameraScreen = pNode->CreateComponent<Camera>();
	cameraScreen->ProjParams(fieldOfView, AspectHByW, 1.0f, 100.0f);

	int nCount = 3;
	XMFLOAT3 eyePos1(-30.0f, 30.0f, 18.0f);
	XMVECTOR q0 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(0.0));
	XMVECTOR q1 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(90.0f));
	XMVECTOR q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(180.0f));
	XMVECTOR q3 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(70.0f));
	mCamerAnimation.Keyframes.resize(nCount);
	mCamerAnimation.Keyframes[0].TimePos = 0.0f;
	mCamerAnimation.Keyframes[0].Translation = eyePos1;
	mCamerAnimation.Keyframes[0].Scale = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	XMStoreFloat4(&mCamerAnimation.Keyframes[0].RotationQuat, q0);

	mCamerAnimation.Keyframes[1].TimePos = 2.0f;
	mCamerAnimation.Keyframes[1].Translation = XMFLOAT3(-35.0f, 35.0f, 20.0f);
	mCamerAnimation.Keyframes[1].Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMStoreFloat4(&mCamerAnimation.Keyframes[1].RotationQuat, q1);


	mCamerAnimation.Keyframes[2].TimePos = 10.0f;
	//mCamerAnimation.Keyframes[2].Translation = XMFLOAT3(-10.0f, 18.0f, -5.0f);
	mCamerAnimation.Keyframes[2].Translation = XMFLOAT3(-30.0f, 30.0f, 15.0f);
	mCamerAnimation.Keyframes[2].Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMStoreFloat4(&mCamerAnimation.Keyframes[2].RotationQuat, q2);
	mCamerTimPos = 0.0f;
	colorRT = std::make_shared<RenderTarget2D>(mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	depthRT = std::make_shared<RenderTarget2D>(mClientWidth, mClientHeight, DXGI_FORMAT_R32_FLOAT,true);
	m_pDepthShader = g_objMaterial.GetShader("HLSL\\ShadowSample\\Depth.hlsl");
	m_pShadowShader = g_objMaterial.GetShader("HLSL\\ShadowSample\\Shadow.hlsl");


}

void SnowmanSample::UpdateScene(float fTotalTime, float fDeltaTime)
{

	Sample::UpdateScene(fTotalTime, fDeltaTime);
	bool bLoop = true;
	float fDt = fDeltaTime / 1000.0f;

	mCamerTimPos += fDt*0.5f;
	if (mCamerTimPos >= mCamerAnimation.GetEndTime() && bLoop)	
	{
		mCamerTimPos = 0.0f;// 
	}
	XMFLOAT4X4 mSkullWorld;
	Aniframe aniFrame = mCamerAnimation.Interpolate(mCamerTimPos, mSkullWorld);

	cameraScreenNode_->SetWorldPosition(aniFrame.Translation);
	Quaternion rotate = aniFrame.RotationQuat;
	cameraScreenNode_->SetWorldRotation(rotate);

	if (KEYDOWN(VK_SPACE))
	{
		float cameraX = cameraNode_->GetWorldPosition().x;
		float cameraZ = cameraNode_->GetWorldPosition().z;
		Matrix s = boxPos * Matrix::CreateRotationY(XM_PI / 3 * mTimer.TotalTime());
		float boxX = Vector3::Transform(Vector3::Zero,s).x;
		float boxZ = Vector3::Transform(Vector3::Zero, s).z;
		float distance = (cameraX - boxX) * (cameraX - boxX) + (cameraZ - boxZ) * (cameraZ - boxZ);
		if(distance < 11*11)
			bFollowBox = !bFollowBox;
	}

	if (bFollowBox)
	{
		Matrix s = boxPos * Matrix::CreateRotationY(XM_PI / 3 * mTimer.TotalTime());
		cameraNode_->SetPosition(Vector3::Transform(Vector3(0, 10, 0), s));
	}
	

}
void SnowmanSample::DrawScene()
{
	SwapChainPtr->Begin();
	Matrix mWorld;
	Matrix mView;
	Matrix mProj;
	if (cameraMain)
	{
		mView = cameraMain->GetView();
		mProj = cameraMain->GetProjection();
	}
	TurnZBufferOn();
	RenderRT();

	ID3D11RenderTargetView* pRTV = SwapChainPtr->GetRenderTargetView();
	ID3D11DepthStencilView* pDSV = SwapChainPtr->GetDepthStencilView();
	pRTV = colorRT->GetRTView();
	float ClearColor[4] = { sqrt(0.25f), sqrt(0.25f), sqrt(0.5f), 0.0f };
	m_deviceContext->ClearRenderTargetView(pRTV, ClearColor);
	m_deviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);
	// Rebind to original back buffer and depth buffer
	ID3D11RenderTargetView * pRTVs[2] = { pRTV, NULL };
	pRTVs[0] = colorRT->GetRTView();
	m_deviceContext->OMSetRenderTargets(1, pRTVs, pDSV);

	Vector3 lightPos = cameraScreen->GetNode()->GetWorldPosition();
	Vector4 ambientColor = Vector4(0.15f, 0.15f, 0.15f, 1.0f);
	Vector4 diffuseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	gameObject.SetMaterial(m_pShadowShader);
	Matrix lightViewMatrix, lightProjectionMatrix;
	if (cameraScreen)
	{
		lightViewMatrix = cameraScreen->GetView();
		lightProjectionMatrix = cameraScreen->GetProjection();
	}
	Matrix rotate = Matrix::CreateRotationY(XM_PI / 3 * mTimer.TotalTime());
	mWorld = Matrix::CreateScale(3, 3, 3);
	DefaultShadowMaterial(mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);
	ID3D11SamplerState* LinearClamp = g_objStates.LinearClamp();
	m_pShadowShader->PSSetShaderResources(TU_DIFFUSE, gameObject.GetTexture());
	m_pShadowShader->PSSetShaderResources(TU_CUBE, depthRT->GetSRView());

	m_deviceContext->PSSetSamplers(0, 1, &LinearClamp);
	m_deviceContext->PSSetSamplers(1, 1, &LinearClamp);
	m_pShadowShader->PSSetConstantBuffers("ambientColor", &ambientColor);
	m_pShadowShader->PSSetConstantBuffers("diffuseColor", &diffuseColor);

	gameObject.SetMaterial(m_pShadowShader);
	gameObject.Render();

	

	RenderSnowMan(Matrix::CreateTranslation(0, 0, 0));

	mWorld = boxSize * boxPos * rotate;
	RenderObjInDrawScene(boxGO, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);

	RenderSnowMan(boxPos * rotate);
	


	Vector3 eyePos = cameraNode_->GetWorldPosition();
	mWorld = Matrix::CreateTranslation(eyePos.x, eyePos.y, eyePos.z);
	

	SkyBoxPtr->Render(mWorld*mView*mProj);

	SwapChainPtr->Begin();
	// apply FXAA
	TurnZBufferOff();
	if (m_fxaaEnabled)
	{
		MaterialPtr pFXAAShader = g_objMaterial.GetShader("HLSL\\FXAA.hlsl");
		m_deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		float frameWidth = (float)mClientWidth;
		float frameHeight = (float)mClientHeight;
		Vector4  vFxaa = Vector4(1.0f / frameWidth, 1.0f / frameHeight, 0.0f, 0.0f);

		pFXAAShader->VSSetConstantBuffers("RCPFrame", &vFxaa);
		pFXAAShader->PSSetConstantBuffers("RCPFrame", &vFxaa);

		pFXAAShader->PSSetShaderResources(TU_DIFFUSE, colorRT->GetSRView());
		pFXAAShader->Apply();
		m_deviceContext->Draw(3, 0);
	}

	SwapChainPtr->Flip();
}

void SnowmanSample::RenderRT()
{
	depthRT->Begin();
	Matrix worldMatrix, lightViewMatrix, lightProjectionMatrix;
	if (cameraScreen)
	{
		lightViewMatrix = cameraScreen->GetView();
		lightProjectionMatrix = cameraScreen->GetProjection();
	}
	worldMatrix = Matrix::CreateScale(3, 3, 3);
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	gameObject.SetMaterial(m_pDepthShader);
	gameObject.Render();

	
	RenderSnowManShadow(Matrix::CreateTranslation(0, 0, 0));
	
	// box
 	Matrix rotate = Matrix::CreateRotationY(XM_PI / 3 * mTimer.TotalTime());
	worldMatrix = boxSize * boxPos * rotate;
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	boxGO.SetMaterial(m_pDepthShader);
	boxGO.Render();

	RenderSnowManShadow(boxPos * rotate);

	depthRT->End();
	//depthRT->Save("depth.png");
}

void SnowmanSample::DefaultDepthMaterial(Matrix& worldMatrix, Matrix & viewMatrix,Matrix & projMatrix)
{
	m_pDepthShader->VSSetConstantBuffers("worldMatrix", &worldMatrix);
	m_pDepthShader->VSSetConstantBuffers("viewMatrix", &viewMatrix);
	m_pDepthShader->VSSetConstantBuffers("projectionMatrix", &projMatrix);
}

void SnowmanSample::DefaultShadowMaterial(Matrix & worldMatrix, Matrix & viewMatrix, Matrix & projMatrix, Matrix & lightViewMatrix, Matrix & lightProjectionMatrix, Vector3 & lightPos)
{
	m_pShadowShader->VSSetConstantBuffers("worldMatrix", &worldMatrix);
	m_pShadowShader->VSSetConstantBuffers("viewMatrix", &viewMatrix);
	m_pShadowShader->VSSetConstantBuffers("projectionMatrix", &projMatrix);
	m_pShadowShader->VSSetConstantBuffers("lightViewMatrix", &lightViewMatrix);
	m_pShadowShader->VSSetConstantBuffers("lightProjectionMatrix", &lightProjectionMatrix);
	m_pShadowShader->VSSetConstantBuffers("lightPosition", &lightPos);
}

void SnowmanSample::RenderObjInDrawScene(GameObject & go, Matrix & worldMatrix, Matrix & viewMatrix, Matrix & projMatrix, Matrix & lightViewMatrix, Matrix & lightProjectionMatrix, Vector3 & lightPos)
{
	DefaultShadowMaterial(worldMatrix, viewMatrix, projMatrix, lightViewMatrix, projMatrix, lightPos);
	m_pShadowShader->PSSetShaderResources(TU_DIFFUSE, go.GetTexture());
	go.SetMaterial(m_pShadowShader);
	go.Render();
}

void SnowmanSample::RenderSnowMan(Matrix & snowmanPos)
{
	Matrix mWorld;
	Matrix mView;
	Matrix mProj;
	if (cameraMain)
	{
		mView = cameraMain->GetView();
		mProj = cameraMain->GetProjection();
	}

	Matrix lightViewMatrix, lightProjectionMatrix;
	if (cameraScreen)
	{
		lightViewMatrix = cameraScreen->GetView();
		lightProjectionMatrix = cameraScreen->GetProjection();
	}

	Vector3 lightPos = cameraScreen->GetNode()->GetWorldPosition();
	// body
	mWorld = bodySize * bodyPos * snowmanPos;
	RenderObjInDrawScene(gameSphereObject, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);

	// head
	mWorld = headSize * headPos* snowmanPos;
	RenderObjInDrawScene(gameSphereObject, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);

	// eye
	mWorld = eyeSize * lEyePos* snowmanPos;
	RenderObjInDrawScene(blackSphereGO, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);
	mWorld = eyeSize * rEyePos* snowmanPos;
	RenderObjInDrawScene(blackSphereGO, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);

	// hand
	mWorld = handSize * lHandPos * snowmanPos;
	RenderObjInDrawScene(handGO, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);
	mWorld = handSize * rHandPos * snowmanPos;
	RenderObjInDrawScene(handGO, mWorld, mView, mProj, lightViewMatrix, lightProjectionMatrix, lightPos);
}

void SnowmanSample::RenderSnowManShadow(Matrix & snowmanPos)
{
	Matrix worldMatrix, lightViewMatrix, lightProjectionMatrix;
	if (cameraScreen)
	{
		lightViewMatrix = cameraScreen->GetView();
		lightProjectionMatrix = cameraScreen->GetProjection();
	}
	// body
	worldMatrix = bodySize * bodyPos * snowmanPos;
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	gameSphereObject.SetMaterial(m_pDepthShader);
	gameSphereObject.Render();

	// head
	worldMatrix = headSize * headPos * snowmanPos;
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	gameSphereObject.SetMaterial(m_pDepthShader);
	gameSphereObject.Render();

	// hand
	worldMatrix = handSize * lHandPos * snowmanPos;
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	handGO.SetMaterial(m_pDepthShader);
	handGO.Render();
	worldMatrix = handSize * rHandPos * snowmanPos;
	DefaultDepthMaterial(worldMatrix, lightViewMatrix, lightProjectionMatrix);
	handGO.SetMaterial(m_pDepthShader);
	handGO.Render();
}
