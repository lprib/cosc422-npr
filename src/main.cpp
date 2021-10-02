#define _USE_MATH_DEFINES // for C++
#include <GL/glew.h>

#include <GL/freeglut.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <cmath>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "shaders.h"
using namespace std;

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
MyMesh mesh;
float modelScale;
float xc, yc, zc;
float rotn_x = 0.0, rotn_y = 0.0;
GLuint vaoID;
GLuint mvpMatrixLoc, mvMatrixLoc, norMatrixLoc, lgtLoc, wireLoc;
glm::mat4 view, projView;
int num_Elems;
bool wireframe = false;

// Gets the min max values along x, y, z for scaling and centering the model in
// the view frustum
void getBoundingBox(float& xmin, float& xmax, float& ymin, float& ymax, float& zmin, float& zmax)
{
    MyMesh::VertexIter vit = mesh.vertices_begin();
    MyMesh::Point pmin, pmax;

    pmin = pmax = mesh.point(*vit);

    // Iterate over the mesh using a vertex iterator
    for (vit = mesh.vertices_begin() + 1; vit != mesh.vertices_end(); vit++) {
        pmin.minimize(mesh.point(*vit));
        pmax.maximize(mesh.point(*vit));
    }
    xmin = pmin[0];
    ymin = pmin[1];
    zmin = pmin[2];
    xmax = pmax[0];
    ymax = pmax[1];
    zmax = pmax[2];
}

// Initialisation function for OpenMesh, shaders and OpenGL
void initialize()
{
    float xmin, xmax, ymin, ymax, zmin, zmax;
    float CDR = M_PI / 180.0f;

    //============= Load mesh ==================
    if (!OpenMesh::IO::read_mesh(mesh, "Homer.off")) {
        cerr << "Mesh file read error.\n";
    }
    getBoundingBox(xmin, xmax, ymin, ymax, zmin, zmax);

    xc = (xmin + xmax) * 0.5f;
    yc = (ymin + ymax) * 0.5f;
    zc = (zmin + zmax) * 0.5f;
    OpenMesh::Vec3f box = {xmax - xc, ymax - yc, zmax - zc};
    modelScale = 1.0 / box.max();

    GLint program = compile_program("src/mesh.vert", "src/mesh.frag", NULL, NULL, NULL);
    glUseProgram(program);

    //==============Get vertex, normal data from mesh=========
    int num_verts = mesh.n_vertices();
    int num_faces = mesh.n_faces();
    float* vertPos = new float[num_verts * 3];
    float* vertNorm = new float[num_verts * 3];
    num_Elems = num_faces * 3;
    short* elems = new short[num_Elems]; // Asumption: Triangle mesh

    if (!mesh.has_vertex_normals()) {
        mesh.request_face_normals();
        mesh.request_vertex_normals();
        mesh.update_normals();
    }

    MyMesh::VertexIter vit;      // A vertex iterator
    MyMesh::FaceIter fit;        // A face iterator
    MyMesh::FaceVertexIter fvit; // Face-vertex iterator
    OpenMesh::VertexHandle verH1, verH2;
    OpenMesh::FaceHandle facH;
    MyMesh::Point pos;
    MyMesh::Normal norm;
    int indx;

    // Use a vertex iterator to get vertex positions and vertex normal vectors
    indx = 0;
    for (vit = mesh.vertices_begin(); vit != mesh.vertices_end(); vit++) {
        verH1 = *vit; // Vertex handle
        pos = mesh.point(verH1);
        norm = mesh.normal(verH1);
        for (int j = 0; j < 3; j++) {
            vertPos[indx] = pos[j];
            vertNorm[indx] = norm[j];
            indx++;
        }
    }

    // Use a face iterator to get the vertex indices for each face
    indx = 0;
    for (fit = mesh.faces_begin(); fit != mesh.faces_end(); fit++) {
        facH = *fit;
        for (fvit = mesh.fv_iter(facH); fvit.is_valid(); fvit++) {
            verH2 = *fvit; // Vertex handle
            elems[indx] = verH2.idx();
            indx++;
        }
    }

    mesh.release_vertex_normals();

    //============== Load buffer data ==============
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    GLuint vboID[3];
    glGenBuffers(3, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_verts * 3, vertPos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0); // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_verts * 3, vertNorm, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1); // Vertex normal

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short) * num_faces * 3, elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

    //============== Create uniform variables ==============
    mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
    mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
    norMatrixLoc = glGetUniformLocation(program, "norMatrix");
    wireLoc = glGetUniformLocation(program, "wireMode");
    lgtLoc = glGetUniformLocation(program, "lightPos");
    glm::vec4 light = glm::vec4(5.0, 5.0, 10.0, 1.0);
    glm::mat4 proj;
    proj = glm::perspective(60.0f * CDR, 1.0f, 2.0f,
                            10.0f); // perspective projection matrix
    view = glm::lookAt(
        glm::vec3(0, 0, 4.0),
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0)); // view matrix
    projView = proj * view;
    glm::vec4 lightEye = view * light;
    glUniform4fv(lgtLoc, 1, &lightEye[0]);

    //============== Initialize OpenGL state ==============
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Callback function for special keyboard events
void special(int key, int x, int y)
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

// Callback function for keyboard events
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'w')
        wireframe = !wireframe;
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutPostRedisplay();
}

// The main display callback function
void display()
{
    float CDR = M_PI / 180.0;
    glm::mat4 matrix = glm::mat4(1.0);
    matrix = glm::rotate(matrix, rotn_x * CDR, glm::vec3(1.0, 0.0, 0.0)); // rotation about x
    matrix = glm::rotate(matrix, rotn_y * CDR, glm::vec3(0.0, 1.0, 0.0)); // rotation about y
    matrix = glm::scale(matrix, glm::vec3(modelScale, modelScale, modelScale));
    matrix = glm::translate(matrix, glm::vec3(-xc, -yc, -zc));

    glm::mat4 viewMatrix = view * matrix; // The model-view matrix
    glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &viewMatrix[0][0]);

    glm::mat4 prodMatrix = projView * matrix; // The model-view-projection matrix
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &prodMatrix[0][0]);

    glm::mat4 invMatrix = glm::inverse(viewMatrix); // Inverse of model-view
                                                    // matrix
    glUniformMatrix4fv(norMatrixLoc, 1, GL_TRUE, &invMatrix[0][0]);

    if (wireframe)
        glUniform1i(wireLoc, 1);
    else
        glUniform1i(wireLoc, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, num_Elems, GL_UNSIGNED_SHORT, NULL);

    glFlush();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Mesh Viewer (OpenMesh)");
    glutInitContextVersion(4, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    if (glewInit() == GLEW_OK) {
        cout << "GLEW initialization successful! " << endl;
        cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
    } else {
        cerr << "Unable to initialize GLEW  ...exiting." << endl;
        exit(EXIT_FAILURE);
    }

    initialize();
    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
