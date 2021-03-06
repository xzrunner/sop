#include "sop/node/Boolean.h"
#include "sop/GeometryImpl.h"
#include "sop/NodeHelper.h"
#include "sop/GeoAttrRebuild.h"

#include <halfedge/Polyhedron.h>
#include <polymesh3/Polytope.h>
#include <model/BrushModel.h>

namespace sop
{
namespace node
{

void Boolean::Execute(const ur::Device& dev, Evaluator& eval)
{
    m_geo_impl.reset();

    auto geo_a = NodeHelper::GetInputGeo(*this, IDX_A);
    auto geo_b = NodeHelper::GetInputGeo(*this, IDX_B);
    if (!geo_a || !geo_b) {
        return;
    }
    auto brush_model_a = geo_a->GetBrushModel();
    auto brush_model_b = geo_b->GetBrushModel();
    if (!brush_model_a || !brush_model_b) {
        return;
    }

    m_geo_impl = std::make_shared<GeometryImpl>(dev, GeoAdaptor::Type::Brush);

    std::vector<model::BrushModel::Brush> brushes;

    auto& brushes_a = brush_model_a->GetBrushes();
    auto& brushes_b = brush_model_b->GetBrushes();
    for (auto& a : brushes_a)
    {
        for (auto& b : brushes_b)
        {
            auto topo_a = a.impl->GetTopoPoly();
            auto topo_b = b.impl->GetTopoPoly();
            if (!topo_a || !topo_b) {
                continue;
            }

            switch (m_operator)
            {
            case Operator::Union:
                break;
            case Operator::Intersect:
            {
                auto poly = topo_a->Intersect(*topo_b);
                if (poly && poly->GetLoops().Size() > 0)
                {
                    model::BrushModel::Brush brush;
                    brush.impl = std::make_shared<pm3::Polytope>(poly);
                    brushes.push_back(brush);
                }
            }
                break;
            case Operator::Subtract:
            {
                auto polys = topo_a->Subtract(*topo_b);
                for (auto& poly : polys) {
                    if (poly->GetLoops().Size() > 0)
                    {
                        model::BrushModel::Brush brush;
                        brush.impl = std::make_shared<pm3::Polytope>(poly);
                        brushes.push_back(brush);
                    }
                }
            }
                break;
            default:
                assert(0);
            }
        }
    }

    if (!brushes.empty())
    {
        GeoAttrRebuild attr_rebuild(*m_geo_impl);

        auto brush_model = std::make_unique<model::BrushModel>();
        brush_model->SetBrushes(brushes);
        m_geo_impl->UpdateByBrush(dev, *brush_model);
        m_geo_impl->StoreBrush(brush_model);
    }
}

}
}