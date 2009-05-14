#begin Bloom_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Bloom_FragmentSource
uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform float avgLuminance;
uniform float maxLuminance;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  vec4 color = vec4(0.0);
  //color += texture2D( samplerColor, gl_TexCoord[0].xy );
  //color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-0.5,-0.5) * pixelSize );
  
  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //linear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;
  
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
