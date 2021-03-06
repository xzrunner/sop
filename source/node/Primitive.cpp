#include "sop/node/Primitive.h"
#include "sop/node/Transform.h"
#include "sop/NodeHelper.h"
#include "sop/GeometryImpl.h"

namespace sop
{
namespace node
{

void Primitive::Execute(const ur::Device& dev, Evaluator& eval)
{
    m_geo_impl.reset();

    auto prev_geo = NodeHelper::GetInputGeo(*this, 0);
    if (!prev_geo) {
        return;
    }

    m_geo_impl = std::make_shared<GeometryImpl>(*prev_geo);

    std::shared_ptr<Group> group = nullptr;
    if (!m_group_name.empty())
    {
        group = m_geo_impl->GetGroup().Query(m_group_name);
        if (!group) {
            return;
        }
    }

    auto mat = Transform::CalcTransformMat(m_translate, m_rotate * SM_DEG_TO_RAD, m_scale, m_shear);
    auto& attr = m_geo_impl->GetAttr();
    if (group)
    {
        sm::vec3 c;
        switch (group->GetType())
        {
        case GroupType::Points:
        {
            auto c = m_geo_impl->GetAttr().GetAABB().Center();
            for (auto& i : group->GetItems()) {
                auto& p = attr.GetPoints()[i];
                p->pos = mat * (p->pos - c) + c;
            }
        }
            break;
        case GroupType::Primitives:
        {
            for (auto& i : group->GetItems()) {
                UpdatePrim(*attr.GetPrimtives()[i], mat);
            }
        }
            break;
        default:
            assert(0);
        }
    }
    else
    {
        for (auto& prim : m_geo_impl->GetAttr().GetPrimtives()) {
            UpdatePrim(*prim, mat);
        }
    }

    m_geo_impl->UpdateByAttr(dev);
}

void Primitive::UpdatePrim(GeoAttribute::Primitive& prim, const sm::mat4& mat)
{
    sm::vec3 c(0, 0, 0);
    assert(!prim.vertices.empty());
    for (auto& v : prim.vertices) {
        c += v->point->pos;
    }
    c /= static_cast<float>(prim.vertices.size());

    for (auto& v : prim.vertices) {
        v->point->pos = mat * (v->point->pos - c) + c;
    }
}

}
}