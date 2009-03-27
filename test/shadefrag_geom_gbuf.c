varying vec3 normal;
varying vec4 point;

void main (void)
{
  gl_FragData[0] = point;
  gl_FragData[1] = vec4( normalize( normal ), 0.0 );
  gl_FragData[2] = gl_FrontMaterial.diffuse;
  gl_FragData[3] = vec4( gl_FrontMaterial.specular.xyz, gl_FrontMaterial.shininess );
}
