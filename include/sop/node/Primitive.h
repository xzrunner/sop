#pragma once

#include "sop/Node.h"
#include "sop/GeoAttribute.h"

#include <SM_Vector.h>
#include <SM_Matrix.h>

namespace sop
{
namespace node
{

class Primitive : public Node
{
public:
    Primitive()
    {
        m_imports = {
            {{ NodeVarType::Any, "in" }}
        };
        m_exports = {
            {{ NodeVarType::Any, "out" }},
        };
    }

    virtual void Execute(Evaluator& eval) override;

    void SetGroupName(const std::string& name);

    void SetTranslate(const sm::vec3& t);
    void SetRotate(const sm::vec3& r);
    void SetScale(const sm::vec3& s);
    void SetShear(const sm::vec3& s);

private:
    void UpdatePrim(GeoAttribute::Primitive& prim, const sm::mat4& mat);

private:
    std::string m_group_name;

    sm::vec3 m_translate = sm::vec3(0, 0, 0);
    sm::vec3 m_rotate    = sm::vec3(0, 0, 0);
    sm::vec3 m_scale     = sm::vec3(1, 1, 1);
    sm::vec3 m_shear     = sm::vec3(0, 0, 0);

    RTTR_ENABLE(Node)

}; // Primitive

}
}