#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include <QFileDialog>
#include <renderthread.h>
#include <raytracing/samplers/stratifiedpixelsampler.h>
#include <scene/materials/volumetricmaterial.h>


MyGL::MyGL(QWidget *parent)
    : GLWidget277(parent)
{
    rendering = false;
    setFocusPolicy(Qt::ClickFocus);
}

MyGL::~MyGL()
{
    makeCurrent();

    vao.destroy();
}

void MyGL::reDraw(){

    QOpenGLTexture* texture = new QOpenGLTexture(p_img);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

//    p_img.save("abcd.bmp", "bmp"); // testing; writes bmp image

    texture->bind();
    GLfloat g_quad_vertex_buffer_data[] = {
        /* X,    Y,     Z,     U,    V*/
         -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
          1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         -1.0f,  1.0f, 0.0f, 0.0f, -1.0f,
         -1.0f,  1.0f, 0.0f, 0.0f, -1.0f,
          1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
          1.0f,  1.0f, 0.0f, 1.0f, -1.0f
     };
    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer); //buffer array active
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    //attach vertex position and UV position in shader file
    int vertex = prog.attributeLocation("vert");
    prog.enableAttributeArray(vertex);
    int vertCoord = prog.attributeLocation("vertTexCoord");
    prog.enableAttributeArray(vertCoord);

    glVertexAttribPointer(vertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glVertexAttribPointer(vertCoord, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    //draw two triangles to make a square
    glDrawArrays(GL_TRIANGLES, 0, 6);
    texture->destroy();
    delete texture;
}

void MyGL::initializeGL()
{
    // Create an OpenGL context
    initializeOpenGLFunctions();

    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    char* vertfile = ":/glsl/progren.vert.glsl";
    char* fragfile = ":/glsl/progren.frag.glsl";

    prog.addShaderFromSourceFile(QOpenGLShader::Vertex  , vertfile);
    prog.addShaderFromSourceFile(QOpenGLShader::Fragment, fragfile);
    prog.link();
    prog.bind();

    // Create a Vertex Attribute Object
    vao.create();

    // Create and set up the diffuse shader
    prog_lambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat-color shader
    prog_flat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    prog_ren.create(":/glsl/progren.vert.glsl", ":/glsl/progren.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    vao.bind();

    //Test scene data initialization
    scene.CreateTestScene();
    integrator.scene = &scene;
    integrator.intersection_engine = &intersection_engine;
    intersection_engine.scene = &scene;
    intersection_engine.bvh = bvhNode::InitTree(scene.objects);
    ResizeToSceneCamera();

    printGLErrorLog();
}

void MyGL::resizeGL(int w, int h)
{
    gl_camera = Camera(w, h);

    glm::mat4 viewproj = gl_camera.getViewProj();

    // Upload the projection matrix
    prog_lambert.setViewProjMatrix(viewproj);
    prog_flat.setViewProjMatrix(viewproj);
    prog_ren.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

// This function is called by Qt any time your GL window is supposed to update
// For example, when the function updateGL is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    // Update the viewproj matrix
    prog_lambert.setViewProjMatrix(gl_camera.getViewProj());
    prog_flat.setViewProjMatrix(gl_camera.getViewProj());
    prog_ren.setViewProjMatrix(glm::mat4(1.f));

    if(rendering){
        reDraw();
    }

    GLDrawScene();
}

void MyGL::GLDrawScene()
{
    for(Geometry *g : scene.objects)
    {
        if(g->drawMode() == GL_TRIANGLES)
        {
            prog_lambert.setModelMatrix(g->transform.T());
            prog_lambert.draw(*this, *g);
        }
        else if(g->drawMode() == GL_LINES)
        {
            prog_flat.setModelMatrix(g->transform.T());
            prog_flat.draw(*this, *g);
        }
    }
    for(Geometry *l : scene.lights)
    {
        prog_flat.setModelMatrix(l->transform.T());
        prog_flat.draw(*this, *l);
    }
    prog_flat.setModelMatrix(glm::mat4(1.0f));
    prog_flat.draw(*this, scene.camera);

    //Recursively traverse the BVH hierarchy stored in the intersection engine and draw each node
    //Recursively traverse the BVH hierarchy stored in the intersection engine and draw each node
    // Break BVHTree into a list of nodes
    std::vector<bvhNode*> nodes;
    bvhNode::FlattenTree(intersection_engine.bvh, nodes);

    // Draw bounding boxes
    prog_flat.setModelMatrix(glm::mat4(1.0f));
    for (bvhNode* n: nodes) {
        prog_flat.draw(*this, n->bounding_box);
    }
}

void MyGL::ResizeToSceneCamera()
{
    this->setFixedWidth(scene.camera.width);
    this->setFixedHeight(scene.camera.height);
    gl_camera = Camera(scene.camera);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        gl_camera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        gl_camera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        gl_camera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        gl_camera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        gl_camera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        gl_camera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        gl_camera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        gl_camera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        gl_camera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        gl_camera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        gl_camera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        gl_camera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_F) {
        gl_camera.CopyAttributes(scene.camera);
    } else if (e->key() == Qt::Key_R) {
        scene.camera = Camera(gl_camera);
        scene.camera.recreate();
    }
    gl_camera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}

void MyGL::SceneLoadDialog()
{
    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Scene"), QString("../scene_files"), tr("*.xml"));
    if(filepath.length() == 0)
    {
        return;
    }

    QFile file(filepath);
    int i = filepath.length() - 1;
    while(QString::compare(filepath.at(i), QChar('/')) != 0)
    {
        i--;
    }
    QStringRef local_path = filepath.leftRef(i+1);
    //Reset all of our objects
    scene.Clear();
    // Clean up BVH Tree
    bvhNode::DeleteTree(intersection_engine.bvh);

#if defined(PHOTON_MAP)
    integrator = PhotonMapIntegrator();
#elif defined(ALL_LIGHTING)
    integrator = TotalLightingIntegrator();
#elif defined(DIRECT_LIGHTING)
    integrator = DirectLightingIntegrator();
#else
    integrator = Integrator();
#endif
    intersection_engine = IntersectionEngine();

    //Load new objects based on the XML file chosen.

#if defined(PHOTON_MAP)
    xml_reader.LoadSceneFromFilePhotonMap(file, local_path, scene, integrator);
#else
    xml_reader.LoadSceneFromFile(file, local_path, scene, integrator);
#endif

    integrator.scene = &scene;
    integrator.intersection_engine = &intersection_engine;
    intersection_engine.scene = &scene;
    intersection_engine.bvh = bvhNode::InitTree(scene.objects);

#ifdef PHOTON_MAP
    integrator.PrePass();
#endif
    ResizeToSceneCamera();
    update();

    // Populate volumetric density buffers.
    for (Geometry *object : integrator.scene->objects) {
        if (object->material->is_volumetric) {
            ((VolumetricMaterial *)object->material)->CalculateDensities(object);
        }
    }
}

void MyGL::cleanThreads(){
    if(!signal)
    {
        for(unsigned int i = 0; i < num_render_threads; i++)
        {
            delete render_threads[i];
        }
        delete [] render_threads;
        signal = true;
    }
}

void MyGL::render_check(){
    bool still_running;
    if(rendering){
        still_running = false;
        for(unsigned int i = 0; i < num_render_threads; i++)
        {
            if(render_threads[i]->isRunning())
            {
                still_running = true;
                update();
                break;
            }
        }
        if(still_running)
        {
            //Free the CPU to let the remaining render threads use it
            QThread::yieldCurrentThread();
        }
        else{
            rendering = false;
            DenoisePixels();
            scene.film.WriteImage(filepath);
        }
    }
    else{
        //Finally, clean up the render thread objects
        cleanThreads();
    }

}

void MyGL::RaytraceScene()
{
    signal = false;
    filepath = QFileDialog::getSaveFileName(0, QString("Save Image"), QString("../rendered_images"), tr("*.bmp"));
    if(filepath.length() == 0)
    {
        return;
    }

    p_img = this->grabFramebuffer(); //current frame buffer values

//#define PERLIN_TEST
#define MULTITHREADED
#ifdef MULTITHREADED
    //Set up 16 (max) threads
    unsigned int width = scene.camera.width;
    unsigned int height = scene.camera.height;
    unsigned int x_block_size = (width >= 4 ? width/4 : 1);
    unsigned int y_block_size = (height >= 4 ? height/4 : 1);
    unsigned int x_block_count = width > 4 ? width/x_block_size : 1;
    unsigned int y_block_count = height > 4 ? height/y_block_size : 1;
    if(x_block_count * x_block_size < width) x_block_count++;
    if(y_block_count * y_block_size < height) y_block_count++;

    num_render_threads = x_block_count * y_block_count;
    render_threads = new RenderThread*[num_render_threads];

    //Launch the render threads we've made
    for(unsigned int Y = 0; Y < y_block_count; Y++)
    {
        //Compute the columns of the image that the thread should render
        unsigned int y_start = Y * y_block_size;
        unsigned int y_end = glm::min((Y + 1) * y_block_size, height);
        for(unsigned int X = 0; X < x_block_count; X++)
        {
            //Compute the rows of the image that the thread should render
            unsigned int x_start = X * x_block_size;
            unsigned int x_end = glm::min((X + 1) * x_block_size, width);
            //Create and run the thread
            render_threads[Y * x_block_count + X] = new RenderThread(x_start, x_end, y_start, y_end, scene.sqrt_samples, 5, &(scene.film), &(scene.camera), &(integrator), p_img);
            render_threads[Y * x_block_count + X]->start();
        }
    }
//    #define PROGRESSIVE
    #ifdef PROGRESSIVE
    rendering = true;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(render_check()));
    timer->start(1000); //every 1000 milliseconds re render
    #else
    bool still_running;
        do
        {
            still_running = false;
            for(unsigned int i = 0; i < num_render_threads; i++)
            {
                if(render_threads[i]->isRunning())
                {
                    still_running = true;
                    break;
                }
            }
            if(still_running)
            {
                //Free the CPU to let the remaining render threads use it
                QThread::yieldCurrentThread();
            }
        }
        while(still_running);

        //Finally, clean up the render thread objects
        for(unsigned int i = 0; i < num_render_threads; i++)
        {
            delete render_threads[i];
        }
        delete [] render_threads;
        scene.film.WriteImage(filepath);
    #endif

#elif defined(PERLIN_TEST)
    for(unsigned int j = 0; j < scene.camera.height; j++)
    {
        for(unsigned int i = 0; i < scene.camera.width; i++)
        {
            scene.film.pixels[i][j] = glm::vec3(VolumetricMaterial::PerlinNoise_3d(i, j, 10));
        }
    }
#else
    StratifiedPixelSampler pixel_sampler(scene.sqrt_samples,0);
    rendering = true;
    for(unsigned int i = 0; i < scene.camera.width; i++)
    {
        for(unsigned int j = 0; j < scene.camera.height; j++)
        {
            QList<glm::vec2> sample_points = pixel_sampler.GetSamples(200, 175);
//            QList<glm::vec2> sample_points = pixel_sampler.GetSamples(i, j);
            glm::vec3 accum_color;
            for(int a = 0; a < sample_points.size(); a++)
            {
                glm::vec3 color = integrator.TraceRay(scene.camera.Raycast(sample_points[a]), 0, i, j);
                accum_color += color;
            }
            scene.film.pixels[i][j] = accum_color / (float)sample_points.size();
//            glm::vec3 pixel_color = scene.film.pixels[i][j];
//            if(pixel_color.x > 1.f) pixel_color.x = 1.f;
//            if(pixel_color.y > 1.f) pixel_color.y = 1.f;
//            if(pixel_color.z > 1.f) pixel_color.z = 1.f;
//            p_img.setPixel(i, j, qRgb(pixel_color.x * 255, pixel_color.y * 255 , pixel_color.z * 255));
//            reDraw();
        }
    }
    DenoisePixels();
    scene.film.WriteImage(filepath);

#endif
}

void MyGL::DenoisePixels() {
    std::vector<std::vector<glm::vec3>> tmp_colors;
    tmp_colors = std::vector<std::vector<glm::vec3>>(scene.camera.width);

    for(unsigned int i = 0; i < scene.camera.width; i++)
    {
        tmp_colors[i] = std::vector<glm::vec3>(scene.camera.height);
    }

    for(unsigned int i = 1; i < scene.camera.width-1; i++)
    {
        for(unsigned int j = 1; j < scene.camera.height-1; j++)
        {
            glm::vec3 original_color = scene.film.pixels[i][j];
            float original_depth = scene.film.pixel_depths[i][j];
            std::vector<glm::vec3> neighbors;

            // Check surrounding pixels in cardinal directions.
            if (fabs(scene.film.pixel_depths[i-1][j] - original_depth) < 0.5) {
                // left
                neighbors.push_back(scene.film.pixels[i-1][j]);
            }
            if (fabs(scene.film.pixel_depths[i+1][j] - original_depth) < 0.5) {
                // right
                neighbors.push_back(scene.film.pixels[i+1][j]);
            }
            if (fabs(scene.film.pixel_depths[i][j-1] - original_depth) < 0.5) {
                // up
                neighbors.push_back(scene.film.pixels[i][j-1]);
            }
            if (fabs(scene.film.pixel_depths[i][j+1] - original_depth) < 0.5) {
                // down
                neighbors.push_back(scene.film.pixels[i][j+1]);
            }

            // Check surrounding pixels in cardinal directions.
            if (fabs(scene.film.pixel_depths[i-1][j+1] - original_depth) < 0.5) {
                // upper_left
                neighbors.push_back(scene.film.pixels[i-1][j+1]);
            }
            if (fabs(scene.film.pixel_depths[i+1][j+1] - original_depth) < 0.5) {
                // upper_right
                neighbors.push_back(scene.film.pixels[i+1][j+1]);
            }
            if (fabs(scene.film.pixel_depths[i-1][j-1] - original_depth) < 0.5) {
                // lower_left
                neighbors.push_back(scene.film.pixels[i-1][j-1]);
            }
            if (fabs(scene.film.pixel_depths[i+1][j-1] - original_depth) < 0.5) {
                // lower_right
                neighbors.push_back(scene.film.pixels[i+1][j-1]);
            }

            glm::vec3 suggested_color(0);
            if (!neighbors.empty()) {
                for (glm::vec3 n : neighbors) {
                    suggested_color += n / float(neighbors.size());
                }
            } else {
                tmp_colors[i][j] = original_color;
            }

            tmp_colors[i][j] = original_color * 0.5f
                    + suggested_color * 0.5f;
        }
    }

    for(unsigned int i = 1; i < scene.camera.width-1; i++)
    {
        for(unsigned int j = 1; j < scene.camera.height-1; j++)
        {
            scene.film.pixels[i][j] = tmp_colors[i][j];
        }
    }
}
