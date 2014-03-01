uniform int num_dlights = 0;
uniform float normal_scale = 1.0;
uniform vec4 color_scale = vec4(1.0);
uniform vec3 world_space_offset = vec3(0.0);

void calc_leaf_lighting()
{
	// transform the normal into eye space, but don't normalize because it may be scaled for shadows
	vec3 normal = gl_NormalMatrix * gl_Normal * normal_scale;
	
	vec4 eye_space_pos = gl_ModelViewMatrix * gl_Vertex;
	//if (dot(normal, eye_space_pos.xyz) > 0.0) normal = -normal; // facing away from the eye, so reverse (could use faceforward())
	float nscale = ((dot(normal, eye_space_pos.xyz) > 0.0) ? -1.0 : 1.0);
	normal *= nscale;
	
	// Compute the globalAmbient term
	bool shadowed = (sqrt(dot(gl_Normal, gl_Normal)) < 0.4);
	vec4 color    = gl_FrontMaterial.emission;
	if (enable_light0) color += add_leaf_light_comp(shadowed, normal,  eye_space_pos, 0);
	if (enable_light1) color += add_leaf_light_comp(shadowed, normal,  eye_space_pos, 1);
	if (enable_light2) color += add_pt_light_comp  (normalize(normal), eye_space_pos, 2); // lightning

	if (enable_dlights) {
		vec3 vpos  = gl_Vertex.xyz + world_space_offset;
		color.rgb += add_dlights(vpos, nscale*normalize(gl_Normal), gl_ModelViewMatrixInverse[3].xyz, vec3(1.0)).rgb;
	}
	gl_FrontColor   = min(2*gl_Color, clamp(color*color_scale, 0.0, 1.0)); // limit lightning color
	gl_FogFragCoord = length(eye_space_pos.xyz);
}