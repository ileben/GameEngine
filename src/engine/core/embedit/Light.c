#begin Light_VS

void main (void)
{
  //We expect texture coords to be in [0,1] interval
  //and vertex coords to be in [-1,1] interval.
  gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;
}

#end

///////////////////////////////////////////////////////////////////////////

#begin SpotLight_Func

uniform float attStart;
uniform float attEnd;

float getLightCoeff (vec3 P, vec3 lightP, vec3 L)
{
  //Attenuation falloff
  float dist = length( P - lightP );
  float attCoeff = clamp( (attEnd - dist) / (attEnd - attStart), 0.0, 1.0 );
  if (attCoeff <= 0.0) return 0.0;

  //Spotlight check
  vec3 S = normalize( gl_LightSource[0].spotDirection );
  float cosOuter = gl_LightSource[0].spotCosCutoff;
  float cosInner = gl_LightSource[0].spotExponent;
  float LdotS = max( dot(-L,S), 0.0 );
  if (LdotS < cosOuter) return 0.0;

  //Spotlight falloff
  float spotCoeff = 1.0;
  if (cosInner - cosOuter > 0.0 && LdotS < cosInner)
    spotCoeff = (LdotS - cosOuter) / (cosInner - cosOuter);

  //Full intensity in center
  return spotCoeff * attCoeff;
}

#end

///////////////////////////////////////////////////////////////////////////

#begin PyramidLight_Func

uniform float attStart;
uniform float attEnd;

float getLightCoeff (vec3 P, vec3 lightP, vec3 L)
{
  //Attenuation falloff
  float dist = length( P - lightP );
  float attCoeff = clamp( (attEnd - dist) / (attEnd - attStart), 0.0, 1.0 );
  return attCoeff;
}

#end

///////////////////////////////////////////////////////////////////////////

#begin Light_FS

uniform sampler2D samplerNormal;
uniform sampler2D samplerColor;
uniform sampler2D samplerSpec;
uniform sampler2D samplerParams;
uniform sampler2D samplerShadow;

uniform float attEnd;
uniform int castShadow;
uniform vec2 winSize;

float getLightCoeff (vec3 P, vec3 lightP, vec3 L);

void main (void)
{
  //Input data
  vec2 texCoord = vec2( gl_FragCoord.x / winSize.x, gl_FragCoord.y / winSize.y );
  vec4 normalTexel = texture2D( samplerNormal, texCoord );
  vec4 colorTexel = texture2D( samplerColor, texCoord );
  vec4 specTexel = texture2D( samplerSpec, texCoord );
  vec4 paramTexel = texture2D( samplerParams, texCoord );
  float luminosity = colorTexel.a * 2.0;

  //Calculate eye-space point from eye-z coord
  float eyeZ =  normalTexel.w;
  vec4 clipPoint = vec4(
    eyeZ * ((gl_FragCoord.x / winSize.x) * 2.0 - 1.0),
    eyeZ * ((gl_FragCoord.y / winSize.y) * 2.0 - 1.0), 1.0,1.0 );
  vec3 point = (gl_ProjectionMatrixInverse * clipPoint).xyz;
  point.z = eyeZ;

  //Get normal and direction to light
  vec3 normal = normalTexel.xyz;
  vec3 lightPoint = gl_LightSource[0].position.xyz;
  vec3 light = lightPoint - point;

  //Get light properties
  vec3 diffuse = colorTexel.rgb * gl_LightSource[0].diffuse.rgb;
  vec3 specular = specTexel.rgb * gl_LightSource[0].specular.rgb;
  float shininess = specTexel.a * 128.0;
  
  //Output color
	vec3 Color = vec3( 0.0, 0.0, 0.0 );

  //Input vectors
	vec3 N = normalize( normal );
	vec3 L = normalize( light );
  
	//Basic shading
	float NdotL = max( dot(N,L), 0.0 );

  float diffuseCoeff = 0.0;
  float specCoeff = 0.0;
  if (NdotL > 0.0)
  {
    //Light-specific coefficient
    float lightCoeff = getLightCoeff( point, lightPoint, L );
    if (lightCoeff > 0.0)
    {
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
        vec4 shadowTexel = texture2D( samplerShadow, shadowCoord.xy );
        if (shadowCoord.z > shadowTexel.r)
          shadowCoeff = 0.2;
        /*
        //Percentage-closer filter
        float px = 1.0 / 1024;
        float weight = 1.0 / (2 * 2);
        float shadow = 0.0;
        for (int x=-1; x<=0; ++x) {
          for (int y=-1; y<=0; ++y) {
            vec4 shadowTexel = texture2D( samplerShadow, shadowCoord.xy + vec2( x*px, y*px ));
            if (shadowCoord.z < shadowTexel.r)
              shadow += weight; }}
        shadowCoeff = 0.2 + shadow * 0.8;*/
      }

      //Diffuse color term (avoid going over the ambient term)
      //diffuseCoeff = NdotL * shadowCoeff * spotCoeff;
      float luminosityCoeff = (1.0 - min( luminosity, 1.0 ));
      diffuseCoeff = NdotL * luminosityCoeff * shadowCoeff * lightCoeff;
      Color += diffuse * diffuseCoeff;
      
	    //Phong specular coefficient
	    vec3 E = - normalize( point );
	    vec3 R = normalize( 2.0 * dot(L,N) * N  - L );
	    float shineCoeff = max( dot (R,E), 0.0 );
	    shineCoeff = pow( shineCoeff, shininess );

      //specCoeff = shineCoeff * shadowCoeff * spotCoeff;
      specCoeff = paramTexel[0] * shineCoeff * shadowCoeff * lightCoeff;
	    Color += specular * specCoeff;
    }
  }
	
  gl_FragColor = vec4( Color, diffuseCoeff + specCoeff );
}

#end
