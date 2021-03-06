#begin Ambient_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end


#begin Ambient_FragmentSource
uniform vec3 ambientColor;
uniform sampler2D samplerColor;

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  float luminosity = colorTexel.a * 2.0;
  gl_FragColor = vec4(
    colorTexel.rgb * ambientColor +
    colorTexel.rgb * luminosity,
    luminosity );
}
#end
