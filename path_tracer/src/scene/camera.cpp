#include "camera.h"

#include <la.h>
#include <iostream>
#include <helpers.h>

Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    lens_radius(0.f),
    focal_distance(-10.f),
    eye(e),
    ref(r),
    world_up(worldUp)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    lens_radius(c.lens_radius),
    focal_distance(c.focal_distance),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H)
{}

void Camera::CopyAttributes(const Camera &c)
{
    fovy = c.fovy;
    near_clip = c.near_clip;
    far_clip = c.far_clip;
    lens_radius = c.lens_radius;
    focal_distance = c.focal_distance;
    eye = c.eye;
    ref = c.ref;
    look = c.look;
    up = c.up;
    right = c.right;
    width = c.width;
    height = c.height;
    aspect = c.aspect;
    V = c.V;
    H = c.H;
}

void Camera::RecomputeAttributes()
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::cross(right, look);

    float tan_fovy = tan(fovy/2 * DEG2RAD);
    float len = glm::length(ref - eye);
    aspect = float(width)/height;
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(fovy * DEG2RAD, width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

glm::mat4 Camera::ViewMatrix()
{
    // View matrix = O * T
    glm::mat4 orientation_mat = glm::mat4(
                right.x, up.x, look.x, 0.f,
                right.y, up.y, look.y, 0.f,
                right.z, up.z, look.z, 0.f,
                0.f, 0.f, 0.f, 1.f
                );

    glm::mat4 translation_mat = glm::mat4(
                1.f, 0.f, 0.f, 0.f,
                0.f, 1.f, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f,
                -eye.x, -eye.y, -eye.z, 1.f
                );

    return orientation_mat * translation_mat;
}

glm::mat4 Camera::PerspectiveProjectionMatrix()
{
    // Compute top, bottom, left, right based on near, far, fovy, and aspect
    float top = near_clip * glm::tan(glm::radians(fovy) / 2);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    // Build mat4 by columns
    return glm::mat4(
                (2 * near_clip) / (right - left), 0.f, 0.f, 0.f,
                0.f, (2 * near_clip) / (top - bottom), 0.f, 0.f,
                -(right + left) / (right - left), - (top + bottom) / (top - bottom), far_clip / (far_clip - near_clip), 1.f,
                0.f, 0.f, - (far_clip * near_clip)  / (far_clip - near_clip), 0.f
                );
}

void Camera::RotateAboutUp(float deg)
{
    deg *= DEG2RAD;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), deg, up);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
    deg *= DEG2RAD;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), deg, right);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}

void Camera::TranslateAlongLook(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
    ref += translation;
}
void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
    ref += translation;
}

Ray Camera::Raycast(const glm::vec2 &pt)
{
    return Raycast(pt.x, pt.y);
}

Ray Camera::Raycast(float x, float y)
{
    float ndc_x = (2*x/width - 1);
    float ndc_y = (1 - 2*y/height);
    return RaycastNDC(ndc_x, ndc_y);
}

Ray Camera::RaycastNDC(float ndc_x, float ndc_y)
{
    glm::vec3 P = ref + ndc_x*H + ndc_y*V;
    Ray result(eye, P - eye);

    // Modified for Depth of field
    if (lens_radius > 0.f) {

        // Sample point on lens, takes a (u,v) sample position and map
        // it to a 2D disk centered at the origin (0,0), then scale by the
        // lens radius
        float lens_u, lens_v;
        float r1 = float(rand()) / float(RAND_MAX);
        float r2 = float(rand()) / float(RAND_MAX);
        ConcentricSampleDisk(r1, r2, lens_u, lens_v);
        lens_u *= lens_radius;
        lens_v *= lens_radius;

        // Compute point on plane of focus
        float ft = focal_distance / result.direction.z;
        glm::vec3 point_of_focus = result.origin + ft * result.direction;

        // Update ray for effect of lens
        result.origin = glm::vec3(result.origin.x + lens_u, result.origin.y + lens_v, result.origin.z);
        result.direction = glm::normalize(point_of_focus - result.origin);
    }
    return result;
}

void Camera::create()
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> col;
    std::vector<GLuint> idx;

    //0: Eye position
    pos.push_back(eye);
    //1 - 4: Near clip
        //Lower-left
        Ray r = this->RaycastNDC(-1,-1);
        pos.push_back(eye + r.direction * near_clip);
        //Lower-right
        r = this->RaycastNDC(1,-1);
        pos.push_back(eye + r.direction * near_clip);
        //Upper-right
        r = this->RaycastNDC(1,1);
        pos.push_back(eye + r.direction * near_clip);
        //Upper-left
        r = this->RaycastNDC(-1,1);
        pos.push_back(eye + r.direction * near_clip);
    //5 - 8: Far clip
        //Lower-left
        r = this->RaycastNDC(-1,-1);
        pos.push_back(eye + r.direction * far_clip);
        //Lower-right
        r = this->RaycastNDC(1,-1);
        pos.push_back(eye + r.direction * far_clip);
        //Upper-right
        r = this->RaycastNDC(1,1);
        pos.push_back(eye + r.direction * far_clip);
        //Upper-left
        r = this->RaycastNDC(-1,1);
        pos.push_back(eye + r.direction * far_clip);

    for(int i = 0; i < 9; i++){
        col.push_back(glm::vec3(1,1,1));
    }

    //Frustum lines
    idx.push_back(1);idx.push_back(5);
    idx.push_back(2);idx.push_back(6);
    idx.push_back(3);idx.push_back(7);
    idx.push_back(4);idx.push_back(8);
    //Near clip
    idx.push_back(1);idx.push_back(2);
    idx.push_back(2);idx.push_back(3);
    idx.push_back(3);idx.push_back(4);
    idx.push_back(4);idx.push_back(1);
    //Far clip
    idx.push_back(5);idx.push_back(6);
    idx.push_back(6);idx.push_back(7);
    idx.push_back(7);idx.push_back(8);
    idx.push_back(8);idx.push_back(5);

    //Camera axis
    pos.push_back(eye); col.push_back(glm::vec3(0,0,1)); idx.push_back(9);
    pos.push_back(eye + look); col.push_back(glm::vec3(0,0,1));idx.push_back(10);
    pos.push_back(eye); col.push_back(glm::vec3(1,0,0));idx.push_back(11);
    pos.push_back(eye + right); col.push_back(glm::vec3(1,0,0));idx.push_back(12);
    pos.push_back(eye); col.push_back(glm::vec3(0,1,0));idx.push_back(13);
    pos.push_back(eye + up); col.push_back(glm::vec3(0,1,0));idx.push_back(14);

    count = idx.size();

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(idx.data(), count * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(pos.data(), pos.size() * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(col.data(), col.size() * sizeof(glm::vec3));
}

GLenum Camera::drawMode(){return GL_LINES;}
