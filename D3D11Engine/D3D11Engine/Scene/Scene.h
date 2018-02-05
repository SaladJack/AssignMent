#pragma once

/// Root scene node, represents the whole scene.

#include "CommonDefs.h"
#include "GameNode.h"



typedef HashMap<unsigned, GameNode*> HashMap_NODE;
typedef HashMap<unsigned, GameNode*>::const_iterator  HashMap_NODE_ITERATOR;


typedef HashMap<unsigned, Component*> HashMap_Component;
typedef HashMap<unsigned, Component*>::const_iterator  HashMap_Component_Iter;


class Scene : public GameNode
{
public:
	using GameNode::GetComponent;
public:
	Scene();
	~Scene();
public:

	/// Get free node ID, either non-local or local.
	unsigned GetFreeNodeID(CreateMode mode);
	/// Get free component ID, either non-local or local.
	unsigned GetFreeComponentID(CreateMode mode);

	/// Node added. Assign scene pointer and add to ID map.
	void NodeAdded(GameNode* node);
	/// Node removed. Remove from ID map.
	void NodeRemoved(GameNode* node);
	/// Return node from the whole scene by ID, or null if not found.
	GameNode* GetNode(unsigned id) const;

	/// Component added. Add to ID map.
	void ComponentAdded(Component* component);
	void ComponentRemoved(Component* component);
	/// Return component from the whole scene by ID, or null if not found.
	Component* GetComponent(unsigned id) const;

	void MarkReplicationDirty(GameNode* node);

private:
	/// Replicated scene nodes by ID.
	HashMap_NODE replicatedNodes_;
	/// Local scene nodes by ID.
	HashMap_NODE localNodes_;

	/// Replicated components by ID.
	HashMap_Component replicatedComponents_;
	/// Local components by ID.
	HashMap_Component localComponents_;

	/// Next free non-local node ID.
	unsigned replicatedNodeID_;
	/// Next free local node ID.
	unsigned localNodeID_;

	/// Next free non-local component ID.
	unsigned replicatedComponentID_;
	/// Next free local component ID.
	unsigned localComponentID_;
};

