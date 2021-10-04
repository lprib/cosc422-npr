#include "triangle_mesh.h"

#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "shaders.h"
using namespace std;

// Callback function for special keyboard events
void special(int key, int x, int y) { mesh_special(key); }

// Callback function for keyboard events
void keyboard(unsigned char key, int x, int y) { mesh_keyboard(key); }

// The main display callback function
void display() { mesh_display(); }

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

    mesh_init("assets/Homer.off");
    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
