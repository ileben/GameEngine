#begin Final_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Final_FragmentSource
uniform sampler2D samplerEffects;
uniform sampler2D samplerColor;
uniform float avgLuminance;
uniform float maxLuminance;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  vec3 Color = vec3( 0.0, 0.0, 0.0 );

  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  Color += toneMap( colorTexel.rgb, avgLuminance, maxLuminance );
  //Color += colorTexel.rgb;

  vec4 effectsTexel = texture2D( samplerEffects, gl_TexCoord[0].xy );
  Color += effectsTexel.rgb;
  
  
  gl_FragColor = vec4( Color, 1.0 );
}
#end
