#begin Dof_VS
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

#begin DofCoC_Source
uniform vec4 dofParams;
//uniform float focusDepth;  //dofParams[0]: center of focus
//uniform float focusRange;  //dofParams[1]: full focus range
//uniform float farFalloff;  //dofParams[2]: far falloff range
//uniform float nearFalloff; //dofParams[3]: near falloff range

float computeCoC (float depth)
{
  return
    max(0.0, (depth - dofParams[0]) - dofParams[1]) / (dofParams[2] - dofParams[1]) +
    min(0.0, (depth - dofParams[0]) - dofParams[1]) / (dofParams[3] - dofParams[1]);
  //max(0.0, (depth - focusDepth) - focusRange) / (farFalloff - focusRange) +
  //min(0.0, (depth - focusDepth) + focusRange) / (nearFalloff - focusRange);
}

vec4 computeCoC4 (vec4 depth)
{
  return
    max(0.0, (depth - dofParams[0]) - dofParams[1]) / (dofParams[2] - dofParams[1]) +
    min(0.0, (depth - dofParams[0]) - dofParams[1]) / (dofParams[3] - dofParams[1]);
}
#end

#begin DofDown_FS
uniform sample2D samplerColor;
uniform sample2D samplerNormal;
uniform vec2 pixelSize;
float computeCoC4 (vec4 depth);

void main (void)
{
  vec4 offset = vec4(-1.5, -0.5, 0.5, 1.0);
  vec4 color = vec4(0.0);
  vec4 depth;
  vec4 CoC;

  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //bilinear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  //Effectively, we want to calculate CoC for all the 16 source pixels,
  //then find the average (which is not the same as averaging depth!)
  //but we calculate it for 4 pixels at once to use hardware efficienty
  depth[0] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xx * pixelSize );
  depth[1] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xy * pixelSize );
  depth[2] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xz * pixelSize );
  depth[3] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xw * pixelSize );
  CoC += computeCoC4( depth );

  depth[0] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yx * pixelSize );
  depth[1] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yy * pixelSize );
  depth[2] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yz * pixelSize );
  depth[3] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yw * pixelSize );
  CoC += computeCoC4( depth );

  depth[0] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zx * pixelSize );
  depth[1] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zy * pixelSize );
  depth[2] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zz * pixelSize );
  depth[3] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zw * pixelSize );
  CoC += computeCoC4( depth );

  depth[0] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wx * pixelSize );
  depth[1] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wy * pixelSize );
  depth[2] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wz * pixelSize );
  depth[3] += texture2D( samplerNormal, gl_TexCoord[0].xy + offset.ww * pixelSize );
  CoC += computeCoC4( depth );

  return (color.rgb, (CoC[0] + CoC[1] + CoC[2] + CoC[3]) / 16.0 );
}
#end

#begin DofBlurSeparable_FS
uniform sample2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;

void main (void)
{
  vec4 Color = vec4(0.0);
  vec4 center = texture2D( samplerColor, gl_TexCoord[0].xy );
  
  const int radius = 20;
  for (int r=-radius; r<=radius; ++r)
  {
    vec2 texOffset = float(r) * direction * pixelSize;
    vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );

    float distRatio = (float(abs(r)) / float(radius));
    float texelWeight = (1.0 - distRatio) * abs(texel.a);

    Color.rgb += texelWeight * texel.rgb;
    Color.a += texelWeight;
  }

  Color /= Color.a;
  gl_FragColor = vec4( Color.rgb, center.a );
}
#end

#begin DofBlend_FragmentSource
uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;
uniform sampler2D samplerBlur;
uniform vec2 poissonCoord[12];
float pixelSizeHigh;
float pixelSizeLow;

void main (void)
{
  vec4 centerTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  float centerDepth = texture2D( samplerNormal, gl_TexCoord[0].xy ).w;
  
  for (int p=0; p<12; ++p)
  {
    vec2 coordHigh = gl_TexCoord[0].xy + poissonCoord[p] * 
  }

}
#end
