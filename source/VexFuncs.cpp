#include "sop/VexFuncs.h"
#include "sop/EvalContext.h"
#include "sop/GeometryImpl.h"
#include "sop/Evaluator.h"
#include "sop/ParmList.h"
#include "sop/node/Subnetwork.h"

#include <dag/Variable.h>
#include <cslang/EvalAST.h>
#include <cslang/StringPool.h>
#include <cpputil/StringHelper.h>

namespace
{

cslang::Variant ToVexcVar(const sop::EvalContext& ctx, const dag::Variable& var, int component = -1)
{
    switch (var.type)
    {
    case dag::VarType::Invalid:
        return cslang::Variant();
    case dag::VarType::Bool:
        return cslang::Variant(var.b);
    case dag::VarType::Int:
        return cslang::Variant(var.i);
    case dag::VarType::Float:
        return cslang::Variant(var.f);
    case dag::VarType::Float3:
    {
        if (component != -1)
        {
            assert(component >= 0 && component < 3);
            auto v3 = static_cast<const sm::vec3*>(var.p);
            return cslang::Variant(v3->xyz[component]);
        }
        else
        {
            auto v3 = static_cast<const sm::vec3*>(var.p);
            auto new_v3 = ctx.var_buf.Clone(*v3);
            return cslang::Variant(cslang::VarType::Float3, (void*)(new_v3->xyz));
        }
    }
    case dag::VarType::Double:
        return cslang::Variant(var.d);
    //case dag::VarType::String:
    //{
    //    auto str = static_cast<const char*>(v.p);
    //    auto buf = cslang::StringPool::InsertAndQuery(str, strlen(str));
    //    return cslang::Variant(cslang::VarType::String, buf);
    //}
    default:
        assert(0);
        return cslang::Variant();
    }
}

// ch* funcs
cslang::Variant eval_channel(const std::vector<cslang::Variant>& params, const void* ud)
{
    if (params.empty()) {
        return cslang::Variant();
    }

    auto& p = params[0];
    if (p.type != cslang::VarType::String) {
        return cslang::Variant();
    }

    auto ctx = static_cast<const sop::EvalContext*>(ud);

    std::string path(cslang::StringPool::VoidToString(p.p));
    std::vector<std::string> tokens;
    cpputil::StringHelper::Split(path, "/", tokens);
    auto curr_node = const_cast<sop::Node*>(ctx->node);
    int curr_level = curr_node->GetLevel();
    const int begin_level = curr_level;
    for (size_t i = 0, n = tokens.size(); i < n; ++i)
    {
        if (!curr_node && curr_level == begin_level - 1) {
            curr_node = ctx->eval->QueryNodeByName(tokens[i]).get();
            if (curr_node) {
                continue;
            }
        }

        if (!curr_node) {
            break;
        }

        auto& t = tokens[i];
        if (t == "..") {
            curr_node = curr_node->GetParent().get();
            --curr_level;
            continue;
        }

        // query child
        assert(curr_node);
        if (curr_node->get_type() == rttr::type::get<sop::node::Subnetwork>())
        {
            auto child = static_cast<const sop::node::Subnetwork*>(curr_node)->QueryChild(t);
            if (child) {
                curr_node = child.get();
                continue;
            }
        }

        // query prop
        assert(curr_node);
        auto var = curr_node->GetParms().Query(t);
        if (var.type == dag::VarType::Invalid) {
            var = curr_node->GetParms().Query(t);
        }
        return ToVexcVar(*ctx, var);
    }

    return cslang::Variant();
}

}

namespace sop
{

void SetupVexFuncs()
{
    // ATTRIBUTES AND INTRINSICS

    cslang::RegistBuildInFunc("point", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.size() < 4) {
            return cslang::Variant();
        }
        for (int i = 0; i < 4; ++i) {
            if (params[i].type == cslang::VarType::Invalid) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        Node* base_node = const_cast<Node*>(ctx->node);

        assert(params[0].type == cslang::VarType::String);
        std::string path(cslang::StringPool::VoidToString(params[0].p));
        auto surface_node = ctx->eval->QueryNodeByPath(base_node, path);
        if (!surface_node) {
            return cslang::Variant();
        }

        auto geo = surface_node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        assert(params[1].type == cslang::VarType::Int);
        int point_num = params[1].i;

        assert(params[2].type == cslang::VarType::String);
        std::string attrib_name = cslang::StringPool::VoidToString(params[2].p);

        assert(params[3].type == cslang::VarType::Int);
        int component = params[3].i;

        auto var = geo->GetAttr().QueryParm(GeoAttrClass::Point, attrib_name, point_num);
        return ToVexcVar(*ctx, var, component);
    });

    cslang::RegistBuildInFunc("prim", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.size() < 4) {
            return cslang::Variant();
        }
        for (int i = 0; i < 4; ++i) {
            if (params[i].type == cslang::VarType::Invalid) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        Node* base_node = const_cast<Node*>(ctx->node);

        assert(params[0].type == cslang::VarType::String);
        std::string path(cslang::StringPool::VoidToString(params[0].p));
        auto surface_node = ctx->eval->QueryNodeByPath(base_node, path);
        if (!surface_node) {
            return cslang::Variant();
        }

        auto geo = surface_node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        assert(params[1].type == cslang::VarType::Int);
        int prim_num = params[1].i;

        assert(params[2].type == cslang::VarType::String);
        std::string attrib_name = cslang::StringPool::VoidToString(params[2].p);

        assert(params[3].type == cslang::VarType::Int);
        int component = params[3].i;

        auto var = geo->GetAttr().QueryParm(GeoAttrClass::Primitive, attrib_name, prim_num);
        return ToVexcVar(*ctx, var, component);
    });

    cslang::RegistBuildInFunc("setattrib", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.size() < 6) {
            return cslang::Variant();
        }

        auto geohandle   = params[0].ToInt(ud);
        auto attr_class  = cslang::StringPool::VoidToString(params[1].p);
        auto attr_name   = cslang::StringPool::VoidToString(params[2].p);
        auto element_num = params[3].ToInt(ud);
        auto vertex_num  = params[4].ToInt(ud);
        auto value       = params[5].ToFloat(ud);

        assert(vertex_num == 0);

        assert(geohandle == 0);
        auto ctx = static_cast<const EvalContext*>(ud);
        if (!ctx->node) {
            return cslang::Variant();
        }
        auto geo = ctx->node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        GeoAttrClass cls;
        if (attr_class == "point") {
            cls = GeoAttrClass::Point;
        } else if (attr_class == "vertex") {
            cls = GeoAttrClass::Vertex;
        } else if (attr_class == "detail" || attr_class == "global") {
            cls = GeoAttrClass::Detail;
        } else {
            return cslang::Variant();
        }

        auto& attr = geo->GetAttr();
        auto list = attr.QueryParmList(cls, attr_name);
        if (list)
        {
            assert(list->GetType() == ParmType::Float);
            auto& items = std::static_pointer_cast<ParmFltList>(list)->GetAllItems();
            assert(element_num < static_cast<int>(items.size()));
            const_cast<std::vector<float>&>(items)[element_num] = value;
        }
        else
        {
            const size_t num = attr.GetSize(cls);
            std::vector<float> data(num, 0);
            assert(element_num < static_cast<int>(num));
            data[element_num] = value;
            attr.AddParmList(cls,
                std::make_shared<ParmFltList>(attr_name, ParmType::Float, data)
            );
        }

        return cslang::Variant();
    });

    // INTERPOLATION

    cslang::RegistBuildInFunc("fit", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.size() < 5) {
            return cslang::Variant();
        }
        for (int i = 0; i < 5; ++i) {
            if (params[i].type == cslang::VarType::Invalid) {
                return cslang::Variant();
            }
        }

        auto num     = params[0].ToFloat(ud);
        auto old_min = params[1].ToFloat(ud);
        auto old_max = params[2].ToFloat(ud);
        auto new_min = params[3].ToFloat(ud);
        auto new_max = params[4].ToFloat(ud);

        if (old_min == old_max) {
            return params[0];
        } else {
            return cslang::Variant(new_min + (num - old_min) / (old_max - old_min) * (new_max - new_min));
        }
    });

    // MEASURE

    cslang::RegistBuildInFunc("getbbox_center", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.empty()) {
            auto& p = params[0];
            if (p.type != cslang::VarType::Int || p.i != 0) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        if (!ctx->node) {
            return cslang::Variant();
        }
        auto geo = ctx->node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        auto& aabb = geo->GetAttr().GetAABB();
        auto v = ctx->var_buf.Clone(aabb.Center());
        return cslang::Variant(cslang::VarType::Float3, (void*)(v->xyz));
    });

    cslang::RegistBuildInFunc("getbbox_max", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.empty()) {
            auto& p = params[0];
            if (p.type != cslang::VarType::Int || p.i != 0) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        if (!ctx->node) {
            return cslang::Variant();
        }
        auto geo = ctx->node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        auto& aabb = geo->GetAttr().GetAABB();
        auto v = ctx->var_buf.Clone(sm::vec3(aabb.max));
        return cslang::Variant(cslang::VarType::Float3, (void*)(v->xyz));
    });

    cslang::RegistBuildInFunc("getbbox_min", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.empty()) {
            auto& p = params[0];
            if (p.type != cslang::VarType::Int || p.i != 0) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        if (!ctx->node) {
            return cslang::Variant();
        }
        auto geo = ctx->node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        auto& aabb = geo->GetAttr().GetAABB();
        auto v = ctx->var_buf.Clone(sm::vec3(aabb.min));
        return cslang::Variant(cslang::VarType::Float3, (void*)(v->xyz));
    });

    cslang::RegistBuildInFunc("getbbox_size", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant
    {
        if (params.empty()) {
            auto& p = params[0];
            if (p.type != cslang::VarType::Int || p.i != 0) {
                return cslang::Variant();
            }
        }

        auto ctx = static_cast<const EvalContext*>(ud);
        if (!ctx->node) {
            return cslang::Variant();
        }
        auto geo = ctx->node->GetGeometry();
        if (!geo) {
            return cslang::Variant();
        }

        auto& aabb = geo->GetAttr().GetAABB();
        auto v = ctx->var_buf.Clone(aabb.Size());
        return cslang::Variant(cslang::VarType::Float3, (void*)(v->xyz));
    });

    // NODES

    cslang::RegistBuildInFunc("ch", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant {
        return eval_channel(params, ud);
    });

    cslang::RegistBuildInFunc("chs", [](const std::vector<cslang::Variant>& params, const void* ud)->cslang::Variant {
        return eval_channel(params, ud);
    });

    // GETTER
    cslang::RegistGetter([](const char* sym, const void* ud)->cslang::Variant
    {
        assert(strlen(sym) > 0);

        if (strcmp(sym, "@P") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (!geo) {
                return cslang::Variant();
            }
            if (ctx->attr_type != GeoAttrClass::Point) {
                return cslang::Variant();
            }

            const auto& points = geo->GetAttr().GetPoints();
            if (ctx->attr_idx < 0 || ctx->attr_idx >= static_cast<int>(points.size())) {
                return cslang::Variant();
            }

            auto v = ctx->var_buf.Clone(points[ctx->attr_idx]->pos);
            return cslang::Variant(cslang::VarType::Float3, (void*)(v->xyz));
        }
        else if (strcmp(sym, "@N") == 0 || strcmp(sym, "v@N") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (!geo) {
                return cslang::Variant();
            }
            if (ctx->attr_type != GeoAttrClass::Point) {
                return cslang::Variant();
            }

            auto& attr = geo->GetAttr();
            const auto& points = attr.GetPoints();
            if (ctx->attr_idx < 0 || ctx->attr_idx >= static_cast<int>(points.size())) {
                return cslang::Variant();
            }

            auto norm_list = attr.QueryParmList(GeoAttrClass::Point, GEO_ATTR_NORM);
            if (!norm_list) {
                return cslang::Variant();
            }

            assert(norm_list->GetType() == ParmType::Vector);
            auto& norm_data = std::static_pointer_cast<ParmFlt3List>(norm_list)->GetAllItems();
            assert(ctx->attr_idx < static_cast<int>(norm_data.size()));
            auto& v3 = norm_data[ctx->attr_idx];
            auto new_v3 = ctx->var_buf.Clone(v3);
            return cslang::Variant(cslang::VarType::Float3, (void*)(new_v3->xyz));
        }
        else if (strcmp(sym, "@ptnum") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (ctx->attr_type != GeoAttrClass::Point) {
                return cslang::Variant();
            }
            return cslang::Variant(ctx->attr_idx);
        }
        // find from attrs
        else if (sym[0] == '@')
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (!geo) {
                return cslang::Variant();
            }

            auto attr_name = std::string(&sym[1]);
            auto var = geo->GetAttr().QueryParm(ctx->attr_type, attr_name, ctx->attr_idx);
            return ToVexcVar(*ctx, var);
        }
        else if (strcmp(sym, "$SIZEX") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (geo) {
                return cslang::Variant(geo->GetAttr().GetAABB().Width());
            } else {
                return cslang::Variant();
            }
        }
        else if (strcmp(sym, "$SIZEY") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (geo) {
                return cslang::Variant(geo->GetAttr().GetAABB().Height());
            } else {
                return cslang::Variant();
            }
        }
        else if (strcmp(sym, "$SIZEZ") == 0)
        {
            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return cslang::Variant();
            }
            auto geo = ctx->node->GetGeometry();
            if (geo) {
                return cslang::Variant(geo->GetAttr().GetAABB().Depth());
            } else {
                return cslang::Variant();
            }
        }
        else
        {
            assert(0);
        }

        return cslang::Variant();
    });

    // SETTER
    cslang::RegistSetter([](const char* sym, const char* member, cslang::Variant var, const void* ud)
    {
        auto set_pt_flt3 = [](sop::GeoAttr attr_type, cslang::Variant var, const void* ud, const char* member)
        {
            sm::vec3 v3;
            if (var.type != cslang::VarType::Float3) 
            {
                switch (var.type)
                {
                case cslang::VarType::Float:
                    v3.Set(var.f, var.f, var.f);
                    var = cslang::Variant(cslang::VarType::Float3, (void*)(v3.xyz));
                    break;
                case cslang::VarType::Double:
                {
                    const float f = static_cast<float>(var.d);
                    v3.Set(f, f, f);
                    var = cslang::Variant(cslang::VarType::Float3, (void*)(v3.xyz));
                }
                    break;
                default:
                    return;
                }
            }

            auto ctx = static_cast<const EvalContext*>(ud);
            if (!ctx->node) {
                return;
            }
            auto geo = ctx->node->GetGeometry();
            if (!geo) {
                return;
            }
            if (ctx->attr_type != GeoAttrClass::Point) {
                return;
            }

            auto& attr = geo->GetAttr();
            const auto& points = attr.GetPoints();
            if (ctx->attr_idx < 0 || ctx->attr_idx >= static_cast<int>(points.size())) {
                return;
            }

            auto val_r = static_cast<const sm::vec3*>(var.p);
            auto up_list = attr.QueryParmList(GeoAttrClass::Point, attr_type);
            if (up_list)
            {
                assert(up_list->GetType() == ParmType::Float3 
                    || up_list->GetType() == ParmType::Vector);
                auto& up_data = std::static_pointer_cast<ParmFlt3List>(up_list)->GetAllItems();
                assert(ctx->attr_idx < static_cast<int>(up_data.size()));
                if (member)
                {
                    if (strcmp(member, "x") == 0) {
                        const_cast<std::vector<sm::vec3>&>(up_data)[ctx->attr_idx].x = val_r->x;
                    } else if (strcmp(member, "y") == 0) {
                        const_cast<std::vector<sm::vec3>&>(up_data)[ctx->attr_idx].y = val_r->x;
                    } else if (strcmp(member, "z") == 0) {
                        const_cast<std::vector<sm::vec3>&>(up_data)[ctx->attr_idx].z = val_r->x;
                    } else {
                        assert(0);
                    }
                }
                else
                {
                    const_cast<std::vector<sm::vec3>&>(up_data)[ctx->attr_idx] = *val_r;
                }
            }
            else
            {
                std::vector<sm::vec3> data(points.size(), sm::vec3(0, 0, 0));
                data[ctx->attr_idx] = *val_r;
                attr.AddParmList(GeoAttrClass::Point,
                    std::make_shared<ParmFlt3List>(attr_type, data)
                );
            }
        };

        if (strcmp(sym, "@up") == 0 || strcmp(sym, "v@up") == 0)
        {
            set_pt_flt3(GEO_ATTR_UP, var, ud, member);
        }
        else if (strcmp(sym, "@Cd") == 0)
        {
            set_pt_flt3(GEO_ATTR_CD, var, ud, member);
        }
        else
        {
            assert(0);
        }
    });
}

}