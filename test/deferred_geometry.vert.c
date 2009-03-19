varying vec3 normal;
varying vec4 point;

void main (void)
{
	normal = normalize( gl_NormalMatrix * gl_Normal );
	point = gl_ModelViewMatrix * gl_Vertex;
  gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;
}
