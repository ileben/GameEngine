uniform sampler2D samplerNormal;
uniform sampler2D samplerColor;
uniform sampler2D samplerSpec;
uniform sampler2D samplerParams;
uniform sampler2D samplerShadow;

uniform int castShadow;
uniform vec2 winSize;

void main (void)
{
  //Input data
  vec2 texCoord = vec2( gl_FragCoord.x / winSize.x, gl_FragCoord.y / winSize.y );
  vec4 normalTexel = texture2D( samplerNormal, texCoord );
  vec4 colorTexel = texture2D( samplerColor, texCoord );
  vec4 specTexel = texture2D( samplerSpec, texCoord );
  vec4 paramTexel = texture2D( samplerParams, texCoord );

  //Calculate eye-space point from eye-z coord
  Float eyeZ =  normalTexel.w;
  vec4 clipPoint = vec4(
    eyeZ * ((gl_FragCoord.x / winSize.x) * 2.0 - 1.0),
    eyeZ * ((gl_FragCoord.y / winSize.y) * 2.0 - 1.0), 1.0,1.0 );
  vec3 point = (gl_ProjectionMatrixInverse * clipPoint).xyz;
  point.z = eyeZ;

  vec3 normal = normalTexel.xyz;
  vec3 diffuse = colorTexel.rgb * gl_LightSource[0].diffuse.rgb;
  vec3 specular = specTexel.rgb * gl_LightSource[0].specular.rgb;
  float shininess = specTexel.a * 128.0;

  vec3 light = gl_LightSource[0].position.xyz - point;
  
  //Output color
	vec3 Color = vec3( 0.0, 0.0, 0.0 );

  //Check if light is casting shadows
  float shadowCoeff = 1.0;
  if (castShadow == 1)
  {
    //Transform point into light clip space
    vec4 pointEye = vec4( point, 1.0 );
    vec4 shadowClip = gl_TextureMatrix[0] * pointEye;

    //Perspective division and bias on shadow coordinate
    vec3 shadowCoord = shadowClip.xyz / shadowClip.w;
    shadowCoord = shadowCoord * 0.5 + vec3(0.5,0.5,0.5);

    //Sample the light-nearest depth and compare to light-vertex depth
    vec4 shadowValue = texture2D( samplerShadow, shadowCoord.xy );
    if (shadowCoord.x >= 0.0 && shadowCoord.x <= 1.0 &&
        shadowCoord.y >= 0.0 && shadowCoord.y <= 1.0)
      if (shadowCoord.z > shadowValue.r)
        { shadowCoeff = 0.0; }
  }

  //Input vectors
	vec3 N = normalize( normal );
	vec3 L = normalize( light );
  
	//Basic shading
	float NdotL = max( dot(N,L), 0.0 );

  //Cell shading
  //if (paramTexel.x > 0.5)
    //NdotL = floor( NdotL * 2.0 + 0.5 ) / 2.0;

  float diffuseCoeff = 0.0;
  float specCoeff = 0.0;
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
      
      //Diffuse color term (avoid going over the ambient term)
      //diffuseCoeff = NdotL * shadowCoeff * spotCoeff;
      diffuseCoeff = NdotL * (1.0 - colorTexel.a) * shadowCoeff * spotCoeff;
      Color += diffuse * diffuseCoeff;
      
		  //Phong specular coefficient
		  vec3 E = - normalize( point );
		  vec3 R = normalize( 2.0 * dot(L,N) * N  - L );
		  float shineCoeff = max( dot (R,E), 0.0 );
		  shineCoeff = pow( shineCoeff, shininess );

      //Cell shading
      //if (paramTexel.x > 0.5)
        //specCoeff = floor( specCoeff + 0.5 ) * 0.5;

      specCoeff = paramTexel.g * shineCoeff * shadowCoeff * spotCoeff;
		  Color += specular * specCoeff;
	  }
  }
	
	gl_FragColor = vec4( Color, diffuseCoeff + specCoeff );
  //gl_FragColor = vec4( Color, diffuseCoeff );
}
