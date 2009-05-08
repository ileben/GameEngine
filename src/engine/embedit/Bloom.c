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

void main (void)
{
  vec4 accumTexel = texture2D( samplerAccum, gl_TexCoord[0].xy );
  float luminosity = accumTexel.a;
  
  //float cutoff = 1.0;
  //gl_FragColor = max( accumTexel * (luminosity - cutoff), 0.0 );
  
  vec4 cutoff = vec4(0.2);
  gl_FragColor = max( accumTexel - cutoff, 0.0 );

  //gl_FragColor = accumTexel;
}
#end
