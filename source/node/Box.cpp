#include "everything/node/Box.h"

#include <model/BrushBuilder.h>
#include <model/BrushModel.h>
#include <ns/NodeFactory.h>
#include <node0/SceneNode.h>
#include <node0/CompMaterial.h>
#include <node3/CompModel.h>
#include <node3/CompModelInst.h>
#include <node3/CompTransform.h>
#include <node3/CompAABB.h>
#include <painting3/MaterialMgr.h>

namespace evt
{
namespace node
{

void Box::Execute()
{
    if (!m_node)
    {
        m_node = ns::NodeFactory::Create3D();
        m_node->AddSharedComp<n3::CompModel>();
        m_node->AddUniqueComp<n3::CompModelInst>();

        UpdateModel();
    }

    // CompTransform
    auto& ctrans = m_node->GetUniqueComp<n3::CompTransform>();

    auto& cube = m_box.GetCube();

    ctrans.SetPosition(cube.Center());

    sm::vec3 scale;
    scale.x = cube.xmax - cube.xmin;
    scale.y = cube.ymax - cube.ymin;
    scale.z = cube.zmax - cube.zmin;
    ctrans.SetScale(scale);

    // CompMaterial
    auto& cmaterial = m_node->AddUniqueComp<n0::CompMaterial>();
    auto mat = std::make_unique<pt0::Material>();
    typedef pt3::MaterialMgr::PhongUniforms UNIFORMS;
    mat->AddVar(UNIFORMS::ambient.name,     pt0::RenderVariant(sm::vec3(0.04f, 0.04f, 0.04f)));
    mat->AddVar(UNIFORMS::diffuse.name,     pt0::RenderVariant(sm::vec3(1, 1, 1)));
    mat->AddVar(UNIFORMS::specular.name,    pt0::RenderVariant(sm::vec3(1, 1, 1)));
    mat->AddVar(UNIFORMS::shininess.name,   pt0::RenderVariant(50.0f));
    cmaterial.SetMaterial(mat);
}

void Box::Traverse(std::function<bool(const n0::SceneNodePtr&)> func) const
{
    func(m_node);
}

void Box::SetSize(const sm::vec3& size)
{
    if (m_size == size) {
        return;
    }

    m_size = size;
    UpdateModel();
}

void Box::SetCenter(const sm::vec3& center)
{
    if (m_center == center) {
        return;
    }

    m_center = center;
    UpdateModel();
}

void Box::SetScale(const sm::vec3& scale)
{
    if (m_scale == scale) {
        return;
    }

    m_scale = scale;
    UpdateModel();
}

void Box::UpdateModel()
{
    if (!m_node) {
        return;
    }

    auto brush_model = BuildBrush();
    std::shared_ptr<model::Model> model =
        model::BrushBuilder::PolymeshFromBrush(*brush_model);

    auto& cmodel = m_node->GetSharedComp<n3::CompModel>();
    cmodel.SetModel(model);

    auto& cmodel_inst = m_node->GetUniqueComp<n3::CompModelInst>();
    cmodel_inst.SetModel(model, 0);
}

std::unique_ptr<model::BrushModel>
Box::BuildBrush() const
{
    model::BrushModel::BrushData brush;

    brush.desc.mesh_begin = 0;
    brush.desc.mesh_end = 1;
    const int face_num = 6;
    brush.desc.meshes.push_back({ 0, 0, 0, face_num });

    auto& faces = brush.impl.faces;
    faces.reserve(face_num);

    const sm::vec3 s = m_scale / model::BrushBuilder::VERTEX_SCALE;
    const sm::vec3 h_sz = m_size * 0.5f;
    const sm::vec3 c = m_center / model::BrushBuilder::VERTEX_SCALE;

    const float xmin = -h_sz.x * s.x + c.x;
    const float xmax =  h_sz.x * s.x + c.x;
    const float ymin = -h_sz.y * s.y + c.y;
    const float ymax =  h_sz.y * s.y + c.y;
    const float zmin = -h_sz.z * s.z + c.z;
    const float zmax =  h_sz.z * s.z + c.z;

    const sm::vec3 top_left_front (xmin, ymax, zmin);
    const sm::vec3 top_left_back  (xmin, ymax, zmax);
    const sm::vec3 top_right_front(xmax, ymax, zmin);
    const sm::vec3 top_right_back (xmax, ymax, zmax);
    const sm::vec3 btm_left_front (xmin, ymin, zmin);
    const sm::vec3 btm_left_back  (xmin, ymin, zmax);
    const sm::vec3 btm_right_front(xmax, ymin, zmin);
    const sm::vec3 btm_right_back (xmax, ymin, zmax);

    auto top = std::make_shared<pm3::BrushFace>();
    top->plane = sm::Plane(top_left_front, top_left_back, top_right_back);
    faces.push_back(top);

    auto bottom = std::make_shared<pm3::BrushFace>();
    bottom->plane = sm::Plane(btm_left_front, btm_right_front, btm_right_back);
    faces.push_back(bottom);

    auto left = std::make_shared<pm3::BrushFace>();
    left->plane = sm::Plane(top_left_back, top_left_front, btm_left_front);
    faces.push_back(left);

    auto right = std::make_shared<pm3::BrushFace>();
    right->plane = sm::Plane(top_right_front, top_right_back, btm_right_back);
    faces.push_back(right);

    auto front = std::make_shared<pm3::BrushFace>();
    front->plane = sm::Plane(top_left_front, top_right_front, btm_right_front);
    faces.push_back(front);

    auto back = std::make_shared<pm3::BrushFace>();
    back->plane = sm::Plane(top_right_back, top_left_back, btm_left_back);
    faces.push_back(back);

    brush.impl.BuildVertices();
    brush.impl.BuildGeometry();

    auto model_model = std::make_unique<model::BrushModel>();
    std::vector<model::BrushModel::BrushData> brushes;
    brushes.push_back(brush);
    model_model->SetBrushes(brushes);

    return model_model;
}

}
}