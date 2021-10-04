#include "triangle_mesh.h"

#define _USE_MATH_DEFINES
#include <GL/glew.h>

#include <GL/freeglut.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include "shaders.h"

typedef OpenMesh::TriMesh_ArrayKernelT<> TriangleMesh;
static TriangleMesh mesh;
static float modelScale;
static float xc, yc, zc;
static float rotn_x = 0.0, rotn_y = 0.0;
static GLuint vaoID;
static glm::mat4 view, projView;
static int num_Elems;
static bool wireframe = false;
static const float DEG_TO_RAD = M_PI / 180.0f;

#define UNIFORMS X(mvp_matrix) X(mv_matrix) X(normal_matrix) X(light_pos) X(is_wireframe)
#define X(name) GLuint name##_uniform;
UNIFORMS
#undef X

static void
getBoundingBox(float* xmin, float* xmax, float* ymin, float* ymax, float* zmin, float* zmax);

void mesh_init(const char* filename)
{
    float xmin, xmax, ymin, ymax, zmin, zmax;

    if (!OpenMesh::IO::read_mesh(mesh, filename)) {
        std::cerr << "Mesh file read error.\n";
    }

    getBoundingBox(&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);

    xc = (xmin + xmax) * 0.5f;
    yc = (ymin + ymax) * 0.5f;
    zc = (zmin + zmax) * 0.5f;
    OpenMesh::Vec3f box = {xmax - xc, ymax - yc, zmax - zc};
    modelScale = 1.0 / box.max();

    GLint program = compile_program(
        "src/shaders/0_mesh.vert",
        "src/shaders/2_mesh.frag",
        NULL,
        NULL,
        "src/shaders/1_mesh.geom");
    glUseProgram(program);

    //==============Get vertex, normal data from mesh=========
    int num_verts = mesh.n_vertices();
    int num_faces = mesh.n_faces();
    float* vert_positions = new float[num_verts * 3];
    float* vert_norms = new float[num_verts * 3];
    num_Elems = num_faces * 3;
    short* elems = new short[num_Elems]; // Asumption: Triangle mesh

    if (!mesh.has_vertex_normals()) {
        mesh.request_face_normals();
        mesh.request_vertex_normals();
        mesh.update_normals();
    }

    // Use a vertex iterator to get vertex positions and vertex normal vectors
    int index = 0;
    for (TriangleMesh::VertexIter vertex_iter = mesh.vertices_begin();
         vertex_iter != mesh.vertices_end();
         vertex_iter++) {
        OpenMesh::VertexHandle vertex_handle = *vertex_iter; // Vertex handle
        TriangleMesh::Point pos = mesh.point(vertex_handle);
        TriangleMesh::Normal norm = mesh.normal(vertex_handle);
        for (int j = 0; j < 3; j++) {
            vert_positions[index] = pos[j];
            vert_norms[index] = norm[j];
            index++;
        }
    }

    // Use a face iterator to get the vertex indices for each face
    index = 0;
    for (TriangleMesh::FaceIter face_iter = mesh.faces_begin(); face_iter != mesh.faces_end();
         face_iter++) {
        OpenMesh::FaceHandle face_handle = *face_iter;
        for (TriangleMesh::FaceVertexIter face_vertex_iter = mesh.fv_iter(face_handle);
             face_vertex_iter.is_valid();
             face_vertex_iter++) {
            OpenMesh::VertexHandle vertex_handle = *face_vertex_iter; // Vertex handle
            elems[index] = vertex_handle.idx();
            index++;
        }
    }

    mesh.release_vertex_normals();

    //============== Load buffer data ==============
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    GLuint vboID[3];
    glGenBuffers(3, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_verts * 3, vert_positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0); // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_verts * 3, vert_norms, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1); // Vertex normal

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * num_faces * 3, elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Bind uniforms
#define X(name) name##_uniform = glGetUniformLocation(program, #name);
    UNIFORMS
#undef X

    glm::vec4 light = glm::vec4(5.0, 5.0, 10.0, 1.0);
    glm::mat4 proj;
    proj = glm::perspective(60.0f * DEG_TO_RAD, 1.0f, 2.0f, 10.0f);
    view = glm::lookAt(
        glm::vec3(0, 0, 4.0),
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0)); // view matrix
    projView = proj * view;
    glm::vec4 lightEye = view * light;
    glUniform4fv(light_pos_uniform, 1, &lightEye[0]);

    //============== Initialize OpenGL state ==============
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void mesh_display()
{
    glm::mat4 matrix = glm::mat4(1.0);
    matrix = glm::rotate(matrix, rotn_x * DEG_TO_RAD, glm::vec3(1.0, 0.0, 0.0)); // rotation about x
    matrix = glm::rotate(matrix, rotn_y * DEG_TO_RAD, glm::vec3(0.0, 1.0, 0.0)); // rotation about y
    matrix = glm::scale(matrix, glm::vec3(modelScale, modelScale, modelScale));
    matrix = glm::translate(matrix, glm::vec3(-xc, -yc, -zc));

    glm::mat4 viewMatrix = view * matrix; // The model-view matrix
    glUniformMatrix4fv(mv_matrix_uniform, 1, GL_FALSE, &viewMatrix[0][0]);

    glm::mat4 prodMatrix = projView * matrix; // The model-view-projection matrix
    glUniformMatrix4fv(mvp_matrix_uniform, 1, GL_FALSE, &prodMatrix[0][0]);

    glm::mat4 invMatrix = glm::inverse(viewMatrix); // Inverse of model-view matrix
    glUniformMatrix4fv(normal_matrix_uniform, 1, GL_TRUE, &invMatrix[0][0]);

    if (wireframe)
        glUniform1i(is_wireframe_uniform, 1);
    else
        glUniform1i(is_wireframe_uniform, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, num_Elems, GL_UNSIGNED_SHORT, NULL);

    glFlush();
}

void mesh_special(int key)
{
    if (key == GLUT_KEY_LEFT)
        rotn_y -= 5.0;
    else if (key == GLUT_KEY_RIGHT)
        rotn_y += 5.0;
    else if (key == GLUT_KEY_UP)
        rotn_x -= 5.0;
    else if (key == GLUT_KEY_DOWN)
        rotn_x += 5.0;
    glutPostRedisplay();
}

void mesh_keyboard(unsigned char key)
{
    if (key == 'w')
        wireframe = !wireframe;
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutPostRedisplay();
}

// Gets the min max values along x, y, z for scaling and centering the model in
// the view frustum
static void
getBoundingBox(float* xmin, float* xmax, float* ymin, float* ymax, float* zmin, float* zmax)
{
    TriangleMesh::VertexIter vertex_iter = mesh.vertices_begin();
    TriangleMesh::Point pmin, pmax;

    pmin = pmax = mesh.point(*vertex_iter);

    // Iterate over the mesh using a vertex iterator
    for (vertex_iter = mesh.vertices_begin() + 1; vertex_iter != mesh.vertices_end();
         vertex_iter++) {
        pmin.minimize(mesh.point(*vertex_iter));
        pmax.maximize(mesh.point(*vertex_iter));
    }
    *xmin = pmin[0];
    *ymin = pmin[1];
    *zmin = pmin[2];
    *xmax = pmax[0];
    *ymax = pmax[1];
    *zmax = pmax[2];
}