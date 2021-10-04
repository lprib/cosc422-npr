#version 400

uniform int is_wireframe;

in FRAG_DATA {
    smooth float diffuse;
    smooth vec3 debug_color;
} frag;

void main() 
{
    if(is_wireframe == 1) {
        gl_FragColor = vec4(0, 0, 1, 1);
    }
    else {
        gl_FragColor = frag.diffuse * vec4(0, 1, 1, 1);
        // gl_FragColor = vec4(frag.debug_color, 1.);
    }
}
