#begin Final_VertexSource
void main (void)
{
  //We assume vertex coords to be in [-1,1] interval
  //for a full-screen quad
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Final_FragmentSource
uniform sampler2D samplerEffects;
uniform sampler2D samplerColor;

vec3 toneMap (vec3 color)
{
  float lumAvg = 0.8;
  float lumKey = 0.5;
  float lumMax = 1.0;
	
	//Calculate the luminance of the color
  //(add small value to avoid in final RGB conversion)
  vec3 lumCoeffs = vec3( 0.299, 0.587, 0.114 );
	float luminance = dot( color, lumCoeffs ) + 0.001;
	
	//Apply the modified operator (Eq. 4)
	float lumScaled = luminance * lumKey / lumAvg;
	float lumCompressed = (lumScaled * (1.0 + (lumScaled / (lumMax * lumMax)))) / (1.0 + lumScaled);

  //Convert back to RGB
  vec3 outColor; float saturation = 0.8;
  outColor.r = pow( color.r / luminance, saturation) * lumCompressed;
  outColor.g = pow( color.g / luminance, saturation ) * lumCompressed;
  outColor.b = pow( color.b / luminance, saturation ) * lumCompressed;
  return outColor;

	//return lumCompressed * color;
}

void main (void)
{
  vec3 Color = vec3( 0.0, 0.0, 0.0 );

  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  Color += toneMap( colorTexel.rgb );

  vec4 effectsTexel = texture2D( samplerEffects, gl_TexCoord[0].xy );
  Color += effectsTexel.rgb;
  
  
  gl_FragColor = vec4( Color, 1.0 );
}
#end
