varying vec3 normal;
varying vec4 point;
attribute vec4 boneIndex;
attribute vec4 boneWeight;
uniform mat4 skinMatrix[24];

void main (void)
{
  vec4 inNormal = vec4( gl_Normal, 0.0 );
  vec4 inPoint = gl_Vertex;

  vec4 skinNormal = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 skinPoint = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i=0; i<4; ++i) {
    skinNormal += boneWeight[i] * (skinMatrix[ int(boneIndex[i]) ] * inNormal);
    skinPoint += boneWeight[i] * (skinMatrix[ int(boneIndex[i]) ] * inPoint);
  }

	normal = normalize( gl_NormalMatrix * skinNormal.xyz );
	point = gl_ModelViewMatrix * skinPoint;
  gl_Position  = gl_ModelViewProjectionMatrix * skinPoint;
}
