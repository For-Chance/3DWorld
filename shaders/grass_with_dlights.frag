uniform vec4 color_scale = vec4(1.0);
uniform sampler2D tex0;

varying vec3 dlpos, normal; // world space
varying vec3 eye_norm;
varying vec2 tc;

void main()
{
#ifdef NO_GRASS_TEXTURE
	vec4 texel = vec4(1.0);
#else
	vec4 texel = texture2D(tex0, tc);
#ifdef ENABLE_ALPHA_TEST
	if (texel.a == 0.0) discard;
#endif
#endif

	// calculate lighting: L0-L1 is directional
	vec4 epos  = fg_ModelViewMatrix * vec4(dlpos, 1.0);
	vec3 color = vec3(0.0);
	if (enable_light0 ) color += add_light_comp_pos_smap_light0(eye_norm, epos).rgb;
	if (enable_light1 ) color += add_light_comp_pos_smap_light1(eye_norm, epos).rgb;
	if (enable_dlights) color += gl_Color.rgb * add_dlights(dlpos, normal, vec3(1.0)); // dynamic lighting
	fg_FragColor = texel*color_scale*vec4(color, gl_Color.a);
#ifndef NO_FOG
	fg_FragColor = apply_fog_epos(fg_FragColor, epos);
#endif
}
