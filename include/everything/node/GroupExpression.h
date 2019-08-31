#pragma once

#include "everything/Node.h"
#include "everything/Group.h"

namespace evt
{
namespace node
{

class GroupExpression : public Node
{
public:
    enum class MergeOP
    {
        Replace,
        Union,
        Intersect,
        Subtract,
    };

    struct Instance
    {
        std::string group_name;
        std::string expr_str;
        MergeOP     merge_op = MergeOP::Union;
    };

public:
    GroupExpression()
    {
        m_imports = {
            {{ NodeVarType::Any, "in0" }},
            {{ NodeVarType::Any, "in1" }},
            {{ NodeVarType::Any, "in2" }},
            {{ NodeVarType::Any, "in3" }}
        };
        m_exports = {
            {{ NodeVarType::Any, "out" }},
        };
    }

    virtual void Execute(Evaluator& eval, TreeContext& ctx) override;

    void SetType(GroupType type);

    void AddInstance(const Instance& inst);

private:
    void Select(std::vector<size_t>& items,
        const Instance& inst, Evaluator& eval) const;
    void Merge(MergeOP op, const std::vector<size_t>& src,
        std::vector<size_t>& dst) const;

private:
    GroupType m_type = GroupType::Primitives;

    std::vector<Instance> m_intsts;

    RTTR_ENABLE(Node)

}; // GroupExpression

}
}