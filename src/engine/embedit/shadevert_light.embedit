void main (void)
{
  //We expect texture coords to be in [0,1] interval
  //and vertex coords to be in [-1,1] interval.
  //gl_TexCoord[0] = gl_MultiTexCoord0;
  //gl_Position  = gl_Vertex;
  gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;
}
