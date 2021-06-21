#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif

uniform  mat4 uModelView;
uniform  mat3 uNormalRotation;
uniform  mat4 uModelViewProjection;
attribute  vec3 vaPosition;
attribute  vec3 vaNormal;
//attribute  vec2 vaTexcoord;
//varying  vec3 fNormal;
//varying  vec3 fPosition;
//varying  vec2 fTexcoord;

void main()
{
    //fTexcoord = vaTexcoord;
    //vec3 norm = uNormalRotation*vaNormal;
    //fNormal = normalize(norm);
    vec4 vposition = vec4(vaPosition,1.0);
    //vec4 position = uModelView*vposition;
    //fPosition = position.xyz;
    gl_Position = uModelViewProjection*vposition;
}
