#pragma once

#include "everything/GeoShape.h"

#include <SM_Vector.h>
#include <node0/typedef.h>

#include <functional>

namespace model { class BrushModel; struct Model; }

namespace evt
{

class GroupMgr;
class GeoAttribute;
class GeoShape;

class GeoAdaptor
{
public:
    GeoAdaptor(const GeoShapeType& type);
    GeoAdaptor(const GeoShape& shape);
    GeoAdaptor(const GeoAdaptor& adaptor);
    GeoAdaptor& operator = (const GeoAdaptor& adaptor) = delete;

    auto GetType() const { return m_type; }
    auto GetNode() const { return m_node; }

    void UpdateByBrush(GeoAttribute& attr, const model::BrushModel& brush_model);
    void StoreBrush(std::unique_ptr<model::BrushModel>& brush_model);

    void UpdateByAttr(const GeoAttribute& attr);

    std::unique_ptr<GeoShape> ToGeoShape() const;
    void FromGeoShape(const GeoShape& shape);

    // fixme: move to private
public:
    model::BrushModel* GetBrushModel() const;

private:
    void Init(const GeoShapeType& type);

    void UpdateModel(const std::shared_ptr<model::Model>& model);

    static void StoreToAttr(GeoAttribute& dst, const model::BrushModel& src);
    static void LoadFromAttr(model::BrushModel& dst, const GeoAttribute& src);

private:
    GeoShapeType m_type;

    n0::SceneNodePtr m_node = nullptr;

}; // GeoAdaptor

}