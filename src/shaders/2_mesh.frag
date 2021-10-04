#version 400

uniform int is_wireframe;

in FRAG_DATA {
    flat bool is_outline;
    flat bool is_crease;
    smooth float diffuse;
} frag;

void main() 
{
    if(is_wireframe == 1) {
        if(frag.is_outline || frag.is_crease) {
            discard;
        }
        gl_FragColor = vec4(0, 0, 1, 1);
    }
    else {
        if(frag.is_outline) {
            gl_FragColor = vec4(0, 0, 0, 1);
        } else if(frag.is_crease) {
            gl_FragColor = vec4(1, 0, 0, 1);
        } else {
            gl_FragColor = frag.diffuse * vec4(0, 1, 1, 1);
        }
        // gl_FragColor = vec4(frag.debug_color, 1.);
    }
}
