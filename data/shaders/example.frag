varying lowp vec4 v_color;
varying lowp vec2 v_texCoord;

uniform sampler2D tex0;

void main()
{
	lowp vec4 colour = texture2D(tex0, v_texCoord);
    gl_FragColor = colour + vec4(1.0, 0.0, 0.0, 1.0);
}
