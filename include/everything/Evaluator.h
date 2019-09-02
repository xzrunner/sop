#pragma once

#include "everything/typedef.h"
#include "everything/Node.h"

#include <set>
#include <vector>

namespace evt
{

struct EvalContext;

class Evaluator
{
public:
    // update nodes
    void AddNode(const NodePtr& node);
    void RemoveNode(const NodePtr& node);
    void ClearAllNodes();

    // update node prop
    void PropChanged(const NodePtr& node);

    // update node conn
    void Connect(const Node::PortAddr& from, const Node::PortAddr& to);
    void Disconnect(const Node::PortAddr& from, const Node::PortAddr& to);
    void RebuildConnections(const std::vector<std::pair<Node::PortAddr, Node::PortAddr>>& conns);

    void Update();

    void MakeDirty();

    Variable CalcExpr(const std::string& expr, const EvalContext& ctx) const;

    NodePtr QueryNode(const std::string& name) const;

private:
    void UpdateProps();
    void UpdateNodes();

    void TopologicalSorting();

    static void SetTreeDirty(const NodePtr& root);

    static bool HasNodeConns(const NodePtr& node);

private:
    std::map<std::string, NodePtr> m_nodes_map;

    std::vector<NodePtr> m_nodes_sorted;

    bool m_dirty = false;

    size_t m_next_id = 0;

}; // Evaluator

}
