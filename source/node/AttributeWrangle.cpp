#include "everything/node/AttributeWrangle.h"
#include "everything/NodeHelper.h"
#include "everything/GeometryImpl.h"
#include "everything/EvalContext.h"
#include "everything/Evaluator.h"

namespace evt
{
namespace node
{

void AttributeWrangle::Execute(Evaluator& eval)
{
    m_geo_impl.reset();

    auto prev_geo = NodeHelper::GetInputGeo(*this, 0);
    if (!prev_geo) {
        return;
    }

    m_geo_impl = std::make_shared<GeometryImpl>(*prev_geo);
 
    EvalContext eval_ctx(eval, *this);
    auto& pts = m_geo_impl->GetAttr().GetPoints();
    for (size_t i = 0, n = pts.size(); i < n; ++i)
    {
        eval_ctx.point_idx = i;
        eval.RunStatement("{" + m_vex_expr + "}", eval_ctx);
    }
}

void AttributeWrangle::SetVexExpr(const std::string& expr)
{
    if (m_vex_expr == expr) {
        return;
    }

    m_vex_expr = expr;

    SetDirty(true);
}

}
}