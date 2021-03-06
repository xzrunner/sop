#include "utility.h"

#include <sop/Evaluator.h>
#include <sop/PyLoader.h>
#include <sop/node/Subnetwork.h>

#include <catch/catch.hpp>

TEST_CASE("run file")
{
    test::init();

    sop::Evaluator eval;

    auto p0 = std::make_shared<sop::node::Subnetwork>();
    p0->SetName("obj");
    eval.AddNode(p0);

    auto p1 = std::make_shared<sop::node::Subnetwork>();
    p1->SetName("Cartoon_FarmHouse");
    sop::node::Subnetwork::AddChild(p0, p1);

    sop::PyLoader loader(eval);
    loader.RunFile("D:/OneDrive/work/sop/hou/house.py");
}

TEST_CASE("merge two box")
{
    test::init();

    sop::Evaluator eval;

    auto p0 = std::make_shared<sop::node::Subnetwork>();
    p0->SetName("obj");
    eval.AddNode(p0);

    auto p1 = std::make_shared<sop::node::Subnetwork>();
    p1->SetName("Cartoon_FarmHouse");
    sop::node::Subnetwork::AddChild(p0, p1);

    sop::PyLoader loader(eval);
    loader.RunFile("D:/OneDrive/work/sop/hou/merge_two_box.py");

    eval.Update();

    auto box2 = eval.QueryNodeByPath("/obj/Cartoon_FarmHouse/box2/box2");
    REQUIRE(box2);
    test::check_aabb(box2, { 1.5f, -1, -2 }, { 2.5f, 1, 2 });

    auto merge = eval.QueryNodeByPath("/obj/Cartoon_FarmHouse/box2/merge1");
    REQUIRE(merge);
    test::check_aabb(merge, { -0.5f, -1, -2 }, { 2.5f, 1, 2 });
}