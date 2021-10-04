#version 400

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices=32) out;

uniform mat4 normal_matrix;
uniform mat4 mv_matrix;

const float sil_offset_1 = 0.001;
const float sil_offset_2 = 0.02;
const float sil_overstep = 0.01;

in VERTEX_DATA {
    float diffuse;
    vec3 world_pos;
    vec3 mv_normal;
} verts[];

out FRAG_DATA {
    smooth float diffuse;
    smooth vec3 debug_color;
} frag;

vec4 get_face_normal(vec3 p2, vec3 p1, vec3 p0) {
    return normalize(normal_matrix * vec4(normalize(cross(p1- p0, p2 - p0)), 0.));
}

void main() {

    bool is_edge = false;
    // Silhouette edges:
    vec4 main_normal = get_face_normal(verts[0].world_pos, verts[2].world_pos, verts[4].world_pos);
    // iterate over adjacent faces
    for(int f = 0; f < 3; f ++) {
        int adj_idx_0 = 2*f;
        int adj_idx_1 = 2*f+1;
        int adj_idx_2 = f == 2 ? 0 : 2*f+2;
        vec4 face_normal = get_face_normal(verts[adj_idx_0].world_pos, verts[adj_idx_1].world_pos, verts[adj_idx_2].world_pos);
        if(main_normal.z >= 0. && face_normal.z <= 0.) {
            is_edge = true;
            vec4 v = normalize(main_normal + face_normal);
            vec4 A = gl_in[adj_idx_0].gl_Position;
            vec4 B = gl_in[adj_idx_2].gl_Position;
            vec4 An = vec4(verts[adj_idx_0].mv_normal, 0.);
            vec4 Bn = vec4(verts[adj_idx_2].mv_normal, 0.);
            vec4 overstep = normalize(B-A)*sil_overstep;
            frag.diffuse = 0.;
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
    }

    // Draw main triangle
    for(int i = 0; i < 6; i+=2) {
        // frag.diffuse = is_edge? 1.0:verts[i].diffuse;
        frag.diffuse = verts[i].diffuse;
        // frag.debug_color = main_normal;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
