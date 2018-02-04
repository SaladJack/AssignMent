#include "Camera.h"

#include "GameNode.h"

#include "Renderer/VertexTypes.h"
Camera::Camera() :
mDirtyProj(true),
mDirtyView(true),
orthographic_(false),
frustumDirty_(true)
{
//	SetType("Camera");
}


Camera::~Camera()
{

}

void Camera::LookAt(Vector3 pos, Vector3 target, Vector3 up)
{
	m_Pos = pos;
	m_Target = target;
	m_Up = up;
	mDirtyView = true;
	frustumDirty_ = true;
}

void Camera::updateInternals()
{
	if (mDirtyView)
	{

		m_vLookVector = (m_Target - m_Pos);
		m_vLookVector.Normalize();
		m_vRightVector = (m_Up.Cross(m_vLookVector));
		m_vRightVector.Normalize();
		m_vUpVector = m_vLookVector.Cross(m_vRightVector);

		//	把摄像机 平移回原点，在旋转根坐标轴重合
		// Rx  Ry  Rz									1 0 0
		// Ux  Uy Uz    * M == >				0 1 0
		// Lx  Ly   Lz									1 0 1
		//
		// M 为旋转的逆矩阵， 由于旋转矩阵是正交矩阵， 正交矩阵的逆矩阵 就是 它的转置矩阵
		//
		Matrix ViewMatrix(m_vRightVector, m_vUpVector, m_vLookVector);
		Matrix m = Matrix::CreateTranslation(-m_Pos);
		mViewMatrix = m * ViewMatrix.Transpose();


		mInvViewMatrix = mViewMatrix.Invert();
		mDirtyView = false;
	}
	if (mDirtyProj)
	{
		if(orthographic_)
		{
			float h = (1.0f / (orthoSize_ * 0.5f)) * 1.0f;
			float w = h / mAspectRatio;
			float q, r;

			if (false)
			{
				q = 2.0f / mFarPlane;
				r = -1.0f;
			}
			else
			{
				q = 1.0f / mFarPlane;
				r = 0.0f;
			}

			mProjMatrix.m[0][0] = w;
			mProjMatrix.m[0][3] = 0.0f;
			mProjMatrix.m[1][1] = h;
			mProjMatrix.m[1][3] = 0.0f;
			mProjMatrix.m[2][2] = q;
			mProjMatrix.m[2][3] = r;
			mProjMatrix.m[3][3] = 1.0f;

			//float h = (1.0f / (orthoSize_ * 0.5f));
			//float w = h ;
			//mProjMatrix = XMMatrixOrthographicLH(w, h, mNearPlane, mFarPlane);
		}
		else
		{
			float    SinFov;
			float    CosFov;
			XMScalarSinCos(&SinFov, &CosFov, 0.5f * mFOV);

			float Height = CosFov / SinFov;
			float Width = Height / mAspectRatio;
			float fRange = mFarPlane / (mFarPlane - mNearPlane);

			mProjMatrix.m[0][0] = Width;
			mProjMatrix.m[0][1] = 0.0f;
			mProjMatrix.m[0][2] = 0.0f;
			mProjMatrix.m[0][3] = 0.0f;

			mProjMatrix.m[1][0] = 0.0f;
			mProjMatrix.m[1][1] = Height;
			mProjMatrix.m[1][2] = 0.0f;
			mProjMatrix.m[1][3] = 0.0f;

			mProjMatrix.m[2][0] = 0.0f;
			mProjMatrix.m[2][1] = 0.0f;
			mProjMatrix.m[2][2] = fRange;
			mProjMatrix.m[2][3] = 1.0f;

			mProjMatrix.m[3][0] = 0.0f;
			mProjMatrix.m[3][1] = 0.0f;
			mProjMatrix.m[3][2] = -fRange * mNearPlane;
			mProjMatrix.m[3][3] = 0.0f;

			//mProjMatrix = XMMatrixPerspectiveFovLH(mFOV, mAspectRatio, mNearPlane, mFarPlane);
		}
		mDirtyProj = false;
	}
}



const Frustum& Camera::GetFrustum()
{
	if (frustumDirty_)
	{
		GetView();
		Vector3 eyePos = m_Pos;
		if (!orthographic_)
		{
			frustum_.Define(mFOV, mAspectRatio, 1.0f, mNearPlane, mFarPlane, mInvViewMatrix);
			frustum_.ConstructFrustum(mViewMatrix, mProjMatrix);
		}
		else
		{
			frustum_.DefineOrtho(orthoSize_, mAspectRatio, 1.0f, mNearPlane, mFarPlane, mInvViewMatrix);
		}
		frustumDirty_ = false;
	}
	return frustum_;
}


const Matrix& Camera::GetView() const
{
	if (mDirtyView)
	{
		// Note: view matrix is unaffected by node or parent scale
		mInvViewMatrix = GetEffectiveWorldTransform();
		mViewMatrix = mInvViewMatrix.Invert();
		mDirtyView = false;
	}

	return mViewMatrix;
}

Matrix Camera::GetEffectiveWorldTransform() const
{
	if (node_)
	{
		return node_->GetWorldTransform();
	}
	return Matrix::Identity;
}

void Camera::OnNodeSet(GameNode * node)
{
	if (node)
		node->AddListener(this);
}

void Camera::OnMarkedDirty(GameNode* node)
{
	mDirtyView = true;
	frustumDirty_ = true;
}

DirectX::SimpleMath::Vector3 Camera::ScreenToWorldPoint(const Vector3& screenPos) const
{
	Ray ray = GetScreenRay(screenPos.x, screenPos.y);
	return ray.position +ray.direction * screenPos.z;
}

DirectX::SimpleMath::Ray Camera::GetScreenRay(float x, float y) const
{
	Ray ret;
	Matrix mInvProMatrix = mProjMatrix.Invert();

	Matrix viewProjInverse = mInvProMatrix* mInvViewMatrix;
	// The parameters range from 0.0 to 1.0. Expand to normalized device coordinates (-1.0 to 1.0) & flip Y axis


	x = 2.0f * x - 1.0f;
	y = 1.0f - 2.0f * y;
	Vector3 vNear(x, y, 0.0f);
	Vector3 vFar(x, y, 1.0f);
	Vector3 VText = Vector3::Transform(vNear, mInvProMatrix);
	ret.position = Vector3::Transform(vNear, viewProjInverse);
	ret.direction = Vector3::Transform(vFar, viewProjInverse) - ret.position;
	ret.direction.Normalize();
	return ret;

}

void Camera::GetFrustumSize(Vector3& fNear, Vector3& fFar) const
{
	fNear.z = mNearPlane;
	fFar.z = mFarPlane;

	float halfViewSize = tanf(mFOV / 2.0f);
	Vector3 vNear, vFar;

	fNear.y = fNear.z * halfViewSize;
	fNear.x = fNear.y* mAspectRatio;

	fFar.y = fFar.z * halfViewSize;
	fFar.x = fFar.y* mAspectRatio;

}

