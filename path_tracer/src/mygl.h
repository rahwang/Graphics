#pragma once

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <QImage>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QBuffer>
#include <QTimer>

#include <openGL/glwidget277.h>
#include <la.h>
#include <openGL/shaderprogram.h>
#include <scene/camera.h>
#include <scene/scene.h>
#include <la.h>
#include <scene/xmlreader.h>
#include <scene/xmlreader.h>
#include <raytracing/Integrator.h>
#include <raytracing/directlightingintegrator.h>
#include <raytracing/totallightingintegrator.h>
#include <renderthread.h>

#include <raytracing/photonmapintegrator.h>

// Uncomment corresponding section to build with specified integrator type
//#define PHOTON_MAP
//#define ALL_LIGHTING
#define DIRECT_LIGHTING

class MyGL
    : public GLWidget277
{
    Q_OBJECT
private:
    QOpenGLVertexArrayObject vao;

    ShaderProgram prog_lambert;
    ShaderProgram prog_flat;
    ShaderProgram prog_ren;

    Camera gl_camera;//This is a camera we can move around the scene to view it from any angle.
                                //However, the camera defined in the config file is the one from which the scene will be rendered.
                                //If you move gl_camera, you will be able to see the viewing frustum of the scene's camera.

    Scene scene;
    XMLReader xml_reader;
#if defined(PHOTON_MAP)
    PhotonMapIntegrator integrator;
#elif defined(ALL_LIGHTING)
    TotalLightingIntegrator integrator;
#elif defined(DIRECT_LIGHTING)
    DirectLightingIntegrator integrator;
#else
    Integrator integrator;
#endif
    IntersectionEngine intersection_engine;

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void SceneLoadDialog();
    void GLDrawScene();
    void ResizeToSceneCamera();

    void RaytraceScene();
    //reDraw: Progrssive drawing on framebuffer
    void reDraw();
    //clean and delete threads
    void cleanThreads();

    unsigned int num_render_threads;
    RenderThread** render_threads;

    //the custom shader to draw texture on
    QOpenGLShaderProgram prog;

    //flag to check if thread still rendering
    bool rendering;
    //image to store the pixel buffer
    QImage p_img;
    //flag to check if threads still active
    bool signal = false;
    QString filepath;

    void DenoisePixels();
    void CompressColors(int k);

protected:
    void keyPressEvent(QKeyEvent *e);

signals:
    void sig_ResizeToCamera(int,int);
private slots:
    //function to check if threads active; write to image file
    void render_check();
};
