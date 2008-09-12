varying vec3 normal;
varying vec3 light;
varying vec3 halfvec;
varying vec4 ambient, diffuse;
uniform bool useTextures[2];
uniform sampler2D textures[2];

void main()
{
  vec3 n,l,h;
  float cosNL;
  float cosNH;
  float shine;
  vec4 spec;
  vec4 oneMtex;
  vec4 texelD;
  vec4 texelS;
  vec4 color = ambient;

  //Cosine of normal to light angle
  n = normalize(normal);
  l = normalize(light);
  cosNL = max( dot(n,l), 0.0 );
  
  if (cosNL > 0.0) {
  //if (n.x > 0.5) {

    //color = vec4(0.0,1.0,0.0,0.0);

    
    //Apply diffuse color
    if (useTextures[0]) {
      texelD = texture2D(textures[0], gl_TexCoord[0].st);
      color += cosNL * texelD;
    }else{ color += cosNL * diffuse; }

    //Material and specular color combined
    spec = gl_FrontMaterial.specular * gl_LightSource[0].specular;

    //Shininess factor
    h = normalize(halfvec);
    cosNH = max( dot(n,h), 0.0 );
    shine = pow(cosNH, gl_FrontMaterial.shininess);
    
    //Apply specular color
    if (useTextures[1]) {
      texelS = texture2D(textures[1], gl_TexCoord[0].st);
      color += shine * texelS.r * spec;
    }else{ color += shine * spec; }
  
  //}else{
    //color = vec4(1.0,0.0,0.0,0.0);
  }
  
  gl_FragColor = color;
}
