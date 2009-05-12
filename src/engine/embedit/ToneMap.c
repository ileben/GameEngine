#begin ToneMapSource

vec3 RGBtoYxy (vec3 RGB)
{
  const mat3 RGB2XYZ = mat3(
    0.5141364,  0.265068,   0.0241188,
    0.3238786,  0.67023428, 0.1228178,
    0.16036376, 0.06409157, 0.84442666 );

  vec3 XYZ = max( RGB2XYZ * RGB, vec3(0.0001) );

  vec3 Yxy;
  Yxy[0] = XYZ.y;
  Yxy[1] = XYZ.x / (XYZ.x + XYZ.y + XYZ.z);
  Yxy[2] = XYZ.y / (XYZ.x + XYZ.y + XYZ.z);

  return Yxy;
}

vec3 YxyToRGB (vec3 Yxy)
{
  const mat3 XYZ2RGB  = mat3(
     2.5651, -1.0217,  0.0753,
    -1.1665,  1.9777, -0.2543,
    -0.3986,  0.0439,  1.1892 );

  vec3 XYZ;
  XYZ.x = Yxy[0] * Yxy[1] / Yxy[2];
  XYZ.y = Yxy[0];
  XYZ.z = Yxy[0] * (1.0 - Yxy[1] - Yxy[2]) / Yxy[2];

  return XYZ2RGB * XYZ;
}

vec3 toneMap (vec3 color, float lumAvg, float lumMax)
{
  const float lumKey = 0.5;
	
	//Convert to Yxy to get luminance
  vec3 Yxy = RGBtoYxy( color );
	
	//Apply the modified operator (Eq. 4)
	float lumScaled = Yxy[0] * lumKey / lumAvg;
	Yxy[0] = (lumScaled * (1.0 + (lumScaled / (lumMax * lumMax)))) / (1.0 + lumScaled);

  //Convert back to RGB
  return YxyToRGB( Yxy );
}

#end
