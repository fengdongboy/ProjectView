#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform  mat4 uModelViewProjection;

attribute vec3 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;

//! [0]
void main()
{

	vec4 vposition = vec4(a_position, 1.0);
    //vec4 position = uModelView*vposition;
    //fPosition = position.xyz;
    gl_Position = uModelViewProjection*vposition;
	v_texcoord = a_texcoord;
}
//! [0]
