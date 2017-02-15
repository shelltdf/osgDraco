#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Point>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/fstream>
#include <osgViewer/Viewer>

#include <iostream>

osg::Node* createMeshOrParticle(int count, bool is_mesh)
{
    if (is_mesh) count *= 3;

    int size_in_byte = count * 3 * 3 * 4 * 2 * sizeof(float);
    printf("size_in_byte : %d\n", size_in_byte);

    //
    osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
    osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();

    srand(time(NULL));

    for (int i = 0; i < count; i++)
    {
        float x = float(rand() % 10000) / 10000.0;
        float y = float(rand() % 10000) / 10000.0;
        float z = float(rand() % 10000) / 10000.0;

        // 1 to -1
        x = x * 2 - 1;
        y = y * 2 - 1;
        z = z * 2 - 1;

        osg::Vec3 v(x, y, z);
        v.normalize();

        raw_vertex->push_back(v);
        raw_normal->push_back(v);
        raw_color->push_back(osg::Vec4(v.x(), v.y(), v.z(), 1));
        raw_uv0->push_back(osg::Vec2(v.x(), v.y()));
    }


    //new geometry
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
    geometry->setVertexArray(raw_vertex);
    geometry->setNormalArray(raw_normal);
    geometry->setNormalBinding(osg::Geometry::AttributeBinding::BIND_PER_VERTEX);
    geometry->setTexCoordArray(0, raw_uv0);
    geometry->setColorArray(raw_color);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    if (is_mesh)
    {
        geometry->addPrimitiveSet(new osg::DrawArrays(
            osg::PrimitiveSet::TRIANGLES, 0, raw_vertex->size()));
    }
    else
    {
        geometry->addPrimitiveSet(new osg::DrawArrays(
            osg::PrimitiveSet::POINTS, 0, raw_vertex->size()));
    }


    //geode
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(geometry);

    return geode;
}

osg::Node* createModel()
{
    osg::MatrixTransform* mt = new osg::MatrixTransform();
    mt->setMatrix(osg::Matrix::translate(0, 10, 0));

    //box
    //osg::Geode* geode_box = new osg::Geode();
    //osg::ShapeDrawable* box = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), 1, 1, 1));
    //geode_box->addDrawable(box);
    //mt->addChild(geode_box);


    //
    osg::ref_ptr<osg::Vec3Array> raw_vertex = new osg::Vec3Array();
    osg::ref_ptr<osg::Vec3Array> raw_normal = new osg::Vec3Array();
    //osg::ref_ptr<osg::Vec4Array> raw_color = new osg::Vec4Array();
    osg::ref_ptr<osg::Vec2Array> raw_uv0 = new osg::Vec2Array();

    //
    raw_vertex->push_back(osg::Vec3(0, 0, 0));
    raw_vertex->push_back(osg::Vec3(1, 0, 0));
    raw_vertex->push_back(osg::Vec3(1, 1, 0));
    raw_vertex->push_back(osg::Vec3(0, 0, 0));
    raw_vertex->push_back(osg::Vec3(1, 1, 0));
    raw_vertex->push_back(osg::Vec3(0, 1, 0));

    raw_normal->push_back(osg::Vec3(0, 0, 1));
    raw_normal->push_back(osg::Vec3(0, 0, 1));
    raw_normal->push_back(osg::Vec3(0, 0, 1));
    raw_normal->push_back(osg::Vec3(0, 0, 1));
    raw_normal->push_back(osg::Vec3(0, 0, 1));
    raw_normal->push_back(osg::Vec3(0, 0, 1));

    //raw_color->push_back(osg::Vec4(1, 0, 0, 1));
    //raw_color->push_back(osg::Vec4(1, 0, 0, 1));
    //raw_color->push_back(osg::Vec4(1, 0, 0, 1));
    //raw_color->push_back(osg::Vec4(0, 1, 0, 1));
    //raw_color->push_back(osg::Vec4(0, 1, 0, 1));
    //raw_color->push_back(osg::Vec4(0, 1, 0, 1));

    raw_uv0->push_back(osg::Vec2(0, 0));
    raw_uv0->push_back(osg::Vec2(1, 0));
    raw_uv0->push_back(osg::Vec2(1, 1));
    raw_uv0->push_back(osg::Vec2(0, 0));
    raw_uv0->push_back(osg::Vec2(1, 1));
    raw_uv0->push_back(osg::Vec2(0, 1));


    //new geometry
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
    geometry->setVertexArray(raw_vertex);
    geometry->setNormalArray(raw_normal);
    geometry->setNormalBinding(osg::Geometry::AttributeBinding::BIND_PER_VERTEX);
    geometry->setTexCoordArray(0, raw_uv0);
    //geometry->setColorArray(raw_color);
    //geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, raw_vertex->size()));

    //geode
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(geometry);

    mt->addChild(geode);
    return mt;
}

int main(int argc, char **argv)
{

    //viewer
    osgViewer::Viewer viewer;
    osg::Group* root = new osg::Group();
    //root->getOrCreateStateSet()->setMode(GL_LIGHTING, false);
    root->getOrCreateStateSet()->setAttributeAndModes(
        new osg::Point(5.0), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    viewer.setSceneData(root);

    //
    bool save_to_pointcloud = false;

    //load model
    //osg::Node* node_obj = osgDB::readNodeFile("../data/house.obj");
    //osg::Node* node_obj = createModel();
    osg::Node* node_obj = createMeshOrParticle(1000 * 10, !save_to_pointcloud);
    root->addChild(node_obj);

#if 1

    osgDB::Options* ops = new osgDB::Options();
    std::string opt_string = "";
    {
        if (save_to_pointcloud) opt_string += "draco_point_cloud ";

        opt_string += "draco_save_normal ";
        //opt_string += "draco_save_color ";
        opt_string += "draco_save_uv0 ";

        opt_string += "draco_compression_level=0 "; //0-10

        opt_string += "draco_position_qb=14 "; //14
        opt_string += "draco_normal_qb=8 "; //12
        opt_string += "draco_color_qb=10 "; //10
        opt_string += "draco_uv0_qb=8 "; //12

    }
    ops->setOptionString(opt_string);



    //save to .drc
    osgDB::writeNodeFile(*node_obj, "test.drc", ops);
    osgDB::writeNodeFile(*node_obj, "test.osgb");
    //osgDB::writeNodeFile(*node_obj, "test.obj");

#endif


#if 1

    //offset
    osg::MatrixTransform* mt = new osg::MatrixTransform();
    mt->setMatrix(osg::Matrix::translate(0, 0, 3));

    //load from .drc
    osg::Node* node_drc = osgDB::readNodeFile("test.drc");
    //osg::Node* node_drc = osgDB::readNodeFile("../data/box/box.drc");
    //osg::Node* node_drc = osgDB::readNodeFile("../data/box/box_pc.drc");
    mt->addChild(node_drc);
    root->addChild(mt);

#endif


#if 1

    //run
    viewer.setUpViewInWindow(100, 100, 1280, 720);
    viewer.run();

#endif

    return 0;
}

