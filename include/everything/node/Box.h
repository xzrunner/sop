#pragma once

#include "everything/Node.h"

#include <geoshape/Box.h>
#include <node0/typedef.h>

namespace n3 { class CompTransform; }
namespace model { class BrushModel; }

namespace evt
{
namespace node
{

class Box : public Node
{
public:
    Box()
    {
        m_imports = {
            {{ VariableType::Any, "in" }}
        };
        m_exports = {
            {{ VariableType::Any, "out" }},
        };
    }

    virtual void ExecuteSelf() override;

    void SetSize(const sm::vec3& size);
    void SetCenter(const sm::vec3& center);
    void SetScale(const sm::vec3& scale);

private:
    void UpdateModel();

    std::unique_ptr<model::BrushModel> BuildBrush() const;

private:
    gs::Box m_box;

    sm::vec3 m_size   = sm::vec3(1, 1, 1);
    sm::vec3 m_center = sm::vec3(0, 0, 0);
    sm::vec3 m_scale  = sm::vec3(1, 1, 1);

    RTTR_ENABLE(Node)

}; // Box

}
}