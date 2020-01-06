uniform mat4 u_projection; 
attribute vec2 a_texCoord; 
attribute vec2 a_position; 
varying vec2 v_texCoord; 

void main() { 
    v_texCoord = a_texCoord; 
    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
    gl_PointSize = 1.0;
} 
