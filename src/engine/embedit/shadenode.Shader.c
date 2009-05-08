#begin fragEndCodeGBuffer
gl_FragData[0] = vec4( normalize( tNormal ), tCoord.z );
gl_FragData[1] = vec4( tDiffuse.xyz, uLuminosity * 0.5 );
gl_FragData[2] = vec4( tSpecular.xyz, tSpecularExp / 128.0 );
gl_FragData[3] = vec4( uSpecularity, uCellShading, 0.0, 0.0 );
#end

#begin fragEndCodeShadowMap
gl_FragColor = vec4(1,1,1,1);
#end