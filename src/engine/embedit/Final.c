#begin Final_VertexSource
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin Final_FragmentSource
uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;
uniform sampler2D samplerBloom;
uniform sampler2D samplerDof;
uniform float avgLuminance;
uniform float maxLuminance;
uniform vec2 poissonCoords[12];
uniform vec2 pixelSize;
vec3 toneMap (vec3 c, float avg, float max);

void main (void)
{
  //vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  //vec4 normalTexel = texture2D( samplerNormal, gl_TexCoord[0].xy );
  //vec4 bloomTexel = texture2D( samplerBloom, gl_TexCoord[0].xy );
  vec4 centerTexel = texture2D( samplerDof, gl_TexCoord[0].xy );
  float centerBlur = abs( centerTexel.a * 2.0 - 1.0 );
  vec3 Color = vec3( 0.0 );

  //vec3 colorToned = toneMap( colorTexel.rgb, avgLuminance, maxLuminance );
  //vec3 dofToned = toneMap( dofTexel.rgb, avgLuminance, maxLuminance );
  //Color += mix( colorToned, dofToned, abs(blur) );

  //Color += colorToned;
  //Color += dofToned;

  //Color += colorTexel.rgb;
  //Color += bloomTexel.rgb;

  vec4 dofColor = vec4( 0.0 );
  //dofColor.rgb += centerTexel.rgb;
  //dofColor.a += 1.0;

  float discRadius = centerBlur * 20.0;
  for (int p=0; p<12; ++p)
  {
    //Sample the dof texture
    vec2 tapCoord = gl_TexCoord[0].xy + pixelSize * poissonCoords[p] * discRadius;
    vec4 tapTexel = texture2D( samplerDof, tapCoord );

    //Unpack tap pixel's own blur value
    float tapBlur = abs( tapTexel.a * 2.0 - 1.0 );

    //If tap is closer than center pixel use it's blur as weight else full weight
    float tapWeight = (tapTexel.a >= centerTexel.a) ? 1.0 : tapBlur;
    //float tapWeight = (tapTexel.a >= centerTexel.a) ? 1.0 : 0.0;

    //Accumulate
    dofColor.rgb += tapTexel.rgb * tapWeight;
    dofColor.a += tapWeight;
    //dofColor.rgb += tapTexel.rgb;
    //dofColor.a += 1.0;
  }

  Color += dofColor.rgb / dofColor.a;
  
  gl_FragColor = vec4( Color, 1.0 );
}
#end
