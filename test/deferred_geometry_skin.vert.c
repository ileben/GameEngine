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

  /*
  if (boneWeight[2] > 0.0 &&
      int(boneIndex[2]) == 0)
    skinPoint.x = 0.0;
*/
  //if (int(boneIndex[3]) == 2)
    //skinPoint.x = 0.0;
  //if (int(boneIndex[2]) == 1)
    //skinPoint.x = 0.0;

  /*
  if (boneWeight[3] > 0.0 ||
      boneWeight[1] > 0.0 ||
      boneWeight[2] > 0.0 ||
      boneWeight[3] > 0.0)
      skinPoint.x = 0.0;
*/
  //if (/*int(boneIndex[0]) == 0 &&*/ boneWeight[0] > 0.0)
    //skinPoint.x = 0.0;
  /*
  if (int(boneIndex[0]) == int(boneIndex[1]) &&
      int(boneIndex[0]) == int(boneIndex[2]) &&
      int(boneIndex[0]) == int(boneIndex[3]))
    skinPoint.x = 0.0;
    */

  //if (skinMatrix[0] == skinMatrix[1])
    //skinPoint.x = 0.0;
  /*
  if (boneWeight[0] == boneWeight[1] &&
      boneWeight[0] == boneWeight[2] &&
      boneWeight[0] == boneWeight[3])
    skinPoint.x = 0.0;
*/
  //skinPoint = inPoint;

	normal = normalize( gl_NormalMatrix * skinNormal.xyz );
	point = gl_ModelViewMatrix * skinPoint;
  gl_Position  = gl_ModelViewProjectionMatrix * skinPoint;
}
