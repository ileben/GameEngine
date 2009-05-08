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

void main (void)
{
  vec3 Color = vec3( 0.0, 0.0, 0.0 );

  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  Color += colorTexel.rgb;

  vec4 effectsTexel = texture2D( samplerEffects, gl_TexCoord[0].xy );
  Color += effectsTexel.rgb;
  
  gl_FragColor = vec4( Color, 1.0 );
}
#end
