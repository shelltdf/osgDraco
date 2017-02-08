
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
    size_t size() { return triangles.size(); }
};

//GeometryData
class GeometryData
{
public:

    GeometryData()
    {
        raw_vertex = new osg::Vec3Array();
        raw_color = new osg::Vec4Array();
        raw_normal = new osg::Vec3Array();
        raw_uv0 = new osg::Vec2Array();
    }

    void resize(rsize_t s)
    {
        raw_vertex->resize(s);
        raw_color->resize(s);
        raw_normal->resize(s);
        raw_uv0->resize(s);
    }

    osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
    osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();
};

//flat
class GeometryFlat
    :public osg::NodeVisitor
{
public:
    GeometryFlat(bool v = true, bool c = true, bool n = true, bool uv0 = true)
        :osg::NodeVisitor(osg::NodeVisitor::TraversalMode::TRAVERSE_ALL_CHILDREN)
        , enable_vertex(v)
        , enable_color(c)
        , enable_normal(n)
        , enable_uv0(uv0)
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

    bool enable_vertex;
    bool enable_color;
    bool enable_normal;
    bool enable_uv0;

    //ºÏ²¢geometryµÄ¾ØÕó
    void processGeomatry(osg::Geometry& geometry, osg::Matrix in_matrix)
    {
        //triangulate
        osg::TriangleIndexFunctor< TriangleCollector > tif;
        (&geometry)->accept(tif);

        //vertex normal color uv0
        osg::Vec3Array* vertex = 0;
        osg::Vec4Array* color = 0;
        osg::Vec3Array* normal = 0;
        osg::Vec2Array* uv0 = 0;
        if (enable_vertex) vertex = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
        if (enable_color) color = dynamic_cast<osg::Vec4Array*>(geometry.getColorArray());
        if (enable_normal) normal = dynamic_cast<osg::Vec3Array*>(geometry.getNormalArray());
        if (enable_uv0) uv0 = dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(0));
        //osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
        //osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
        //osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
        //osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();

        //for normal correct
        osg::Matrix in_matrix_rs = in_matrix;
        in_matrix_rs.setTrans(0, 0, 0);

        //size
        size_t size = std::max<>(tif.triangles.size(), vertex ? vertex->size() : 0);
        //size_t step = tif.triangles.size()>0 ? 3 : 1;
        size_t step = 1;

        //for
        for (size_t i = 0; i < size; i += step)
        {
            int t0 = tif.size() > 0 ? tif.triangles[i + 0] : i + 0;

            //vertex
            if (vertex)
            {
                m_geomtry_data.raw_vertex->push_back(vertex->at(t0) *in_matrix);
            }

            //color
            if (color)
            {
                m_geomtry_data.raw_color->push_back(color->at(t0));
            }
            else
            {
                m_geomtry_data.raw_color->push_back(osg::Vec4(1, 1, 1, 1));
            }

            //normal
            if (normal)
            {
                osg::Vec3 n0 = normal->at(t0) *in_matrix_rs;
                n0.normalize();
                m_geomtry_data.raw_normal->push_back(n0);
            }
            else
            {
                m_geomtry_data.raw_normal->push_back(osg::Vec3(0, 0, 1));
            }

            //uv0
            if (uv0)
            {
                m_geomtry_data.raw_uv0->push_back(uv0->at(t0));
            }
            else
            {
                m_geomtry_data.raw_uv0->push_back(osg::Vec2(0, 0));
            }
        }//for


        // if normal equal 0
        if (tif.size() > 0 && vertex && normal)
        {
            for (size_t i = 0; i < size; i += step)
            {
                int t0 = tif.triangles[i + 0];
                int t1 = tif.triangles[i + 0];
                int t2 = tif.triangles[i + 0];

                osg::Vec3 n0 = normal->at(t0);
                osg::Vec3 n1 = normal->at(t1);
                osg::Vec3 n2 = normal->at(t2);

                if (n0.length() == 0
                    || n1.length() == 0
                    || n2.length() == 0)
                {
                    osg::Vec3 p0 = vertex->at(t0);
                    osg::Vec3 p1 = vertex->at(t1);
                    osg::Vec3 p2 = vertex->at(t2);

                    osg::Vec3 n = (p2 - p1) ^ (p0 - p1);

                    (*normal)[t0] = n;
                    (*normal)[t1] = n;
                    (*normal)[t2] = n;
                }
            }//for
        }//if


    }

};

