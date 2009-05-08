#begin Bloom_VertexSource
void main (void)
{
  //We assume vertex coords to be in [-1,1] interval
  //for a full-screen quad
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end


#begin Bloom_FragmentSource
uniform sampler2D samplerAccum;

vec3 toneMap (vec3 color)
{
  float lumAvg = 0.8;
  float lumKey = 0.5;
  float lumMax = 1.0;
	
	//Calculate the luminance of the color
  vec3 lumCoeffs = vec3( 0.299, 0.587, 0.114 );
	float luminance = dot( color, lumCoeffs );
	
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
  vec4 accumTexel = texture2D( samplerAccum, gl_TexCoord[0].xy );
  
  vec3 cutoff = vec3(1.0);
  gl_FragColor = vec4( max( toneMap(accumTexel.rgb) - cutoff, 0.0 ), 1.0 );

  //vec4 cutoff = vec4(1.0);
  //gl_FragColor = max( accumTexel - cutoff, 0.0 );

  //gl_FragColor = accumTexel;
}
#end
