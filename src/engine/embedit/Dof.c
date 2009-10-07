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
//dofParams[0]: center of focus
//dofParams[1]: full focus range
//dofParams[2]: near falloff range
//dofParams[3]: far falloff range

float computeCoC (float depth)
{
  //The first row yields [-1,0] for depths closer than focus center, 0 otherwise
  //The second row yields [0,+1] for depths farther than focus center, 0 otherwise
  return
    clamp( ((depth - dofParams[0]) + dofParams[1]) / (dofParams[2]), -1.0, 0.0 ) +
    clamp( ((depth - dofParams[0]) - dofParams[1]) / (dofParams[3]), 0.0, +1.0 );
}

vec4 computeCoC4 (vec4 depth)
{
  return
    clamp( ((depth - dofParams[0]) + dofParams[1]) / (dofParams[2]), vec4(-1.0), vec4(0.0) ) +
    clamp( ((depth - dofParams[0]) - dofParams[1]) / (dofParams[3]), vec4(0.0), vec4(+1.0) );
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
  //float depth = texture2D( samplerNormal, gl_TexCoord[0].xy ).w;

  if (paramsTexel[1] > 0.0)
    quantizeLight( colorTexel.rgb, colorTexel.a, 4.0, 0.3 );

  //gl_FragColor = vec4( colorTexel.rgb, abs( computeCoC( depth ) ) );
  //gl_FragColor = vec4( colorTexel.rgb, -1.0 * min( computeCoC( depth ), 0.0 ) );
  gl_FragColor = vec4( colorTexel.rgb, 0.0 );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofDown_FS

uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;
uniform vec2 pixelSize;
vec4 computeCoC4 (vec4 depth);

void updateCoC (vec4 depth, inout vec4 CoC)
{
  vec4 newCoC;

  //newCoC = computeCoC4( depth );
  //CoC = min( CoC, newCoC );

  newCoC = computeCoC4( depth );
  CoC = min( CoC, newCoC );

  //newCoC = max( computeCoC4( depth ), 0.0 );
  //CoC = max( CoC, newCoC );

  //newCoC = abs( computeCoC4( depth ) );
  //CoC = min( CoC, newCoC );

  //newCoC = abs( computeCoC4( depth ) );
  //CoC += newCoC;

  //Just front
  //newCoC = max( computeCoC4( depth ), 0.0 );
  //CoC += newCoC;
}

void main (void)
{
  vec4 color = vec4(0.0);

  //Sample 4 times on the corners between 4 pixels. This takes advantage of
  //bilinear filtering to get the same result as if sampling 16 pixel centers.
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,-1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(+1.0,+1.0) * pixelSize );
  color += texture2D( samplerColor, gl_TexCoord[0].xy + vec2(-1.0,+1.0) * pixelSize );
  color /= 4.0;

  //gl_FragColor = color;

  //We need only the near (negative) CoC values so we need to downsample "manually"
  //instead of filtering the texture and using non-center pixel lookups. We would either
  //need to do this for the full CoC or near CoC depending on which is written into
  //the color buffer at the dof init pass.

  vec4 depth = vec4( 0.0 );
  vec4 CoC = vec4( 1.0 );
  vec4 newCoC;

  vec4 offset = vec4(-1.5, -0.5, 0.5, 1.5);

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.xw * pixelSize ).w;
  updateCoC( depth, CoC );

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.yw * pixelSize ).w;
  updateCoC( depth, CoC );

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.zw * pixelSize ).w;
  updateCoC( depth, CoC );

  depth[0] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wx * pixelSize ).w;
  depth[1] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wy * pixelSize ).w;
  depth[2] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.wz * pixelSize ).w;
  depth[3] = texture2D( samplerNormal, gl_TexCoord[0].xy + offset.ww * pixelSize ).w;
  updateCoC( depth, CoC );

  float maxCoC = min( min( CoC[0], CoC[1] ), min( CoC[2], CoC[3] ) );
  //float maxCoC = max( max( CoC[0], CoC[1] ), max( CoC[2], CoC[3] ) );
  //float maxCoC = (CoC[0] + CoC[1] + CoC[2] + CoC[3]) / 16.0;

  gl_FragColor = vec4( color.rgb, maxCoC );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofExtractFar_FS

uniform sampler2D samplerColor;

void main (void)
{
  vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy );
  gl_FragColor = vec4( max( texel.a, 0.0 ) );
}

#end


#begin DofExtractNear_FS

uniform sampler2D samplerColor;

void main (void)
{
  vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy );
  gl_FragColor = vec4( -1.0 * min( texel.a, 0.0 ) );
}

#end


#begin DofBlurNear_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;
uniform int radius;

void main (void)
{
  vec4 colorSum = vec4( 0.0 );
  float weightSum = 0.0;
  
  for (int r=-radius; r<=radius; ++r)
  {
    vec2 texOffset = float(r) * direction * pixelSize;
    vec4 texel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );
    
    float distRatio = (float(abs(r)) / float(radius));
    float weight = (1.0 - distRatio);

    colorSum += weight * texel;
    weightSum += weight;
  }

  colorSum /= weightSum;

  gl_FragColor = colorSum;
}

#end


#begin DofMerge_FS

uniform sampler2D samplerNear;
uniform sampler2D samplerNearBlur;
uniform sampler2D samplerFar;

void main (void)
{
  vec4 texelNear = texture2D( samplerNear, gl_TexCoord[0].xy );
  vec4 texelNearBlur = texture2D( samplerNearBlur, gl_TexCoord[0].xy );
  vec4 texelFar = texture2D( samplerFar, gl_TexCoord[0].xy );
  vec4 nearCoC = clamp( max( texelNear, 2.0 * texelNearBlur - texelNear), 0.0, 1.0 );
  gl_FragColor = vec4( max( nearCoC, texelFar ) );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin GaussBlur_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform int radius;

void main (void)
{
  vec4 colorSum = vec4(0.0);
  float weightSum = 0.0;
  
  //const int radius = 5;
  for (int rx=-radius; rx<=radius; ++rx)
  {
    for (int ry=-radius; ry<=radius; ++ry)
    {
      //Tap from the source
      vec2 dir = vec2(float(rx),float(ry));
      vec2 texOffset = dir * pixelSize;
      vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );

      //Calculate weight
      float distRatio = (min(length(dir),float(radius)) / float(radius));
      float weight = (1.0 - distRatio);

      //Accumulate
      colorSum += weight * colorTexel;
      weightSum += weight;
    }
  }

  //Average
  colorSum /= weightSum;
  gl_FragColor = colorSum;
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin DofBlur_FS

uniform sampler2D samplerColor;
uniform vec2 pixelSize;
uniform vec2 direction;
uniform int radius;
uniform vec4 dofParams;

void main (void)
{
  vec4 colorSum = vec4(0.0);
  float rgbWeightSum = 0.0;
  float cocWeightSum = 0.0;

  //Start with center texel;
  vec4 colorCenter = texture2D( samplerColor, gl_TexCoord[0].xy );
  
  colorSum += colorCenter;
  rgbWeightSum += 1.0;
  cocWeightSum += 1.0;
  
  //const int radius = 5;
  for (int rx=-radius; rx<=radius; ++rx)
  {
    for (int ry=-radius; ry<=radius; ++ry)
    {
      //Tap from the source
      vec2 dir = vec2(float(rx),float(ry));
      vec2 texOffset = dir * pixelSize;
      vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy + texOffset );

      //Weight the texels by CoC
      float distRatio = (min(length(dir),float(radius)) / float(radius));
      float rgbWeight = (1.0 - distRatio) * colorTexel.a;
      float cocWeight = (1.0 - distRatio);

      //Accumulate
      colorSum.rgb += rgbWeight * colorTexel.rgb;
      colorSum.a   += cocWeight * colorTexel.a;
      rgbWeightSum += rgbWeight;
      cocWeightSum += cocWeight;
    }
  }

  //Average
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
float computeCoC (float depth);

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  vec4 dofMedTexel = texture2D( samplerMedBlur, gl_TexCoord[0].xy );
  vec4 dofLargeTexel = texture2D( samplerLargeBlur, gl_TexCoord[0].xy );
  vec4 depthTexel = texture2D( samplerDepth, gl_TexCoord[0].xy );
  vec3 Color = vec3(0.0);

  float rawCoC = computeCoC( depthTexel.a );
  float farCoC = max( rawCoC, 0.0 );

  //This blends between blurred and non-blurred CoC so pixels right on the edge have full blur
  float clearNearCoC = -1.0 * min( rawCoC, 0.0 );
  float nearCoC = clamp( max( clearNearCoC, 2.0 * dofMedTexel.a - clearNearCoC), 0.0, 1.0 );
  //float nearCoC = dofLargeTexel.a;
  
  float CoC = max( nearCoC, farCoC );

  //Color = vec3( abs( dofMedTexel.a ) );
  //Color = dofMedTexel.rgb;
  //Color = vec3( abs( depthTexel.a ) );
  //Color = colorTexel.rgb;
  //Color = colorTexel.aaa;
  //Color = dofLargeTexel.rgb;
  //Color = dofLargeTexel.aaa;
  //Color = dofMedTexel.rgb;
  //Color = vec3( CoC );
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
/////// Poisson disc implementation (retained for reference)
/*

//This array is passed in as a uniform
GLfloat poissonCoords[] = {
  -0.326212f, -0.40581f,
  -0.840144f, -0.07358f,
  -0.695914f,  0.457137f,
  -0.203345f,  0.620716f,
   0.96234f,  -0.194983f,
   0.473434f, -0.480026f,
   0.519456f,  0.767022f,
   0.185461f, -0.893124f,
   0.507431f,  0.064425f,
   0.89642f,   0.412458f,
  -0.32194f,  -0.932615f,
  -0.791559f, -0.59771f
};

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
