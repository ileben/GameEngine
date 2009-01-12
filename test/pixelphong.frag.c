varying vec3 normal;
varying vec3 light; //Use this for POINT light
varying vec3 point;

void main (void)
{
	//Input colors
	vec4 ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	vec4 diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	vec4 specular = gl_FrontMaterial.specular * gl_LightSource[0].specular;
	vec4 Color = ambient;

  //Input vectors
	vec3 N = normalize( normal );
	//vec3 L = normalize( gl_LightSource[0].position ).xyz; //Use this for DIR light
	vec3 L = normalize( light ); //Use this for POINT light
  
	//Basic shading
	float NdotL = max( dot(N,L), 0.0 );
  if (NdotL > 0.0)
  {
    //Spotlight check
    vec3 S = normalize( gl_LightSource[0].spotDirection );
    float cosOuter = gl_LightSource[0].spotCosCutoff;
    float cosInner = gl_LightSource[0].spotExponent;
    float LdotS = max( dot(-L,S), 0.0 );
    if (LdotS >= cosOuter)
    {
      //Spotlight falloff
      float spotCoeff = 1.0;
      if (cosInner - cosOuter > 0.0 && LdotS < cosInner)
        spotCoeff = (LdotS - cosOuter) / (cosInner - cosOuter);
      
      //Diffuse color term
	    Color += diffuse * NdotL * spotCoeff;
	    Color.a = diffuse.a;
      
		  //Phong specular coefficient
		  vec3 E = - normalize( point );
		  vec3 R = normalize( 2.0 * dot(L,N) * N  - L );
		  float specCoeff = max( dot (R,E), 0.0 );
		  specCoeff = pow( specCoeff, gl_FrontMaterial.shininess );
		  Color += specular * specCoeff * spotCoeff;
	  }
  }
	
	gl_FragColor = Color;
}
