#begin fragEndCodeGBuffer
gl_FragData[0] = Coord;
gl_FragData[1] = vec4( normalize( Normal ), 0.0 );
gl_FragData[2] = vec4( Diffuse.xyz, gl_FrontMaterial.emission.a );
gl_FragData[3] = vec4( Specular.xyz, SpecularExp );
#end

#begin fragEndCodeShadowMap
gl_FragColor = vec4(1,1,1,1);
#end