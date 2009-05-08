#begin Blur_VertexSource
void main (void)
{
  //We assume vertex coords to be in [-1,1] interval
  //for a full-screen quad
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Blur_FragmentSource
uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;

void main (void)
{
  int radius = 20;
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
