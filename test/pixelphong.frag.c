varying vec3 normal;
varying vec3 light; //Use this for POINT light
varying vec3 vert;

void main (void)
{
	vec4  Color;
	
	//Colors
	vec4 ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	vec4 diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	vec4 specular = gl_FrontMaterial.specular * gl_LightSource[0].specular;
	
	//Basic shading
	vec3 N = normalize( normal );
	//vec3 L = normalize( gl_LightSource[0].position ).xyz; //Use this for DIR light
	vec3 L = normalize( light ); //Use this for POINT light
	float NdotL = max( dot(N,L), 0.0 );
	Color = diffuse * NdotL;
	Color.a = diffuse.a;
  
	//Specular term
	if (NdotL > 0.0)
	{
		//Phong specular coefficient
		vec3 E = - normalize( vert );
		vec3 R = normalize( 2.0 * dot(L,N) * N  - L );
		float RdotE = max( dot (R,E), 0.0 );
		float specCoeff = pow( RdotE, gl_FrontMaterial.shininess );
		Color += specular * specCoeff;
	}
	
	gl_FragColor = Color;
}
