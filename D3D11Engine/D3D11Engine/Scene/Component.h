#pragma once
#include "Object.h"
#include "StringHash.h"



class Component
{
	OBJECT(Component);
	BASEOBJECT(Component);
	friend class GameNode;
	friend class Scene;
	static StringHash GetTypeStaticEx() { static const StringHash typeStaticex("Component"); return typeStaticex; } \
public:
	Component();
	virtual ~Component();
	/// Handle enabled/disabled state change.
	virtual void OnSetEnabled() { }
	/// Remove from the scene node. If no other shared pointer references exist, causes immediate deletion.
	void Remove();

	/// Set ID. Called by Scene.
	void SetID(unsigned id){ id_ = id; }
	// Set scene node. Called by Node when creating the component.
	void SetNode(GameNode* node);
	unsigned GetID() const { return id_; }
	/// Return scene node.
	GameNode* GetNode() const { return node_; }

	/// Return the scene the node belongs to.
	Scene* GetScene() const;
	/// Return whether is enabled.
	bool IsEnabled() const { return enabled_; }

	template <class T> T* GetComponent() const;
	/// Return component in the same scene node by type. If there are several, returns the first.
	Component* GetComponent(StringHash type) const;

	/// Set enabled/disabled state.
	void SetEnabled(bool enable);

protected:
	/// Handle scene node being assigned at creation.
	virtual void OnNodeSet(GameNode* node);
	virtual void OnSceneSet(Scene* scene);

	/// Handle scene node transform dirtied.
	virtual void OnMarkedDirty(GameNode* node);
protected:
	/// Scene node.
	GameNode* node_;
	/// Unique ID within the scene.
	unsigned id_;
	/// Enabled flag.
	bool enabled_;


};

template <class T>
T* Component::GetComponent() const
{
	return static_cast<T*>(GetComponent(T::GetTypeStatic())); 
}

