#include "utility.h"

#include <everything/Node.h>
#include <everything/Geometry.h>

#include <unirender/gl/RenderContext.h>
#include <unirender/Blackboard.h>
#include <painting3/AABB.h>
#include <renderpipeline/RenderMgr.h>
#include <node0/SceneNode.h>
#include <node3/CompAABB.h>

#include <gl/glew.h>
#include <glfw3.h>
#include <catch/catch.hpp>

namespace
{

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

bool InitGL()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(100, 100, "rotate-crop", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	//// Initialize GLEW to setup the OpenGL Function pointers
	//if (glewInit() != GLEW_OK) {
	//	return -1;
	//}

	return true;
}

void InitRender()
{
    auto ur_rc = std::make_shared<ur::gl::RenderContext>(4096, [&](ur::RenderContext& ctx) {
        ctx.EnableFlushCB(false);
        rp::RenderMgr::Instance()->Flush();
        ctx.EnableFlushCB(true);
    });
    ur::Blackboard::Instance()->SetRenderContext(ur_rc);
}

}

namespace test
{

void init()
{
    static bool inited = false;
    if (!inited) {
        InitGL();
        InitRender();
        inited = true;
    }
}

void check_aabb(const evt::NodePtr& node, const sm::vec3& min, const sm::vec3& max)
{
    auto sn = node->GetGeometry()->GetNode();
    auto& caabb = sn->GetUniqueComp<n3::CompAABB>();
    auto& aabb = caabb.GetAABB();

    REQUIRE(aabb.Min()[0] == Approx(min.x));
    REQUIRE(aabb.Min()[1] == Approx(min.y));
    REQUIRE(aabb.Min()[2] == Approx(min.z));
    REQUIRE(aabb.Max()[0] == Approx(max.x));
    REQUIRE(aabb.Max()[1] == Approx(max.y));
    REQUIRE(aabb.Max()[2] == Approx(max.z));
}

void check_pos(const evt::NodePtr& node, size_t idx, const sm::vec3& pos)
{
    auto geo = node->GetGeometry();
    REQUIRE(geo != nullptr);

    size_t i = 0;
    sm::vec3 src;
    src.MakeInvalid();
    geo->TraversePoints([&](const sm::vec3& p, bool& dirty)->bool
    {
        dirty = false;
        if (i++ == idx) {
            src = p;
            return false;
        } else {
            return true;
        }
    });

    REQUIRE(src.x == Approx(pos.x));
    REQUIRE(src.y == Approx(pos.y));
    REQUIRE(src.z == Approx(pos.z));
}

void check_points_num(const evt::NodePtr& node, size_t num)
{
    auto geo = node->GetGeometry();
    REQUIRE(geo != nullptr);

    size_t n = 0;
    geo->TraversePoints([&](const sm::vec3& p, bool& dirty)->bool {
        ++n;
        return true;
    });
    REQUIRE(num == n);
}

void check_edges_num(const evt::NodePtr& node, size_t num)
{
    auto geo = node->GetGeometry();
    REQUIRE(geo != nullptr);

    size_t n = 0;
    geo->TraverseEdges([&](const sm::vec3& begin, const sm::vec3& end)->bool {
        ++n;
        return true;
    });
    REQUIRE(num == n);
}

void check_faces_num(const evt::NodePtr& node, size_t num)
{
    auto geo = node->GetGeometry();
    REQUIRE(geo != nullptr);

    size_t n = 0;
    geo->TraverseFaces([&](pm3::Polytope& poly, size_t face_idx, bool& dirty)->bool {
        ++n;
        return true;
    });
    REQUIRE(num == n);
}

}