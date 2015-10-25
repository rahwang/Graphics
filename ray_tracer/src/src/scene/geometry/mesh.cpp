#include <scene/geometry/mesh.h>
#include <la.h>
#include <tinyobj/tiny_obj_loader.h>
#include <iostream>

Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3):
    Triangle(p1, p2, p3, glm::vec3(1), glm::vec3(1), glm::vec3(1), glm::vec2(0), glm::vec2(0), glm::vec2(0))
{
    for(int i = 0; i < 3; i++)
    {
        normals[i] = plane_normal;
    }
}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3):
    Triangle(p1, p2, p3, n1, n2, n3, glm::vec2(0), glm::vec2(0), glm::vec2(0))
{}


Triangle::Triangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3, const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3)
{
    plane_normal = glm::normalize(glm::cross(p2 - p1, p3 - p2));
    points[0] = p1;
    points[1] = p2;
    points[2] = p3;
    normals[0] = n1;
    normals[1] = n2;
    normals[2] = n3;
    uvs[0] = t1;
    uvs[1] = t2;
    uvs[2] = t3;
}

float Triangle::GetArea(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
{
    return glm::length(glm::cross(p1 - p2, p3 - p2)) * 0.5f;
}

//Returns the interpolation of the triangle's three normals based on the point inside the triangle that is given.
glm::vec3 Triangle::GetNormal(const glm::vec3 &P)
{
    float A = GetArea(points[0], points[1], points[2]);
    float A0 = GetArea(points[1], points[2], P);
    float A1 = GetArea(points[0], points[2], P);
    float A2 = GetArea(points[0], points[1], P);
    return glm::normalize(normals[0] * A0/A + normals[1] * A1/A + normals[2] * A2/A);
}

glm::vec2 Triangle::GetUVCoordinates(const glm::vec3 &point) {
    float total_area = GetArea(points[0], points[1], points[2]);
    float A = GetArea(point, points[1], points[2]) / total_area;
    float B = GetArea(point, points[0], points[2]) / total_area;
    float C = GetArea(point, points[0], points[1]) / total_area;
    return uvs[0] * A + uvs[1] * B + uvs[2] * C;
}

glm::vec4 Triangle::GetNormal(const glm::vec4 &position)
{
    glm::vec3 result = GetNormal(glm::vec3(position));
    return glm::vec4(result, 0);
}

//HAVE THEM IMPLEMENT THIS
Intersection Triangle::GetIntersection(Ray r, Camera &camera)
{
    // Get ray in local space.
    Ray r_local = r.GetTransformedCopy(transform.invT());

    Intersection intersection;
    // First check for intersection with plane defined by triangle.
    // Using the formula dot(N, (S-R_origin)) / dot(N, R_direction).

    // A ray will not intersect with a plane if it's parallel.
    // If parallel, dot(normal, R_direction) will be zero.
    float denominator = glm::dot(plane_normal, r_local.direction);
    if (fequal(denominator, 0.0f)) {
        return intersection;
    }

    // Calculate t in object space.
    // t = distance along ray where intersection occurs.
    float t = glm::dot(plane_normal, (points[0] - r_local.origin))
            / denominator;

    // If t is negative, no intersection.
    if (t < 0) {
        return intersection;
    }
    // Find intersection point in object space.
    glm::vec3 point = r_local.origin + t * r_local.direction;

    // Check if the point falls within the triangle.
    // True if the areas of the three triangles formed by plane intersection
    // point p with the vertices have an area which sums to the total triangle
    // area.
    float total_area = GetArea(points[0], points[1], points[2]);
    float sub_area0 = GetArea(point, points[0], points[1]) / total_area;
    float sub_area1 = GetArea(point, points[0], points[2]) / total_area;
    float sub_area2 = GetArea(point, points[1], points[2]) / total_area;
    if (fequal(sub_area0 + sub_area1 + sub_area2, 1.0f)
            && (sub_area0 >= 0.0f) && (sub_area0 <= 1.0f)
            && (sub_area1 >= 0.0f) && (sub_area1 <= 1.0f)
            && (sub_area2 >= 0.0f) && (sub_area2 <= 1.0f)) {

        // Do normal mapping.
        //glm::vec3 new_normal = NormalMapping(point, GetNormal(point));

        intersection.point = point;
        intersection.normal = GetNormal(point);
        intersection.t = t;
        intersection.object_hit = this;
        intersection.color = material->base_color
                * Material::GetImageColor(GetUVCoordinates(point), material->texture);
    }
    return intersection;
}

glm::vec3 Triangle::NormalMapping(const glm::vec3 &point, const glm::vec3 &normal) {
    glm::vec3 delta_pos0 = points[1] - points[0];
    glm::vec3 delta_pos1 = points[2] - points[0];
    glm::vec2 delta_uvs0 = uvs[1] - uvs[0];
    glm::vec2 delta_uvs1 = uvs[2] - uvs[0];
    glm::vec3 tangent = (delta_uvs1.y * delta_pos0 - delta_uvs0.y * delta_pos1)
            / (delta_uvs1.y * delta_uvs0.x - delta_uvs0.y * delta_uvs1.x);
    glm::vec3 bitangent;
    if (delta_uvs1.y != 0) {
        bitangent = (delta_pos1 - delta_uvs1.x * tangent) / delta_uvs1.y;
    } else {
        bitangent = (delta_pos0 - delta_uvs0.x * tangent) / delta_uvs0.y;
    }
    glm::vec3 new_normal = material->GetObjectNormal(
                   GetUVCoordinates(point), normal, tangent, bitangent);
    return new_normal;
}

bvhNode *Triangle::SetBoundingBox() {
    bvhNode *node = new bvhNode();

    glm::vec3 vertex0 = glm::vec3(transform.T() * glm::vec4(points[0], 1.0f));
    glm::vec3 vertex1 = glm::vec3(transform.T() * glm::vec4(points[1], 1.0f));
    glm::vec3 vertex2 = glm::vec3(transform.T() * glm::vec4(points[2], 1.0f));

    float min_x = fmin(fmin(vertex0.x, vertex1.x), vertex2.x);
    float min_y = fmin(fmin(vertex0.y, vertex1.y), vertex2.y);
    float min_z = fmin(fmin(vertex0.z, vertex1.z), vertex2.z);
    float max_x = fmax(fmax(vertex0.x, vertex1.x), vertex2.x);
    float max_y = fmax(fmax(vertex0.y, vertex1.y), vertex2.y);
    float max_z = fmax(fmax(vertex0.z, vertex1.z), vertex2.z);

    bounding_box = &(node->bounding_box);
    bounding_box->minimum = glm::vec3(min_x, min_y, min_z);
    bounding_box->maximum = glm::vec3(max_x, max_y, max_z);
    bounding_box->center = bounding_box->minimum
            + (bounding_box->maximum - bounding_box->minimum)/ 2.0f;
    bounding_box->object = this;
    bounding_box->SetNormals();
    bounding_box->create();

    return node;
}

Intersection Mesh::GetIntersection(Ray r, Camera &camera)
{
    Intersection intersection;

    intersection = bvh->GetIntersection(r, camera);
    intersection.point = glm::vec3(transform.T() * glm::vec4(intersection.point, 1.0f));
    intersection.normal = glm::normalize(glm::vec3(transform.invTransT()
                                                   * glm::vec4(intersection.normal, 0.0f)));
    intersection.t = glm::length(intersection.point - r.origin);
    return intersection;
}

glm::vec2 Mesh::GetUVCoordinates(const glm::vec3 &point) {
    return glm::vec2(0);
}

glm::vec3 Mesh::NormalMapping(const glm::vec3 &point, const glm::vec3 &normal)
{
    return glm::vec3(0);
}

bvhNode *Mesh::SetBoundingBox() {
    std::vector<bvhNode*> leaves;
    foreach (Triangle *face, faces) {
        face->transform = transform;
        leaves.push_back(face->SetBoundingBox());
    }
    bvh = bvhNode::CreateTree(leaves, 0, 0, leaves.size()-1);

    bvhNode *node = new bvhNode();
    node->bounding_box = bvh->bounding_box;
    node->left = bvh->left;
    node->right = bvh->right;
    bounding_box = &(node->bounding_box);
    bounding_box->object = this;

    return node;
}

void Mesh::SetMaterial(Material *m)
{
    this->material = m;
    for(Triangle *t : faces)
    {
        t->SetMaterial(m);
    }
}

void Mesh::LoadOBJ(const QStringRef &filename, const QStringRef &local_path)
{
    QString filepath = local_path.toString(); filepath.append(filename);
    std::vector<tinyobj::shape_t> shapes; std::vector<tinyobj::material_t> materials;
    std::string errors = tinyobj::LoadObj(shapes, materials, filepath.toStdString().c_str());
    std::cout << errors << std::endl;
    if(errors.size() == 0)
    {
        //Read the information from the vector of shape_ts
        for(unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<float> &positions = shapes[i].mesh.positions;
            std::vector<float> &normals = shapes[i].mesh.normals;
            std::vector<float> &uvs = shapes[i].mesh.texcoords;
            std::vector<unsigned int> &indices = shapes[i].mesh.indices;
            for(unsigned int j = 0; j < indices.size(); j += 3)
            {
                glm::vec3 p1(positions[indices[j]*3], positions[indices[j]*3+1], positions[indices[j]*3+2]);
                glm::vec3 p2(positions[indices[j+1]*3], positions[indices[j+1]*3+1], positions[indices[j+1]*3+2]);
                glm::vec3 p3(positions[indices[j+2]*3], positions[indices[j+2]*3+1], positions[indices[j+2]*3+2]);

                Triangle* t = new Triangle(p1, p2, p3);
                if(normals.size() > 0)
                {
                    glm::vec3 n1(normals[indices[j]*3], normals[indices[j]*3+1], normals[indices[j]*3+2]);
                    glm::vec3 n2(normals[indices[j+1]*3], normals[indices[j+1]*3+1], normals[indices[j+1]*3+2]);
                    glm::vec3 n3(normals[indices[j+2]*3], normals[indices[j+2]*3+1], normals[indices[j+2]*3+2]);
                    t->normals[0] = n1;
                    t->normals[1] = n2;
                    t->normals[2] = n3;
                }
                if(uvs.size() > 0)
                {
                    glm::vec2 t1(uvs[indices[j]*2], uvs[indices[j]*2+1]);
                    glm::vec2 t2(uvs[indices[j+1]*2], uvs[indices[j+1]*2+1]);
                    glm::vec2 t3(uvs[indices[j+2]*2], uvs[indices[j+2]*2+1]);
                    t->uvs[0] = t1;
                    t->uvs[1] = t2;
                    t->uvs[2] = t3;
                }
                this->faces.append(t);
            }
        }
        std::cout << "" << std::endl;
    }
    else
    {
        //An error loading the OBJ occurred!
        std::cout << errors << std::endl;
    }
}

void Mesh::create(){
    //Count the number of vertices for each face so we can get a count for the entire mesh
        std::vector<glm::vec3> vert_pos;
        std::vector<glm::vec3> vert_nor;
        std::vector<glm::vec3> vert_col;
        std::vector<GLuint> vert_idx;

        unsigned int index = 0;

        for(int i = 0; i < faces.size(); i++){
            Triangle* tri = faces[i];
            vert_pos.push_back(tri->points[0]); vert_nor.push_back(tri->normals[0]); vert_col.push_back(tri->material->base_color);
            vert_pos.push_back(tri->points[1]); vert_nor.push_back(tri->normals[1]); vert_col.push_back(tri->material->base_color);
            vert_pos.push_back(tri->points[2]); vert_nor.push_back(tri->normals[2]); vert_col.push_back(tri->material->base_color);
            vert_idx.push_back(index++);vert_idx.push_back(index++);vert_idx.push_back(index++);
        }

        count = vert_idx.size();
        int vert_count = vert_pos.size();

        bufIdx.create();
        bufIdx.bind();
        bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufIdx.allocate(vert_idx.data(), count * sizeof(GLuint));

        bufPos.create();
        bufPos.bind();
        bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufPos.allocate(vert_pos.data(), vert_count * sizeof(glm::vec3));

        bufCol.create();
        bufCol.bind();
        bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufCol.allocate(vert_col.data(), vert_count * sizeof(glm::vec3));

        bufNor.create();
        bufNor.bind();
        bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
        bufNor.allocate(vert_nor.data(), vert_count * sizeof(glm::vec3));
}

//This does nothing because individual triangles are not rendered with OpenGL; they are rendered all together in their Mesh.
void Triangle::create(){}
