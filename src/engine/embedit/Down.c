#begin Down_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Down_FragmentSource
uniform sampler2D samplerColor;
uniform vec2 pixelSize;

void main (void)
{
  vec4 color = vec4(0.0);
  
  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //bilinear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  gl_FragColor = color;
}
