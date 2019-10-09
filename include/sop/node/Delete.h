#pragma once

#include "sop/Node.h"

namespace sop
{
namespace node
{

class Delete : public Node
{
public:
    enum class EntityType
    {
        Points,
        Edges,
        Faces,
    };

public:
    Delete()
    {
        m_imports = {
            {{ NodeVarType::Any, "in" }}
        };
        m_exports = {
            {{ NodeVarType::Any, "out" }},
        };
    }

    virtual void Execute(Evaluator& eval) override;

    void SetDelNonSelected(bool del_non_selected);

    void SetEntityType(EntityType type);

    void SetFilterExpr(const std::string& expr);

private:
    bool m_delete_non_selected = false;

    EntityType m_entity_type = EntityType::Points;

    std::string m_filter_expr;

    RTTR_ENABLE(Node)

}; // Delete

}
}