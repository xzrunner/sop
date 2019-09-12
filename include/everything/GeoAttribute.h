#pragma once

#include "everything/Variable.h"

#include "SM_Cube.h"

#include <vector>

#include <boost/noncopyable.hpp>

namespace evt
{

class GeoShape;

class GeoAttribute : boost::noncopyable
{
public:
    struct Point
    {
        Point(const sm::vec3& pos) : pos(pos) {}

        sm::vec3 pos;

    }; // Point

    struct Primitive;
    struct Vertex
    {
        std::shared_ptr<Point>   point = nullptr;
        std::weak_ptr<Primitive> prim;

    }; // Vertex

    struct Primitive
    {
        std::vector<std::shared_ptr<Vertex>> vertices;

    }; // Primitive

    //struct Detail
    //{
    //}; // Detail

    struct AttrList
    {
        void Clear();

        std::vector<std::string> names;
        std::vector<Variable>    vars;

    }; // AttrList

public:
    GeoAttribute() {}
    GeoAttribute(const GeoShape& shape);
    GeoAttribute(const GeoAttribute& attr);
    GeoAttribute& operator = (const GeoAttribute& attr);

    auto& GetPoints() const { return m_points; }
    void  SetPoints(const std::vector<std::shared_ptr<Point>>& points);

    auto& GetVertices() const { return m_vertices; }
    void  SetVertices(const std::vector<std::shared_ptr<Vertex>>& vertices);

    auto& GetPrimtives() const { return m_primtives; }
    void  SetPrimtives(const std::vector<std::shared_ptr<Primitive>>& prims);

    void Reset(const std::vector<std::shared_ptr<Point>>& points,
        const std::vector<std::shared_ptr<Vertex>>& vertices,
        const std::vector<std::shared_ptr<Primitive>>& prims);

    void Combine(const GeoAttribute& attr);

    void FromGeoShape(const GeoShape& shape);

    auto& GetAABB() const { return m_aabb; }

    template<typename T>
    int QueryIndex(const T& i) const;

private:
    void Clear();

    void SetupAABB();

private:
    std::vector<std::shared_ptr<Point>>     m_points;
    std::vector<std::shared_ptr<Vertex>>    m_vertices;
    std::vector<std::shared_ptr<Primitive>> m_primtives;
//    Detail                 m_detail;

    AttrList m_attr_point;
    AttrList m_attr_vertex;
    AttrList m_attr_prim;
    AttrList m_attr_detail;

    sm::cube m_aabb;

    friend class GeoAdaptor;

}; // GeoAttribute

}

#include "everything/GeoAttribute.inl"