
#begin Bloom_VS

void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}

#end

////////////////////////////////////////////////////////////////////////////////////////

#begin ToneMap_Func

vec3 RGBtoYxy (vec3 RGB)
{
  const mat3 RGB2XYZ = mat3(
    0.5141364,  0.265068,   0.0241188,
    0.3238786,  0.67023428, 0.1228178,
    0.16036376, 0.06409157, 0.84442666 );

  vec3 XYZ = max( RGB2XYZ * RGB, vec3(0.0001) );

  vec3 Yxy;
  Yxy[0] = XYZ.y;
  Yxy[1] = XYZ.x / (XYZ.x + XYZ.y + XYZ.z);
  Yxy[2] = XYZ.y / (XYZ.x + XYZ.y + XYZ.z);

  return Yxy;
}

vec3 YxyToRGB (vec3 Yxy)
{
  const mat3 XYZ2RGB  = mat3(
     2.5651, -1.0217,  0.0753,
    -1.1665,  1.9777, -0.2543,
    -0.3986,  0.0439,  1.1892 );

  vec3 XYZ;
  XYZ.x = Yxy[0] * Yxy[1] / Yxy[2];
  XYZ.y = Yxy[0];
  XYZ.z = Yxy[0] * (1.0 - Yxy[1] - Yxy[2]) / Yxy[2];

  return XYZ2RGB * XYZ;
}

vec3 toneMap (vec3 color, float lumAvg, float lumMax)
{
  const float lumKey = 0.5;
	
	//Convert to Yxy to get luminance
  vec3 Yxy = RGBtoYxy( color );
	
	//Apply the modified operator (Eq. 4)
	float lumScaled = Yxy[0] * lumKey / lumAvg;
	Yxy[0] = (lumScaled * (1.0 + (lumScaled / (lumMax * lumMax)))) / (1.0 + lumScaled);

  //Convert back to RGB
  return YxyToRGB( Yxy );
}

#end

////////////////////////////////////////////////////////////////////////////////////////

#begin BloomDown_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform float avgLuminance;
uniform float maxLuminance;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  vec4 color = vec4(0.0);
  
  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //linear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  //color += texture2D( samplerColor, gl_TexCoord[0].xy );
  //color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-0.5,-0.5) * pixelSize );
  
/*
  float s1 = -2.0;
  float s2 = -1.0;
  float s3 = +1.0;
  float s4 = +2.0;

  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s1,s1) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s2,s1) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s3,s1) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s4,s1) * pixelSize );

  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s1,s2) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s2,s2) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s3,s2) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s4,s2) * pixelSize );

  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s1,s3) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s2,s3) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s3,s3) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s4,s3) * pixelSize );

  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s1,s4) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s2,s4) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s3,s4) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(s4,s4) * pixelSize );

  color /= 16.0;
 */

  //Apply tone mapping and cutoff
  vec3 cutoff = vec3(1.0);
  vec3 colorToned = toneMap( color.rgb, avgLuminance, maxLuminance );
  gl_FragColor = vec4( max( colorToned - cutoff, vec3(0.0) ), 1.0 );

  //vec4 cutoff = vec4(1.0);
  //gl_FragColor = max( color.rgb - cutoff, 0.0 );

  //gl_FragColor = vec4( color.rgb, 1.0 );
}
#end

////////////////////////////////////////////////////////////////////////////////////////

#begin BloomBlur_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;
uniform int radius;

void main (void)
{
  vec3 Color = vec3( 0.0, 0.0, 0.0 );
  
  for (int r=-radius; r<=radius; ++r)
  {
    vec2 texOffset = float(r) * direction * pixelSize;
    vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );
    float weight = (float(abs(r)) / float(radius));
    Color += (1.0 - weight) * texel.rgb;
  }

  Color /= (float(radius)*2.0+1.0) * 0.5;

  gl_FragColor = vec4( Color, 1.0 );
}

#end

////////////////////////////////////////////////////////////////////////////////////////

#begin BloomMix_FS

uniform sampler2D samplerColor;
uniform sampler2D samplerBloom;
uniform float avgLuminance;
uniform float maxLuminance;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  vec3 color = vec3(0.0);

  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  vec3 colorToned = toneMap( colorTexel.rgb, avgLuminance, maxLuminance );
  color += colorToned;
  //color += colorTexel.rgb;

  vec4 bloomTexel = texture2D( samplerBloom, gl_TexCoord[0].xy );
  color += bloomTexel.rgb;

  gl_FragColor = vec4( color, 1.0 );
}

#end
