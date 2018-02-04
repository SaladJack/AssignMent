#include "GameNode.h"
#include "Scene.h"
#include "Component.h"

GameNode::GameNode()
{

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	children_.clear();
	parent_ = NULL;
	worldTransform_ = Matrix::Identity;
	scene_ = NULL;
	position_ = Vector3::Zero;
	rotation_ = Quaternion::Identity;
	scale_ = Vector3::One;
	worldRotation_ = Quaternion::Identity;
	dirty_ = false;
	
}

GameNode::~GameNode()
{
	RemoveAllChildren();
	RemoveAllComponents();
	// Remove from the scene
	if (scene_)
	{
		scene_->NodeRemoved(this);
	}
	scene_ = NULL;
}

GameNode* GameNode::CreateChild(const String& name /*= String::EMPTY*/, 
	CreateMode mode /*= REPLICATED*/, unsigned id /*= 0*/)
{
	GameNode* newNode = CreateChild(id, mode);
	newNode->SetName(name);
	return newNode;
}

GameNode* GameNode::CreateChild(unsigned id, CreateMode mode)
{
	SPNode newNode(new GameNode());
	// If zero ID specified, or the ID is already taken, let the scene assign
	if (scene_)
	{
		if (!id || scene_->GetNode(id))
			id = scene_->GetFreeNodeID(mode);
		newNode->SetID(id);
	}
	else
		newNode->SetID(id);

	AddChild(newNode);
	return newNode.get();
}


void GameNode::SetName(const String& name)
{
	if (name != name_)
	{
		name_ = name;
		nameHash_ = name_;
	}
}

void GameNode::AddChild(SPNode node, unsigned index /*= M_MAX_UNSIGNED*/)
{
	if (!node)
	{
		return;
	}
	GameNode* pNode = node.get();
	if (!pNode || pNode == this || pNode->parent_ == this)
	{
		return;
	}
	// Check for illegal or redundant parent assignment
	// Check for possible cyclic parent assignment
	GameNode* parent = parent_;
	while (parent)
	{
		if (parent == pNode)
			return;
		parent = parent->parent_;
	}
	VEC_SPNODE::iterator i = children_.begin();
	SPNode nodeShared(node);
	children_.push_back(nodeShared);
	node->parent_ = this;
	node->MarkDirty();
}

void GameNode::SetPosition(const Vector3& position)
{
	position_ = position;
	MarkDirty();
}

void GameNode::MarkDirty()
{
	dirty_ = true;
	// Notify listener components first, then mark child nodes
	VEC_WP_COMPONENT::iterator iter = listeners_.begin();
	for (; iter != listeners_.end();)
	{
		if ((*iter))
		{
			(*iter)->OnMarkedDirty(this);
			++iter;
		}
		// If listener has expired, erase from list
		else
			iter = listeners_.erase(iter);
	}
	VEC_SPNODE::iterator i = children_.begin();
	for (; i != children_.end(); ++i)
		(*i)->MarkDirty();

}

void GameNode::SetWorldPosition(const Vector3& position)
{
	//设置该节点的位置
	//该节点的位置 = 该节点的局部位置* 父亲的位置
	// --> 该节点的局部位置 = 该节点的位置 * 父亲inv
	if (parent_)
	{
		Matrix m = parent_->GetWorldTransform();
		m = m.Invert();
		Vector3 vPos = Vector3::Transform(position, m);
		SetPosition(vPos);
	}
	else
	{
		SetPosition(position);
	}

}

void GameNode::SetWorldRotation(const Quaternion& rotation)
{
	if (parent_)
	{
		Quaternion rotationInv;
		parent_->GetWorldRotation().Inverse(rotationInv);
		SetRotation(rotation *rotationInv);
	}
	else
	{
		SetRotation(rotation);
	}
}

void GameNode::SetWorldScale(const Vector3& scale)
{
	if (parent_)
	{
		Vector3 vScale = parent_->GetWorldScale();
		vScale = scale / vScale;
		vScale = scale / vScale;
		SetScale(vScale);
	}
	else
	{
		SetScale(scale);
	}
}


void GameNode::UpdateWorldTransform()const
{
	Matrix transform = GetTransform();
	if (!parent_)
	{
		worldTransform_ = transform;
		worldRotation_ = rotation_;
	}
	else
	{
		//获取父亲节点的位置
		Matrix parentTransform = parent_->GetWorldTransform();
		//自己的局部位置*父亲的矩阵 = 自己的世界坐标系
		worldTransform_ = transform *parentTransform;
		worldRotation_ = rotation_ * parent_->GetWorldRotation();
	}

	dirty_ = false;
}

Matrix GameNode::GetTransform() const
{
	Matrix M;
	XMVECTOR S = XMLoadFloat3(&scale_);
	XMVECTOR P = XMLoadFloat3(&position_);
	XMVECTOR Q = XMLoadFloat4(&rotation_);
	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	return M;
}

void GameNode::SetRotation(const Quaternion& rotation)
{
	rotation_ = rotation;
	MarkDirty();
}

void GameNode::SetScale(float scale)
{
	SetScale(Vector3(scale, scale, scale));
}

void GameNode::SetScale(const Vector3& scale)
{
	scale_ = scale;
	// Prevent exact zero scale e.g. from momentary edits as this may cause division by zero
	// when decomposing the world transform matrix
	static const float M_EPSILON = 0.000001f;
	if (scale_.x == 0.0f)
		scale_.x = M_EPSILON;
	if (scale_.y == 0.0f)
		scale_.y = M_EPSILON;
	if (scale_.z == 0.0f)
		scale_.z = M_EPSILON;
	MarkDirty();
}

void GameNode::SetTransformSilent(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	position_ = position;
	rotation_ = rotation;
	scale_ = scale;
}

void GameNode::SetTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	position_ = position;
	rotation_ = rotation;
	scale_ = scale;
	MarkDirty();
}

GameNode* GameNode::GetChild(unsigned index) const
{
	return index < children_.size() ? children_[index].get() : 0;
}

GameNode* GameNode::GetChild(const String& name, bool recursive /*= false*/) const
{
	return GetChild(StringHash(name), recursive);
}

GameNode* GameNode::GetChild(const char* name, bool recursive /*= false*/) const
{
	return GetChild(StringHash(name), recursive);
}

GameNode* GameNode::GetChild(StringHash nameHash, bool recursive /*= false*/) const
{
	for (VEC_SPNODE::const_iterator i = children_.begin(); i != children_.end(); ++i)
	{
		if ((*i)->GetNameHash() == nameHash)
			return (*i).get();

		if (recursive)
		{
			GameNode* node = (*i)->GetChild(nameHash, true);
			if (node)
				return node;
		}
	}

	return 0;
}

void GameNode::RemoveChild(GameNode* pNode)
{
	if (!pNode)
		return;
	VEC_SPNODE::iterator i = children_.begin();
	for (; i != children_.end(); ++i)
	{
		if ((*i).get() == pNode)
		{
			RemoveChild(i);
			return;
		}
	}
}

void GameNode::SetID(unsigned id)
{
	id_ = id;
}

void GameNode::SetScene(Scene* scene)
{
	scene_ = scene;
}

void GameNode::RemoveAllChildren()
{
	RemoveChildren(true, true, true);
}

void GameNode::RemoveChildren(bool removeReplicated, bool removeLocal, bool recursive)
{
	unsigned numRemoved = 0;

	for (unsigned i = children_.size() - 1; i < children_.size(); --i)
	{
		bool remove = false;
		GameNode* childNode = children_[i].get();

		if (recursive)
			childNode->RemoveChildren(removeReplicated, removeLocal, true);
		if (childNode->GetID() < FIRST_LOCAL_ID && removeReplicated)
			remove = true;
		else if (childNode->GetID() >= FIRST_LOCAL_ID && removeLocal)
			remove = true;

		if (remove)
		{
			RemoveChild(children_.begin() + i);
			++numRemoved;
		}
	}

	// Mark node dirty in all replication states
	if (numRemoved)
		MarkReplicationDirty();
}

void GameNode::RemoveChild(VEC_SPNODE::iterator iter)
{
	// Send change event. Do not send when already being destroyed
	GameNode* child = (*iter).get();

	if (child && scene_)
	{

	}

	child->parent_ = 0;
	child->MarkDirty();
	if (scene_)
		scene_->NodeRemoved(child);
	children_.erase(iter);
}

void GameNode::ResetScene()
{
	SetID(0);
	SetScene(0);
}

void GameNode::MarkReplicationDirty()
{

}

void GameNode::AddComponent(class Component* component, unsigned id, CreateMode mode)
{
	if (!component)
		return;
	components_.push_back(SPComponent (component));
	component->SetNode(this);

	// If zero ID specified, or the ID is already taken, let the scene assign
	if (scene_)
	{
		if (!id || scene_->GetComponent(id))
			id = scene_->GetFreeComponentID(mode);
		component->SetID(id);
		scene_->ComponentAdded(component);
	}
	else
		component->SetID(id);

	component->OnMarkedDirty(this);

}

Component* GameNode::GetComponent(StringHash type, bool recursive /*= false*/) const
{
	for (VEC_SP_COMPONENT::const_iterator i = components_.begin(); i != components_.end(); ++i)
	{
		if ((*i)->GetType() == type)
			return (*i).get();
	}

	if (recursive)
	{
		for (VEC_SPNODE::const_iterator i = children_.begin(); i != children_.end(); ++i)
		{
			Component* component = (*i)->GetComponent(type, true);
			if (component)
				return component;
		}
	}

	return 0;
}

void GameNode::RemoveAllComponents()
{
	RemoveComponents(true, true);
}

void GameNode::RemoveComponents(bool removeReplicated, bool removeLocal)
{
	unsigned numRemoved = 0;

	for (unsigned i = components_.size() - 1; i < components_.size(); --i)
	{
		bool remove = false;
		Component* component = components_[i].get();

		if (component->GetID() < FIRST_LOCAL_ID && removeReplicated)
			remove = true;
		else if (component->GetID() >= FIRST_LOCAL_ID && removeLocal)
			remove = true;

		if (remove)
		{
			RemoveComponent(components_.begin() + i);
			++numRemoved;
		}
	}

	// Mark node dirty in all replication states
	if (numRemoved)
		MarkReplicationDirty();
}

void GameNode::RemoveComponent(Component* component)
{
	if (!component)
		return;
	for (VEC_SP_COMPONENT::iterator i = components_.begin(); i != components_.end(); ++i)
	{
		if ((*i).get() == component)
		{
			RemoveComponent(i);

			// Mark node dirty in all replication states
			MarkReplicationDirty();
			return;
		}
	}
}

void GameNode::RemoveComponent(StringHash type)
{
	unsigned numRemoved = 0;

	for (unsigned i = components_.size() - 1; i < components_.size(); --i)
	{
		if (components_[i]->GetType() == type)
		{
			RemoveComponent(components_.begin() + i);
			++numRemoved;
		}
	}

	// Mark node dirty in all replication states
	if (numRemoved)
		MarkReplicationDirty();
}

void GameNode::RemoveComponent(VEC_SP_COMPONENT::iterator iter)
{
	Component* child = (*iter).get();

	if (scene_)
		scene_->ComponentRemoved(child);
	child->SetNode(0);
	components_.erase(iter);

}

void GameNode::AddListener(Component* component)
{
	if (!component)
		return;
	VEC_WP_COMPONENT::iterator i = listeners_.begin();
	// Check for not adding twice
	for (; i != listeners_.end(); ++i)
	{
		if ((*i)== component)
			return;
	}
	listeners_.push_back(component);
	// If the node is currently dirty, notify immediately
	if (dirty_)
		component->OnMarkedDirty(this);
}

void GameNode::Pitch(float angle, TransformSpace space /*= TS_LOCAL*/)
{

	Rotate(Quaternion::CreateFromAngleAxis(angle, Vector3::Right), space);

}

void GameNode::Yaw(float angle, TransformSpace space /*= TS_LOCAL*/)
{
	Rotate(Quaternion::CreateFromAngleAxis(angle, Vector3::Up), space);
}

void GameNode::Roll(float angle, TransformSpace space /*= TS_LOCAL*/)
{
	Rotate(Quaternion::CreateFromAngleAxis(angle, Vector3::Backward), space);
}

bool GameNode::LookAt(const Vector3& target, const Vector3& up /*= Vector3::UP*/, TransformSpace space /*= TS_WORLD*/)
{
	return true;
}

void GameNode::Rotate(const Quaternion& delta, TransformSpace space /*= TS_LOCAL*/)
{

	switch (space)
	{
	case TS_LOCAL:
		rotation_ = (delta * rotation_);
		break;

	case TS_PARENT:
		rotation_ = (rotation_ * delta);
		break;

	case TS_WORLD:
		if (parent_ == scene_ || !parent_)
			rotation_ = (rotation_ * delta);
		else
		{
			Quaternion worldRotation = GetWorldRotation();
			Quaternion worldRotationInv;
			worldRotation.Inverse(worldRotationInv);
			rotation_ = worldRotation * delta * worldRotationInv * rotation_;
		}
		break;
	}

	MarkDirty();
}

void GameNode::Translate(const Vector3& delta, TransformSpace space /*= TS_LOCAL*/)
{
	switch (space)
	{
	case TS_LOCAL:
		// Note: local space translation disregards local scale for scale-independent movement speed
	{	
		Vector3 vAdd = Vector3::Transform(delta, rotation_);
		position_ += vAdd;
		break;
	}
	case TS_PARENT:
		position_ += delta;
		break;

	case TS_WORLD:
		Matrix worldInv;
		parent_->GetWorldTransform().Invert(worldInv);
		if (parent_ == scene_ || !parent_)
		{
			position_ = delta + position_;
		}
		else
		{
			position_ += Vector3::Transform(delta,worldInv);
		}
		break;
	}

	MarkDirty();
}

void GameNode::SetDirection(const Vector3& direction)
{
	Quaternion ret = Quaternion::CreateFromRotationTo(Vector3::Backward, direction);

	SetRotation(ret);
}


