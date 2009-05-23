#begin Dof_VS
void main (void)
{
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}
#end

////////////////////////////////////////////////////////////////////////////////

#begin ComputeCoC_Func

uniform vec4 dofParams;
//uniform float focusDepth;  //dofParams[0]: center of focus
//uniform float focusRange;  //dofParams[1]: full focus range
//uniform float farFalloff;  //dofParams[2]: far falloff range
//uniform float nearFalloff; //dofParams[3]: near falloff range

float computeCoC (float depth)
{
  //The first row yields [0,1] for depths farther than focusDepth, 0 otherwise
  //The second row yields [-1,0] for depths closer than focusDepth, 0 otherwise
  //clamp( ((depth - focusDepth) - focusRange) / (farFalloff - focusRange), 0.0, +1.0 ) -
  //clamp( ((depth - focusDepth) + focusRange) / (nearFalloff - focusRange), -1.0, 0.0 );

  return
    clamp( ((depth - dofParams[0]) - dofParams[1]) / (dofParams[2] - dofParams[1]), 0.0, +1.0 ) -
    clamp( ((depth - dofParams[0]) + dofParams[1]) / (dofParams[3] - dofParams[1]), -1.0, 0.0 );
}

vec4 computeCoC4 (vec4 depth)
{
  return
    clamp( ((depth - dofParams[0]) - dofParams[1]) / (dofParams[2] - dofParams[1]), vec4(0.0), vec4(+1.0) ) -
    clamp( ((depth - dofParams[0]) + dofParams[1]) / (dofParams[3] - dofParams[1]), vec4(-1.0), vec4(0.0) );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofInit_FS

uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;
uniform sampler2D samplerParams;
float computeCoC (float depth);
void quantizeLight (inout vec3 c, float light, float steps, float alpha);

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  vec4 paramsTexel = texture2D( samplerParams, gl_TexCoord[0].xy );
  float depth = texture2D( samplerNormal, gl_TexCoord[0].xy ).w;

  if (paramsTexel[1] > 0.0)
    quantizeLight( colorTexel.rgb, colorTexel.a, 4.0, 0.15 );

  gl_FragColor = vec4( colorTexel.rgb, computeCoC( depth ) );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofDown_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;

void main (void)
{
  vec4 color = vec4(0.0);

  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //bilinear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  gl_FragColor = color;
/*
  //We can't just average (so neither filter) the depth or the CoC values cause the
  //edges between far and near objects would result in values close to zero. Instead
  //we always take the maximum of all the samples. We calculate it for 4 pixels at
  //once to use hardware efficienty.
  vec4 offset = vec4(-1.5, -0.5, 0.5, 1.5);

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xw * pixelSize ).w;
  newCoC = computeCoC4( depth );
  //CoC = max( CoC, newCoC );
  CoC += newCoC;

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yw * pixelSize ).w;
  newCoC = computeCoC4( depth );
  //CoC = max( CoC, newCoC );
  CoC += newCoC;

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zw * pixelSize ).w;
  newCoC = computeCoC4( depth );
  //CoC = max( CoC, newCoC );
  CoC += newCoC;

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.ww * pixelSize ).w;
  newCoC = computeCoC4( depth );
  //CoC = max( CoC, newCoC );
  CoC += newCoC;

  //maxCoC = max( max( CoC[0], CoC[1] ), max( CoC[2], CoC[3] ) );
  maxCoC = (CoC[0] + CoC[1] + CoC[2] + CoC[3]) / 16.0;
  gl_FragColor = vec4( color.rgb, maxCoC );
  */
}

#end

#begin DofNear_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec4 dofParams;

void main (void)
{
  vec4 color = vec4(0.0);

  //Sample 4 times in the middle of 4 pixels. This takes advantage of
  //bilinear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  gl_FragColor = color;
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DepthBlur_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;
uniform int radius;
uniform vec4 dofParams;

void main (void)
{
  vec4 colorSum = vec4(0.0);
  float weightSum = 0.0;

  //Start with center texel;
  vec4 center = texture2D( samplerColor, gl_TexCoord[0].xy );
  //Color.rgb += center.rgb;
  //Color.a += 1.0;
  
  //const int radius = 5;
  for (int rx=-radius; rx<=radius; ++rx)
  {
    for (int ry=-radius; ry<=radius; ++ry)
    {
      vec2 dir = vec2(float(rx),float(ry));
      vec2 texOffset = dir * pixelSize;
      vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );

      //Weight the texel by distance
      float distRatio = (min(length(dir),float(radius)) / float(radius));
      float texWeight = (1.0 - distRatio);
      //if (texel.a > dofParams[0] - dofParams[1]) texel = center;
      //if (texel.a > center.a) texel = center;
      //if (texel.a > center.a) texWeight = 0.0;

      //Accumulate
      colorSum += texWeight * texel;
      weightSum += texWeight;
    }
  }

  colorSum /= weightSum;
  gl_FragColor = colorSum;
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofBlur_FS

uniform sampler2D samplerColor;
uniform sampler2D samplerDepth;
uniform vec2 pixelSize;
uniform vec2 direction;
uniform int radius;
uniform vec4 dofParams;

void main (void)
{
  vec4 colorSum = vec4(0.0);
  float rgbWeightSum = 0.0;
  float cocWeightSum = 0.0;

  vec4 depthCenter = texture2D( samplerDepth, gl_TexCoord[0].xy );

  //Start with center texel;
  vec4 center = texture2D( samplerColor, gl_TexCoord[0].xy );
  colorSum += center;
  rgbWeightSum += 1.0;
  cocWeightSum += 1.0;
  
  //const int radius = 5;
  for (int rx=-radius; rx<=radius; ++rx)
  {
    for (int ry=-radius; ry<=radius; ++ry)
    {
      vec2 dir = vec2(float(rx),float(ry));
      vec2 texOffset = dir * pixelSize;
      vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );
      vec4 depthTexel = texture2D( samplerDepth, gl_TexCoord[0].xy + texOffset );

      //Weight the texels by CoC
      //float texCoC = 0.5 + (texel.a * (float(radius)-0.5));
      //float distRatio = clamp( length(dir) / texCoC, 0.0, 1.0 );
      //float texelWeight = 1.0 - distRatio;

      float distRatio = (min(length(dir),float(radius)) / float(radius));
      float rgbWeight = (1.0 - distRatio) * texel.a;
      //if (texel.a == 0.0)
        //if (depthTexel.a > dofParams[0] + dofParams[1])
          //rgbWeight = 0.0;

      //float rgbWeight = (1.0 - distRatio);
      //if (depthCenter.a > dofParams[0] + dofParams[1])
      //if (depthCenter.a > dofParams[0] - dofParams[1])
      //if (depthCenter.a > dofParams[0])
        //rgbWeight *= texel.a;

      //if (center.a == 0.0)
        //if (depthTexel.r < depthCenter.r)
          //rgbWeight = 0.0;

      //float rgbWeight = (1.0 - distRatio);
      float cocWeight = (1.0 - distRatio);

      //Accumulate
      colorSum.rgb += rgbWeight * texel.rgb;
      colorSum.a   += cocWeight * texel.a;
      rgbWeightSum += rgbWeight;
      cocWeightSum += cocWeight;
    }
  }

  colorSum.rgb /= rgbWeightSum;
  colorSum.a /= cocWeightSum;
  gl_FragColor = colorSum;
}

#end

////////////////////////////////////////////////////////////////////////////////////////

#begin DofMix_FS

uniform sampler2D samplerColor;
uniform sampler2D samplerMedBlur;
uniform sampler2D samplerLargeBlur;
uniform sampler2D samplerDepth;
uniform sampler2D samplerDepthBlur;
uniform vec4 dofParams;

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  vec4 dofMedTexel = texture2D( samplerMedBlur, gl_TexCoord[0].xy );
  vec4 dofLargeTexel = texture2D( samplerLargeBlur, gl_TexCoord[0].xy );
  vec4 depthTexel = texture2D( samplerDepth, gl_TexCoord[0].xy );
  vec4 depthBlurTexel = texture2D( samplerDepthBlur, gl_TexCoord[0].xy );
  vec3 Color = vec3(0.0);

  //float CoC = (depthBlurTexel.r < depthTexel.r) ? dofLargeTexel.a : colorTexel.a;

  //This blends between blurred and non-blurred CoC so that pixel right on the edge have full blur
  float nearCoC = max( colorTexel.a, 2.0 * dofLargeTexel.a - colorTexel.a );
  float CoC = (depthBlurTexel.a < depthTexel.a) ? nearCoC : colorTexel.a;

  //if (depthBlurTexel.a > 0.0 )
    //CoC = mix( colorTexel.a, nearCoC, depthBlurTexel.a );
    //CoC = dofLargeTexel.a;


  //Color = vec3(0.0);
  //if (depthBlurTexel.a < depthTexel.a)
    //Color = vec3(1.0);
  
  //Color = depthBlurTexel.aaa / 100.0;
  //Color = depthBlurTexel.aaa;

  //Color = colorTexel.rgb;
  //Color = colorTexel.aaa;
  //Color = dofLargeTexel.rgb;
  //Color = dofLargeTexel.aaa;
  //Color = dofMedTexel.rgb;
  Color = mix( colorTexel.rgb, dofLargeTexel.rgb, CoC );

  /*
  float d = 0.50;
  float w1 = CoC / d;
  float w2 = (CoC - d) / (1.0 - d);
  
  if (CoC < d)
    Color += mix( colorTexel.rgb, dofMedTexel.rgb, w1 );
  else
    Color += mix( dofMedTexel.rgb, dofLargeTexel.rgb, w2 );
*/

  gl_FragColor = vec4( Color, 1.0 );
}

#end

////////////////////////////////////////////////////////////////////////////////////////

  /*
  /// Poisson disc implementation (retained for reference)

  vec4 centerTexel = texture2D( samplerDof, gl_TexCoord[0].xy );
  float centerBlur = abs( centerTexel.a * 2.0 - 1.0 );
  vec3 Color = vec3( 0.0 );

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
  */
