#begin Shadow_VS
void main (void)
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
#end

#begin Shadow_FS
void main (void)
{
  gl_FragColor = vec4(0.0);
}
#end
