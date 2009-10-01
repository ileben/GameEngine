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


#begin skinTangentNode
vec4 inTangent4 = vec4( inTangent, 0.0 );
outTangent = vec3( 0,0,0 );
for (int i=0; i<4; ++i)
  outTangent += jointWeight[i] * (skinMatrix[ int(jointIndex[i]) ] * inTangent4).xyz;
#end

#begin skinBitangentNode
vec4 inBitangent4 = vec4( inBitangent, 0.0 );
outBitangent = vec3( 0,0,0 );
for (int i=0; i<4; ++i)
  outBitangent += jointWeight[i] * (skinMatrix[ int(jointIndex[i]) ] * inBitangent4).xyz;
#end
