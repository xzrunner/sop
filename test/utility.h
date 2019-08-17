#pragma once

#include <everything/typedef.h>

#include <SM_Vector.h>

namespace test
{

void init();

void check_aabb(const evt::NodePtr& node, const sm::vec3& min, const sm::vec3& max);
void check_pos(const evt::NodePtr& node, size_t idx, const sm::vec3& pos);

void check_points_num(const evt::NodePtr& node, size_t num);
void check_edges_num(const evt::NodePtr& node, size_t num);
void check_faces_num(const evt::NodePtr& node, size_t num);

}