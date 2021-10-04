#version 400

uniform int is_wireframe;

in FRAG_DATA {
    smooth float diffuse;
} frag;

void main() 
{
    if(is_wireframe == 1) {
        gl_FragColor = vec4(0, 0, 1, 1);
    }
    else {
        gl_FragColor = frag.diffuse * vec4(0, 1, 1, 1);
    }
}
