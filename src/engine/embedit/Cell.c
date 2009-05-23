#begin Cell_VertexSource

void main (void)
{
  //We assume vertex coords to be in [-1,1] interval
  //for a full-screen quad
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_Vertex;
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin HSLHSV_Func

void HSLtoRGB1 (inout float c)
{
  if (c < 0.0) c += 1.0;
  else if (c > 1.0) c -= 1.0;
}

void HSLtoRGB2 (inout float c, in float p, in float q)
{
  if (c < 0.166666) {
    c = p + ((q - p) * 6.0 * c);
  }else if (c < 0.5) {
    c = q;
  }else if (c < 0.666666) {
    c = p + ((q - p) * 6.0 * (0.666666 - c));
  }else {
    c = p;
  }
}

void quantizeHSL (inout vec3 c, float steps, float alpha)
{
  //RGB to HSL

  float cmax = max( max( c.r, c.g ), c.b );
  float cmin = min( min( c.r, c.g ), c.b );
  float h,s,l;
  
  if (cmax == cmin) {
    h = 0.0;
  }else if (cmax == c.r) {
    h = 60.0 * (c.g - c.b) / (cmax - cmin) + 360.0;
    h = mod( h, 360.0 );
  }else if (cmax == c.g) {
    h = 60.0 * (c.b - c.r) / (cmax - cmin) + 120.0;
  } else {
    h = 60.0 * (c.r - c.g) / (cmax - cmin) + 240.0;
  }

  l = 0.5 * (cmax + cmin);

  if (cmax == cmin) {
    s = 0.0;
  } else if (l <= 0.5) {
    s = (cmax - cmin) / (cmax + cmin);
  } else {
    s = (cmax - cmin) / (2.0 - (cmax + cmin));
  }

  //Quantize L

  l = alpha + (1.0 - alpha) * floor( l * steps + 0.5 ) / steps;

  //HSL to RGB

  float q, p, hk;

  if (l < 0.5) {
    q = l * (1.0 + s);
  }else{
    q = l + s - (l * s);
  }

  p = 2.0 * l - q;

  hk = h / 360.0;

  c.r = hk + 0.333333;
  c.g = hk;
  c.b = hk - 0.333333;

  HSLtoRGB1( c.r );
  HSLtoRGB1( c.g );
  HSLtoRGB1( c.b );

  HSLtoRGB2( c.r, p, q );
  HSLtoRGB2( c.g, p, q );
  HSLtoRGB2( c.b, p, q );
}

void quantizeHSV (inout vec3 c, float steps, float alpha)
{
  //RGB to HSV

  float cmax = max( max( c.r, c.g ), c.b );
  float cmin = min( min( c.r, c.g ), c.b );
  float h,s,v;
  
  if (cmax == cmin) {
    h = 0.0;
  }else if (cmax == c.r) {
    h = 60.0 * (c.g - c.b) / (cmax - cmin) + 360.0;
    h = mod( h, 360.0 );
  }else if (cmax == c.g) {
    h = 60.0 * (c.b - c.r) / (cmax - cmin) + 120.0;
  }else {
    h = 60.0 * (c.r - c.g) / (cmax - cmin) + 240.0;
  }

  if (cmax == 0.0) {
    s = 0.0;
  }else{
    s = 1.0 - cmin/cmax;
  }

  v = cmax;

  //Quantize V

  v = alpha + (1.0 - alpha) * floor( v * steps + 0.5 ) / steps;

  //HSV to RGB

  int hi = int( mod( floor( h / 60.0 ), 6.0 ));
  float f = h / 60.0 - floor( h / 60.0 );

  float p = v * (1.0 - s);
  float q = v * (1.0 - f * s);
  float t = v * (1.0 - (1.0 - f) * s );
  
  if (hi == 0) {
    c = vec3( v,t,p );
  }else if (hi == 1) {
    c = vec3( q,v,p );
  }else if (hi == 2) {
    c = vec3( p,v,t );
  }else if (hi == 3) {
    c = vec3( p,q,v );
  }else if (hi == 4) {
    c = vec3( t,p,v );
  }else if (hi == 5) {
    c = vec3( v,p,q );
  }
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin QuantizeLight_Func

void quantizeLight (inout vec3 c, float light, float steps, float alpha)
{
  float coeff = alpha + (1.0 - alpha) * floor( light * steps + 0.5 ) / steps;
  c *= coeff / max( light, 0.001 );
}

#end

////////////////////////////////////////////////////////////////////////////////

#begin Cell_FragmentSource

uniform sampler2D samplerColor;

void main (void)
{
  vec4 colorTexel = texture2D( samplerColor, gl_TexCoord[0].xy );
  quantizeLight( colorTexel.rgb, colorTexel.a, 4.0, 0.15 );
  //quantizeHSV( colorTexel.rgb, 4.0, 0.15 );
  gl_FragColor = vec4( colorTexel.rgb, 1.0 );
}

#end
