#pragma once

#include "sop/Node.h"
#include "sop/typedef.h"

namespace sop
{
namespace node
{

class ObjectMerge : public Node
{
public:
    ObjectMerge()
    {
        m_exports = {
            {{ NodeVarType::Any, "out" }},
        };
    }

    virtual void Execute(Evaluator& eval) override;

    RTTR_ENABLE(Node)

#define PARM_FILEPATH "sop/node/ObjectMerge.parm.h"
#include <hdiop/node_parms_gen.h>
#undef PARM_FILEPATH

}; // ObjectMerge

}
}