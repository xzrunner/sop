#include "everything/Evaluator.h"
#include "everything/TreeContext.h"
#include "everything/EvalContext.h"

#include <vexc/EvalAST.h>
#include <vexc/Parser.h>

#include <stack>
#include <queue>

#include <assert.h>

namespace evt
{

void Evaluator::AddNode(const NodePtr& node)
{
    std::string name;
    do {
        name = "node" + std::to_string(m_next_id++);
    } while (m_nodes_map.find(name) != m_nodes_map.end());
    node->SetName(name);

    assert(m_nodes_map.size() == m_nodes_sorted.size());
    m_nodes_map.insert({ name, node });
    m_nodes_sorted.insert(m_nodes_sorted.begin(), node);

    m_dirty = true;
}

void Evaluator::RemoveNode(const NodePtr& node)
{
    if (m_nodes_map.empty()) {
        return;
    }

    auto itr = m_nodes_map.find(node->GetName());
    if (itr == m_nodes_map.end()) {
        return;
    }

    SetTreeDirty(node);

    assert(m_nodes_map.size() == m_nodes_sorted.size());
    m_nodes_map.erase(itr);
    for (auto itr = m_nodes_sorted.begin(); itr != m_nodes_sorted.end(); ++itr) {
        if (*itr == node) {
            m_nodes_sorted.erase(itr);
            break;
        }
    }

    m_dirty = true;
}

void Evaluator::ClearAllNodes()
{
    if (m_nodes_map.empty()) {
        return;
    }

    assert(m_nodes_map.size() == m_nodes_sorted.size());
    m_nodes_map.clear();
    m_nodes_sorted.clear();

    m_dirty = true;
}

void Evaluator::PropChanged(const NodePtr& node)
{
    SetTreeDirty(node);

    m_dirty = true;
}

void Evaluator::Connect(const Node::PortAddr& from, const Node::PortAddr& to)
{
    make_connecting(from, to);

    auto node = to.node.lock();
    assert(node);
    SetTreeDirty(node);

    m_dirty = true;
}

void Evaluator::Disconnect(const Node::PortAddr& from, const Node::PortAddr& to)
{
    disconnect(from, to);

    auto node = to.node.lock();
    assert(node);
    SetTreeDirty(node);

    m_dirty = true;
}

void Evaluator::RebuildConnections(const std::vector<std::pair<Node::PortAddr, Node::PortAddr>>& conns)
{
    // update dirty
    for (auto itr : m_nodes_map) {
        if (HasNodeConns(itr.second)) {
            SetTreeDirty(itr.second);
        }
    }

    // remove conns
    for (auto itr : m_nodes_map)
    {
        for (auto& in : itr.second->GetImports()) {
            const_cast<Node::Port&>(in).conns.clear();
        }
        for (auto& out : itr.second->GetExports()) {
            const_cast<Node::Port&>(out).conns.clear();
        }
    }

    // add conns
    for (auto& conn : conns)
    {
        auto t_node = conn.second.node.lock();
        assert(t_node);
        SetTreeDirty(t_node);
        make_connecting(conn.first, conn.second);
    }

    m_dirty = true;
}

void Evaluator::Update()
{
    if (!m_dirty) {
        return;
    }

    //// fixme: prepare geo for prop: $SIZEX
    //UpdateNodes();
    //UpdateProps();

    UpdateProps();
    UpdateNodes();

    m_dirty = false;
}

void Evaluator::MakeDirty()
{
    m_dirty = true;
    for (auto itr : m_nodes_map) {
        itr.second->SetDirty(true);
    }
}

Variable Evaluator::CalcExpr(const std::string& str, const EvalContext& ctx) const
{
    vexc::Parser parser(str.c_str());
    auto expr = vexc::ast::ExpressionParser::ParseExpression(parser);
    auto val = vexc::EvalExpression(expr, &ctx);
    switch (val.type)
    {
    case vexc::VarType::Bool:
        return Variable(val.b);
    case vexc::VarType::Int:
        return Variable(val.i);
    case vexc::VarType::Float:
        return Variable(val.f);
    case vexc::VarType::Float3:
    {
        auto f3 = static_cast<const float*>(val.p);
        return Variable(sm::vec3(f3[0], f3[1], f3[2]));
    }
    case vexc::VarType::Double:
        return Variable(static_cast<float>(val.d));
    case vexc::VarType::String:
        return Variable(vexc::StringPool::VoidToString(val.p));
    default:
        assert(0);
        return Variable();
    }
}

NodePtr Evaluator::QueryNode(const std::string& name) const
{
    auto itr = m_nodes_map.find(name);
    return itr == m_nodes_map.end() ? nullptr : itr->second;
}

void Evaluator::UpdateProps()
{
    // todo: topo sort props

    if (m_nodes_sorted.empty()) {
        return;
    }

    for (auto& node : m_nodes_sorted) {
        for (auto& prop : node->GetProps().GetProps()) {
            const_cast<NodeProp&>(prop).Update(*this, node);
        }
    }
}

void Evaluator::UpdateNodes()
{
    if (m_nodes_sorted.empty()) {
        return;
    }

    TopologicalSorting();

    std::map<NodePtr, int> map2index;
    for (int i = 0, n = m_nodes_sorted.size(); i < n; ++i) {
        map2index.insert({ m_nodes_sorted[i], i });
    }

    std::vector<TreeContext> ctxes(m_nodes_sorted.size());
    for (int i = 0, n = m_nodes_sorted.size(); i < n; ++i)
    {
        auto& node = m_nodes_sorted[i];

        auto& ctx = ctxes[i];
        for (auto& port : node->GetImports())
        {
            for (auto& conn : port.conns)
            {
                auto node = conn.node.lock();
                assert(node);
                auto itr = map2index.find(node);
                //assert(itr != map2index.end());
                if (itr != map2index.end()) {
                    ctx.Combine(ctxes[itr->second]);
                }
            }
        }

        if (node->IsDirty()) {
            node->Execute(*this, ctx);
            node->SetDirty(false);
        }

        node->UpdateContext(ctx);
    }
}

void Evaluator::TopologicalSorting()
{
    std::vector<NodePtr> nodes;
    nodes.reserve(m_nodes_map.size());
    for (auto itr : m_nodes_map) {
        nodes.push_back(itr.second);
    }

    // prepare
    std::vector<int> in_deg(m_nodes_map.size(), 0);
    std::vector<std::vector<int>> out_nodes(nodes.size());
    for (int i = 0, n = nodes.size(); i < n; ++i)
    {
        auto& node = nodes[i];
        for (auto& port : node->GetImports())
        {
            if (port.conns.empty()) {
                continue;
            }

            assert(port.conns.size() == 1);
            auto from = port.conns[0].node.lock();
            assert(from);
            for (int j = 0, m = nodes.size(); j < m; ++j) {
                if (from == nodes[j]) {
                    in_deg[i]++;
                    out_nodes[j].push_back(i);
                    break;
                }
            }
        }
    }

    // sort
    std::stack<int> st;
    m_nodes_sorted.clear();
    for (int i = 0, n = in_deg.size(); i < n; ++i) {
        if (in_deg[i] == 0) {
            st.push(i);
        }
    }
    while (!st.empty())
    {
        int v = st.top();
        st.pop();
        m_nodes_sorted.push_back(nodes[v]);
        for (auto& i : out_nodes[v]) {
            assert(in_deg[i] > 0);
            in_deg[i]--;
            if (in_deg[i] == 0) {
                st.push(i);
            }
        }
    }
}

void Evaluator::SetTreeDirty(const NodePtr& root)
{
    std::queue<const Node*> buf;
    buf.push(root.get());
    while (!buf.empty())
    {
        auto n = buf.front(); buf.pop();
        if (n->IsDirty()) {
            continue;
        }

        n->SetDirty(true);
        for (auto& port : n->GetExports()) {
            for (auto& conn : port.conns) {
                auto node = conn.node.lock();
                buf.push(node.get());
            }
        }
    }
}

bool Evaluator::HasNodeConns(const NodePtr& node)
{
    for (auto& i : node->GetImports()) {
        if (!i.conns.empty()) {
            return true;
        }
    }
    for (auto& o : node->GetExports()) {
        if (!o.conns.empty()) {
            return true;
        }
    }
    return false;
}

}