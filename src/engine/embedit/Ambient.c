#begin Ambient_VertexSource
void main (void)
{
  //We assume vertex coords to be in [-1,1] interval
  //for a full-screen quad
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end


#begin Ambient_FragmentSource
uniform sampler2D samplerColor;

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  gl_FragColor = vec4( colorTexel.rgb * colorTexel.a, colorTexel.a );
  //gl_FragColor = vec4( colorTexel.rgb, colorTexel.a );
}
#end
