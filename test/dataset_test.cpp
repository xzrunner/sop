#include "utility.h"

#include <everything/TreeContext.h>
#include <everything/Evaluator.h>

#include <everything/node/Box.h>
#include <everything/node/Geometry.h>

#include <catch/catch.hpp>

TEST_CASE("node prop")
{
    test::init();

    evt::Evaluator eval;

    auto box = std::make_shared<evt::node::Box>();
    eval.AddNode(box);

    eval.Update();

    auto& props = const_cast<evt::NodePropsMgr&>(box->GetProps());

    REQUIRE(props.Size() == evt::node::Box::MAX_BUILD_IN_PROP);

    props.Add("prop0", evt::Variable(1.1f));
    props.Add("prop1", evt::Variable(2));
    props.Add("prop2", evt::Variable(sm::vec3(4, 5, 6)));
    props.Add("prop3", evt::Variable(true));
    props.Add("prop4", evt::Variable("zz"));

    REQUIRE(props.Size() == evt::node::Box::MAX_BUILD_IN_PROP + 5);

    test::check_prop(box, "prop0", evt::Variable(1.1f));
    test::check_prop(box, "prop1", evt::Variable(2));
    test::check_prop(box, "prop2", evt::Variable(sm::vec3(4, 5, 6)));
    test::check_prop(box, "prop3", evt::Variable(true));
    test::check_prop(box, "prop4", evt::Variable("zz"));

    props.Remove("prop2");
    test::check_prop(box, "prop2", evt::Variable());

    REQUIRE(props.Size() == evt::node::Box::MAX_BUILD_IN_PROP + 4);

    props.Clear();
    test::check_prop(box, "prop3", evt::Variable());

    REQUIRE(props.Size() == evt::node::Box::MAX_BUILD_IN_PROP);
}

TEST_CASE("parent node prop")
{
    test::init();

    evt::Evaluator eval;

    auto geo = std::make_shared<evt::node::Geometry>();
    eval.AddNode(geo);

    auto box = std::make_shared<evt::node::Box>();
    eval.AddNode(box);

    evt::node::Geometry::AddChild(geo, box);

    auto& geo_props = const_cast<evt::NodePropsMgr&>(geo->GetProps());
    geo_props.Add("BoxLength", evt::Variable(8));
    geo_props.Add("BoxWidth",  evt::Variable(5));
    geo_props.Add("BoxHeight", evt::Variable(3));

    auto& box_props = const_cast<evt::NodePropsMgr&>(box->GetProps());
    box_props.SetExpr(evt::node::Box::SIZE_X, "ch(\"../BoxLength\")");
    box_props.SetExpr(evt::node::Box::SIZE_Y, "ch(\"../BoxHeight\")");
    box_props.SetExpr(evt::node::Box::SIZE_Z, "ch(\"../BoxWidth\")");
    box_props.SetExpr(evt::node::Box::POS_Y,  "ch(\"sizey\")/2");

    eval.Update();

    test::check_aabb(box, sm::vec3(-4, 0, -2.5f), sm::vec3(4, 3, 2.5f));
}