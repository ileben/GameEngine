varying vec3 normal;
varying vec3 light;
varying vec3 halfvec;
varying vec4 ambient, diffuse;

void main()
{
  vec3 eyevert;
  vec3 aux;
  
  //Normal to eye coordinates
  normal = normalize(gl_NormalMatrix * gl_Normal);
  
  //Vertex to eye coordinates (light already)
  eyevert = vec3( gl_ModelViewMatrix * gl_Vertex );
  
  //Direction from vertex to light source
  aux = gl_LightSource[0].position.xyz - eyevert;
  light = normalize(aux);
  
  //Vector at half-way between light and look
  //halfvec = normalize(gl_LightSource[0].halfVector.xyz); //not working on Intel GMA 950!!!
  halfvec = normalize( light - normalize( eyevert ) );
  
  //Light ambient combined with material ambient color
  ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  
  //Light diffuse combined with material diffuse color
  diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;

  //Transform texture coordinates (same coords for both!)
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
  
  //Position to eye coordinates
  gl_Position = ftransform();
}
