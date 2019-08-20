#include "everything/node/Add.h"
#include "everything/Geometry.h"

namespace evt
{
namespace node
{

void Add::Execute(TreeContext& ctx)
{
    m_geo.reset();

    std::vector<sm::vec3> vertices;

    auto prev_geo = GetInputGeo(0);
    if (prev_geo)
    {
        auto& prev_points = prev_geo->GetAttr().GetPoints();
        vertices.reserve(prev_points.size());
        for (auto& p : prev_points) {
            vertices.push_back(p->pos);
        }
    }

    if (vertices.empty() && m_points.empty()) {
        return;
    }
    std::copy(m_points.begin(), m_points.end(), std::back_inserter(vertices));

    m_geo = std::make_shared<Geometry>(GeoPolyline(vertices));
}

void Add::SetPoints(const std::vector<sm::vec3>& points)
{
    if (m_points == points) {
        return;
    }

    m_points = points;

    SetDirty(true);
}

}
}