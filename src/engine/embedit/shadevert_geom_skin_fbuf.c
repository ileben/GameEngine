attribute vec4 boneIndex;
attribute vec4 boneWeight;
uniform mat4 skinMatrix[24];

void main (void)
{
  vec4 inPoint = gl_Vertex;
  vec4 skinPoint = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i=0; i<4; ++i) {
    skinPoint += boneWeight[i] * (skinMatrix[ int(boneIndex[i]) ] * inPoint);
  }

  gl_Position  = gl_ModelViewProjectionMatrix * skinPoint;
}
