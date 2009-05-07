#begin fragEndCodeGBuffer
gl_FragData[0] = vec4( normalize( tNormal ), tCoord.z );
gl_FragData[1] = vec4( tDiffuse.xyz, gl_FrontMaterial.emission.a );
gl_FragData[2] = vec4( tSpecular.xyz, tSpecularExp / 128.0 );
gl_FragData[3] = vec4( uCellShading, uSpecularity, 0.0, 0.0 );
#end

#begin fragEndCodeShadowMap
gl_FragColor = vec4(1,1,1,1);
#end