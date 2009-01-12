varying vec3 normal;
varying vec3 point;
varying vec3 light; //Use this for POINT light

void main (void)
{
	normal = normalize( gl_NormalMatrix * gl_Normal );
	point = vec3( gl_ModelViewMatrix * gl_Vertex );
  
	//Use this for POINT light
	light = gl_LightSource[0].position.xyz - point;
	
  gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;
}
