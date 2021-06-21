#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif

	 uniform  int renderType;

	 uniform  vec4 inColor;

     varying  vec3 fNormal;
     varying  vec3 fPosition;

     void main()
     {
		if (renderType == 1)
			 gl_FragColor = vec4(0.95,0.95,0,1.0);
		else if(renderType == 2)
			gl_FragColor = vec4(0.12,0.411,0.82,1.0);
		else if (renderType == 3)
			gl_FragColor = vec4(inColor);
			else 
		gl_FragColor = vec4(inColor);

     }
