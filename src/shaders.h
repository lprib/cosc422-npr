#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>
#include <GL/freeglut.h>

GLuint load_shader(GLenum shader_type, const char* filename);

GLint compile_program(
    const char* vert_filename, const char* frag_filename, const char* tess_control_filename,
    const char* tess_eval_filename, const char* geom_filename);

#endif
