
#include <osg/TriangleIndexFunctor>

//TriangleCollector
struct TriangleCollector
{
    void operator()(unsigned int i1, unsigned int i2, unsigned int i3)
    {
        if (i1 == i2 || i1 == i3 || i2 == i3) return;
        triangles.push_back(i1);
        triangles.push_back(i2);
        triangles.push_back(i3);
    }
    std::vector<unsigned int> triangles;
};

//GeometryData
class GeometryData
{
public:

    GeometryData()
    {
        raw_vertex = new osg::Vec3Array();
        raw_normal = new osg::Vec3Array();
        //raw_color = new osg::Vec4Array();
        raw_uv0 = new osg::Vec2Array();
    }

    void resize(rsize_t s)
    {
        raw_vertex->resize(s);
        raw_normal->resize(s);
        //raw_color->resize(s);
        raw_uv0->resize(s);
    }

    osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
    //osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
    osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();
};

//flat
class GeometryFlat
    :public osg::NodeVisitor
{
public:
    GeometryFlat()
        :osg::NodeVisitor(osg::NodeVisitor::TraversalMode::TRAVERSE_ALL_CHILDREN)
    {
    }
    virtual ~GeometryFlat() {}

    virtual void apply(osg::Transform& transform)
    {
        osg::Matrix m = m_current_matrix;
        transform.computeLocalToWorldMatrix(m_current_matrix, 0);
        traverse(transform);
        m_current_matrix = m;
    }

    virtual void apply(osg::Geode& geode)
    {
        unsigned int chaild_num = geode.getNumChildren();
        for (unsigned int i = 0; i < chaild_num; i++)
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getChild(i));
            if (geom)
            {
                processGeomatry(*geom, m_current_matrix);
            }
        }

        traverse(geode);
    }

    GeometryData m_geomtry_data;

private:

    osg::Matrix m_current_matrix;

    //ºÏ²¢geometryµÄ¾ØÕó
    void processGeomatry(osg::Geometry& geometry, osg::Matrix in_matrix)
    {
        //triangulate
        osg::TriangleIndexFunctor< TriangleCollector > tif;
        (&geometry)->accept(tif);

        //vertex normal color uv0
        osg::Vec3Array* vertex = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
        osg::Vec3Array* normal = dynamic_cast<osg::Vec3Array*>(geometry.getNormalArray());
        osg::Vec4Array* color = dynamic_cast<osg::Vec4Array*>(geometry.getColorArray());
        osg::Vec2Array* uv0 = dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(0));
        //osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
        //osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
        //osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
        //osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();

        //for normal correct
        osg::Matrix in_matrix_rs = in_matrix;
        in_matrix_rs.setTrans(0, 0, 0);

        //for
        for (size_t i = 0; i < tif.triangles.size(); i += 3)
        {
            //vertex
            if (vertex)
            {
                m_geomtry_data.raw_vertex->push_back(vertex->at(tif.triangles[i + 0]) *in_matrix);
                m_geomtry_data.raw_vertex->push_back(vertex->at(tif.triangles[i + 1]) *in_matrix);
                m_geomtry_data.raw_vertex->push_back(vertex->at(tif.triangles[i + 2]) *in_matrix);
            }

#if 0
            //color
            if (color)
            {
                m_geomtry_data.raw_color->push_back(color->at(tif.triangles[i + 0]));
                m_geomtry_data.raw_color->push_back(color->at(tif.triangles[i + 1]));
                m_geomtry_data.raw_color->push_back(color->at(tif.triangles[i + 2]));
            }
            else
            {
                m_geomtry_data.raw_color->push_back(osg::Vec4(1, 1, 1, 1));
                m_geomtry_data.raw_color->push_back(osg::Vec4(1, 1, 1, 1));
                m_geomtry_data.raw_color->push_back(osg::Vec4(1, 1, 1, 1));
            }
#endif

            //normal
            if (normal)
            {
                osg::Vec3 n0 = normal->at(tif.triangles[i + 0]) *in_matrix_rs;
                osg::Vec3 n1 = normal->at(tif.triangles[i + 1]) *in_matrix_rs;
                osg::Vec3 n2 = normal->at(tif.triangles[i + 2]) *in_matrix_rs;
                n0.normalize();
                n1.normalize();
                n2.normalize();

                m_geomtry_data.raw_normal->push_back(n0);
                m_geomtry_data.raw_normal->push_back(n1);
                m_geomtry_data.raw_normal->push_back(n2);
            }
            else
            {
                m_geomtry_data.raw_normal->push_back(osg::Vec3(0, 0, 1));
                m_geomtry_data.raw_normal->push_back(osg::Vec3(0, 0, 1));
                m_geomtry_data.raw_normal->push_back(osg::Vec3(0, 0, 1));
            }

            //uv0
            if (uv0)
            {
                m_geomtry_data.raw_uv0->push_back(uv0->at(tif.triangles[i + 0]));
                m_geomtry_data.raw_uv0->push_back(uv0->at(tif.triangles[i + 1]));
                m_geomtry_data.raw_uv0->push_back(uv0->at(tif.triangles[i + 2]));
            }
            else
            {
                m_geomtry_data.raw_uv0->push_back(osg::Vec2(0, 0));
                m_geomtry_data.raw_uv0->push_back(osg::Vec2(0, 0));
                m_geomtry_data.raw_uv0->push_back(osg::Vec2(0, 0));
            }
        }
    }

};

