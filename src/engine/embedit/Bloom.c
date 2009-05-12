#begin Bloom_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Bloom_FragmentSource
uniform sampler2D samplerColor;
uniform float avgLuminance;
uniform float maxLuminance;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
 
  vec3 cutoff = vec3(1.0);
  vec3 colorToned = toneMap( colorTexel.rgb, avgLuminance, maxLuminance );
  gl_FragColor = vec4( max( colorToned - cutoff, vec3(0.0) ), 1.0 );

  //vec4 cutoff = vec4(1.0);
  //gl_FragColor = max( accumTexel - cutoff, 0.0 );

  //gl_FragColor = accumTexel;
}
#end
