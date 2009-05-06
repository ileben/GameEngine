#begin skinCoordNode
outCoord = vec4( 0,0,0,0 );
for (int i=0; i<4; ++i)
  outCoord += jointWeight[i] * (skinMatrix[ int(jointIndex[i]) ] * inCoord);
#end


#begin skinNormalNode
vec4 inNormal4 = vec4( inNormal, 0.0 );
outNormal = vec3( 0,0,0 );
for (int i=0; i<4; ++i)
  outNormal += jointWeight[i] * (skinMatrix[ int(jointIndex[i]) ] * inNormal4).xyz;
#end
