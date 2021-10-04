#include "triangle_mesh.h"

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



// Initialisation function for OpenMesh, shaders and OpenGL
void initialize()
{
    mesh_init("Homer.off");
}

// Callback function for special keyboard events
void special(int key, int x, int y)
{
    mesh_special(key);
}

// Callback function for keyboard events
void keyboard(unsigned char key, int x, int y)
{
    mesh_keyboard(key);
}

// The main display callback function
void display()
{
    mesh_display();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("COSC422 Assignment 2");
    glutInitContextVersion(4, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    if (glewInit() == GLEW_OK) {
        cout << "GLEW initialization successful! " << endl;
        cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
    } else {
        cerr << "Unable to initialize GLEW  ...exiting." << endl;
        exit(EXIT_FAILURE);
    }

    mesh_init("Homer.off");
    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
