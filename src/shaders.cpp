#include "shaders.h"

#include <GL/freeglut.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint load_shader(GLenum shader_type, const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening shader file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_string = (char*)malloc(file_size + 1);
    fread(file_string, 1, file_size, file);
    file_string[file_size] = 0;
    fclose(file);

    const char* file_string_const = (const char*)file_string;

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &file_string_const, NULL);
    glCompileShader(shader);
    GLint status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
        GLchar* info_log = (GLchar*)malloc(sizeof(GLchar) * (info_log_len + 1));
        memset(info_log, 0, info_log_len + 1);
        glGetShaderInfoLog(shader, info_log_len, NULL, info_log);
        printf("Comile error in shader %s: %s\n", filename, info_log);
        free(info_log);
        exit(EXIT_FAILURE);
    }

    return shader;
}

// Returns the program id
GLint compile_program(
    const char* vert_filename, const char* frag_filename, const char* tess_control_filename,
    const char* tess_eval_filename, const char* geom_filename)
{
    GLuint program = glCreateProgram();
    GLuint shader;

    if (vert_filename != NULL) {
        shader = load_shader(GL_VERTEX_SHADER, vert_filename);
        glAttachShader(program, shader);
    }
    if (frag_filename != NULL) {
        shader = load_shader(GL_FRAGMENT_SHADER, frag_filename);
        glAttachShader(program, shader);
    }
    if (tess_control_filename != NULL) {
        shader = load_shader(GL_TESS_CONTROL_SHADER, tess_control_filename);
        glAttachShader(program, shader);
    }
    if (tess_eval_filename != NULL) {
        shader = load_shader(GL_TESS_EVALUATION_SHADER, tess_eval_filename);
        glAttachShader(program, shader);
    }
    if (geom_filename != NULL) {
        shader = load_shader(GL_GEOMETRY_SHADER, geom_filename);
        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_log_len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
        GLchar* info_log = (GLchar*)malloc(sizeof(GLchar) * (info_log_len + 1));
        memset(info_log, 0, info_log_len + 1);
        glGetProgramInfoLog(program, info_log_len, NULL, info_log);
        fprintf(stderr, "Linker failure: %s\n", info_log);
        free(info_log);
        exit(EXIT_FAILURE);
    }

    return program;
}
