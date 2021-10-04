#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 mv_matrix;
uniform mat4 mvp_matrix;
uniform mat4 normal_matrix;
uniform vec4 light_pos;
 
out VERTEX_DATA {
	float diffuse;
} vertex;

void main() {
	vec4 mv_position = mv_matrix * vec4(position, 1);
	vec3 mv_normal = normalize((normal_matrix * vec4(normal, 0)).xyz);
	vec3 light_vec = normalize(light_pos.xyz - mv_position.xyz); 

	vertex.diffuse = max(dot(light_vec, mv_normal), 0.2);

	gl_Position = mvp_matrix * vec4(position, 1);
}