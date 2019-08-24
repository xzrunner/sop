#include "everything/node/ForeachPrimEnd.h"
#include "everything/node/ForeachPrimBegin.h"
#include "everything/Evaluator.h"
#include "everything/Geometry.h"
#include "everything/GeoAttrHelper.h"

#include <queue>

#include <assert.h>

namespace evt
{
namespace node
{

void ForeachPrimEnd::Execute(TreeContext& ctx)
{
    std::vector<NodePtr> nodes;
    auto begin = FindForeachBegin(nodes);
    if (!begin) {
        return;
    }

    auto prev_geo = begin->GetInputGeo(0);
    if (!prev_geo) {
        return;
    }

    m_geo = std::make_shared<Geometry>(GeoShapeType::Faces);

    // closure
    Evaluator eval;
    for (auto& n : nodes) {
        eval.AddNode(n);
    }

    // foreach prim
    auto& prev_attr = prev_geo->GetAttr();
    auto& prev_prims = prev_attr.GetPrimtives();
    for (size_t i = 0, n = prev_prims.size(); i < n; ++i)
    {
        auto b_geo = begin->GetGeometry();
        auto& b_attr = b_geo->GetAttr();
        GeoAttrHelper::GenAttrFromPrim(b_attr, prev_attr, i);
        b_geo->UpdateByAttr();

        eval.MakeDirty();
        eval.Update();

        auto e_prev_geo = GetInputGeo(0);
        if (e_prev_geo) {
            m_geo->GetGroup().Combine(e_prev_geo->GetGroup(), m_geo->GetAttr().GetPrimtives().size());
            m_geo->GetAttr().Combine(e_prev_geo->GetAttr());
        }
    }
}

NodePtr ForeachPrimEnd::FindForeachBegin(std::vector<NodePtr>& nodes) const
{
    NodePtr begin = nullptr;

    std::queue<NodePtr> buf;
    assert(m_imports.size() == 1);
    if (m_imports[0].conns.empty()) {
        return begin;
    }
    assert(m_imports[0].conns.size() == 1);
    auto prev_node = m_imports[0].conns[0].node.lock();
    assert(prev_node);
    buf.push(prev_node);
    while (!buf.empty())
    {
        auto n = buf.front(); buf.pop();
        assert(n);
        nodes.push_back(n);

        if (n->get_type() == rttr::type::get<node::ForeachPrimBegin>()) {
            begin = n;
            continue;
        }

        for (auto& in : n->GetImports()) {
            for (auto& conn : in.conns) {
                auto prev = conn.node.lock();
                buf.push(prev);
            }
        }
    }

    return begin;
}

}
}