#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VERTEX_DATA {
    float diffuse;
} verts[];

out FRAG_DATA {
    smooth float diffuse;
} frag;

void main() {
    for(int i = 0; i < gl_in.length(); i++) {
        frag.diffuse = verts[i].diffuse;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
