#include "everything/node/Blast.h"
#include "everything/TreeContext.h"
#include "everything/Geometry.h"

#include <model/BrushBuilder.h>
#include <model/BrushModel.h>
#include <node0/SceneNode.h>
#include <polymesh3/Geometry.h>

namespace evt
{
namespace node
{

void Blast::Execute(TreeContext& ctx)
{
    m_geo.reset();

    auto prev_geo = GetInputGeo(0);
    if (!prev_geo) {
        return;
    }

    m_geo = std::make_shared<Geometry>(*prev_geo);

    auto group = m_geo->QueryGroup(m_group_name);
    if (!group) {
        return;
    }

    auto brush_model = m_geo->GetBrushModel();
    assert(brush_model);
    auto& brushes = brush_model->GetBrushes();
    assert(brushes.size() == 1);
    auto& brush = brushes[0];
    assert(brush.impl);

    switch (m_group_type)
    {
    case GroupType::Primitives:
    {
        auto faces = brush.impl->Faces();
        std::vector<bool> del_flags;
        if (m_delete_non_selected)
        {
            del_flags.resize(faces.size(), true);
            for (auto& f : group->items) {
                del_flags[f] = false;
            }
        }
        else
        {
            del_flags.resize(faces.size(), false);
            for (auto& f : group->items) {
                del_flags[f] = true;
            }
        }
        int idx = 0;
        for (auto itr = faces.begin(); itr != faces.end(); ++idx)
        {
            if (del_flags[idx]) {
                itr = faces.erase(itr);
            }
            else {
                ++itr;
            }
        }

        if (faces.size() != brush.impl->Faces().size())
        {
            group->items.clear();
            if (m_delete_non_selected) {
                for (int i = 0, n = faces.size(); i < n; ++i) {
                    group->items.push_back(i);
                }
            }

            brush.impl->SetFaces(faces);

            m_geo->UpdateByBrush(*brush_model);
        }

        break;
    }
    }
}

void Blast::SetGroupType(GroupType type)
{
    if (m_group_type == type) {
        return;
    }

    m_group_type = type;

    SetDirty(true);
}

void Blast::SetDeleteNonSelected(bool del_non_selected)
{
    if (m_delete_non_selected == del_non_selected) {
        return;
    }

    m_delete_non_selected = del_non_selected;

    SetDirty(true);
}

}
}