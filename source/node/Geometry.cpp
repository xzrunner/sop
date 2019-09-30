#include "sop/node/Geometry.h"
#include "sop/NodeHelper.h"

namespace sop
{
namespace node
{

void Geometry::Execute(Evaluator& eval)
{
    for (auto& c : m_children) {
        if (NodeHelper::IsRoot(*c)) {
            c->Execute(eval);
        }
    }
}

void Geometry::AddChild(const std::shared_ptr<Geometry>& parent,
                        const std::shared_ptr<Node>& child)
{
    if (!parent || !child) {
        return;
    }
    if (parent->m_children.find(child) != parent->m_children.end()) {
        return;
    }

    parent->m_children.insert(child);
    child->SetParent(parent);
}

std::shared_ptr<Node>
Geometry::QueryChild(const std::string& name) const
{
    for (auto& c : m_children) {
        if (c->GetName() == name) {
            return c;
        }
    }
    return nullptr;
}

}
}