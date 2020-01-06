precision mediump float; 
uniform sampler2D u_texture; 
varying vec2 v_texCoord; 

void main() { 
    vec4 colour = texture2D(u_texture, v_texCoord); 
    gl_FragColor = vec4(vec3(1.0) - colour.rgb, colour.a);
} 
