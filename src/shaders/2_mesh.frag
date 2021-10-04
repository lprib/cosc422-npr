#version 400

uniform int wireMode;

in FRAG_DATA {
    smooth float diffTerm;
} frag;

void main() 
{
   if(wireMode == 1)    //Wireframe
       gl_FragColor = vec4(0, 0, 1, 1);
   else			//Fill + lighting
       gl_FragColor = frag.diffTerm * vec4(0, 1, 1, 1);
}
