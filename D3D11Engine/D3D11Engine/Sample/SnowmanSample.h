#pragma once
#include "Sample.h"
#include "MaterialMgr.h"
#include "Framework/AnimationHelper.h"
class SnowmanSample : public Sample
{
public:
	SAMPLE_H(SnowmanSample);
	~SnowmanSample();
	virtual void UpdateScene(float fTotalTime, float fDeltaTime);
	virtual void DrawScene();
protected:
	virtual void InitResource();
	class GameNode* cameraScreenNode_;
	class Camera* cameraScreen;

private:
	void RenderRT();
	void DefaultDepthMaterial(Matrix& worldMatrix, Matrix& viewMatrix, Matrix& projMatrix);
	void DefaultShadowMaterial(Matrix& worldMatrix, Matrix& viewMatrix, Matrix& projMatrix, Matrix& lightViewMatrix, Matrix& lightProjectionMatrix, Vector3& lightPos);
	void RenderObjInDrawScene(GameObject& go,Matrix& worldMatrix, Matrix& viewMatrix, Matrix& projMatrix, Matrix& lightViewMatrix, Matrix& lightProjectionMatrix, Vector3& lightPos);
	void RenderSnowMan(Matrix& snowmanPos);
	void RenderSnowManShadow(Matrix& snowmanPos);
private:
	RenderTarget2DPtr colorRT;
	bool m_fxaaEnabled;
	bool bFollowBox;
	RenderTarget2DPtr depthRT; //color and specular intensity
	MaterialPtr m_pDepthShader;

	MaterialPtr m_pShadowShader;
	AnimationHelper::BoneAnimation mCamerAnimation;
	float mCamerTimPos;

	GameObject snowSphereGO;
	GameObject blackSphereGO;
	GameObject boxGO;
	GameObject handGO;

	Matrix boxSize = Matrix::CreateScale(6.f, 6.f, 6.f);					Matrix boxPos = Matrix::CreateTranslation(15.f, 3.f, 0.f);
	Matrix bodySize = Matrix::CreateScale(3.f, 3.f, 3.f);					Matrix bodyPos = Matrix::CreateTranslation(0.f, 3.f, 0.f);
	Matrix headSize = Matrix::CreateScale(2.f, 2.f, 2.f);					Matrix headPos = Matrix::CreateTranslation(0.f,7.8f,0.f);
	Matrix eyeSize  = Matrix::CreateScale(0.2f, 0.2f, 0.2f);				Matrix lEyePos = Matrix::CreateTranslation(-0.5f,9.f,-1.8f); Matrix rEyePos = Matrix::CreateTranslation(0.5f, 9.f, -1.8f);
	Matrix handSize = Matrix::CreateScale(0.5f, 0.5f, 0.5f);				Matrix lHandPos = Matrix::CreateRotationZ(-XM_PI / 4) * Matrix::CreateTranslation(3.f, 5.f, 0.f); Matrix rHandPos = Matrix::CreateRotationZ(XM_PI / 4) * Matrix::CreateTranslation(-3.f, 5.f, 0.f);

};

