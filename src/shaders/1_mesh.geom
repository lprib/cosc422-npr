#version 400

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices=32) out;

uniform mat4 normal_matrix;
uniform mat4 mv_matrix;

const float sil_offset_1 = 0.001;
const float sil_offset_2 = 0.02;
const float sil_overstep = 0.01;

const float crease_angle_thresh = 1.5;
const float crease_offset_1 = 0.001;
const float crease_offset_2 = 0.02;

in VERTEX_DATA {
    float diffuse;
    vec3 world_pos;
    vec3 mv_normal;
} verts[];

out FRAG_DATA {
    flat bool is_outline;
    flat bool is_crease;
    smooth float diffuse;
} frag;

vec4 get_face_normal(vec3 p2, vec3 p1, vec3 p0, mat4 matrix) {
    return normalize(matrix * vec4(normalize(cross(p1- p0, p2 - p0)), 0.));
}

void main() {

    vec4 main_normal = get_face_normal(verts[0].world_pos, verts[2].world_pos, verts[4].world_pos, normal_matrix);
    vec4 main_normal_world = get_face_normal(verts[0].world_pos, verts[2].world_pos, verts[4].world_pos, mat4(1.0));

    // iterate over adjacent faces
    for(int f = 0; f < 3; f ++) {
        int adj_idx_0 = 2*f;
        int adj_idx_1 = 2*f+1;
        int adj_idx_2 = f == 2 ? 0 : 2*f+2;

        // silhouette
        vec4 face_normal = get_face_normal(verts[adj_idx_0].world_pos, verts[adj_idx_1].world_pos, verts[adj_idx_2].world_pos, normal_matrix);
        if(main_normal.z >= 0. && face_normal.z <= 0.) {
            vec4 v = normalize(main_normal + face_normal);
            vec4 A = gl_in[adj_idx_0].gl_Position;
            vec4 B = gl_in[adj_idx_2].gl_Position;
            vec4 An = vec4(verts[adj_idx_0].mv_normal, 0.);
            vec4 Bn = vec4(verts[adj_idx_2].mv_normal, 0.);
            vec4 overstep = normalize(B-A)*sil_overstep;
            frag.is_outline = true;
            frag.is_crease = false;
            gl_Position = A + sil_offset_1 * An - overstep;
            EmitVertex();
            gl_Position = A + sil_offset_2 * An - overstep;
            EmitVertex();
            gl_Position = B + sil_offset_1 * Bn + overstep;
            EmitVertex();
            gl_Position = B + sil_offset_2 * Bn + overstep;
            EmitVertex();
            EndPrimitive();
        }

        // crease
        vec4 face_normal_world = get_face_normal(verts[adj_idx_0].world_pos, verts[adj_idx_1].world_pos, verts[adj_idx_2].world_pos, mat4(1.0));
        if(dot(main_normal_world, face_normal_world) <= cos(crease_angle_thresh)) {
            vec4 A = gl_in[adj_idx_0].gl_Position;
            vec4 B = gl_in[adj_idx_2].gl_Position;

            vec4 u = normalize(B - A);
            vec4 w = vec4(normalize(cross(u.xyz, v.xyz)), 0.);
            vec4 v = normalize(main_normal + face_normal);

            frag.is_outline = false;
            frag.is_crease = true;
            gl_Position = A + crease_offset_1*v + crease_offset_2*w;
            EmitVertex();
            gl_Position = A + crease_offset_1*v - crease_offset_2*w;
            EmitVertex();
            gl_Position = B + crease_offset_1*v + crease_offset_2*w;
            EmitVertex();
            gl_Position = B + crease_offset_1*v - crease_offset_2*w;
            EmitVertex();
            EndPrimitive();
        }
    }

    // Draw main triangle
    frag.is_outline = false;
    frag.is_crease = false;
    for(int i = 0; i < 6; i+=2) {
        frag.diffuse = verts[i].diffuse;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
