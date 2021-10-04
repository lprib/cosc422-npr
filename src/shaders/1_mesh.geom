#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VERTEX_DATA {
    float diffTerm;
} verts[];

out FRAG_DATA {
    smooth float diffTerm;
} frag;

void main() {
    for(int i = 0; i < gl_in.length(); i++) {
        frag.diffTerm = verts[i].diffTerm;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
