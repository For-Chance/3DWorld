// http://www.opengl.org/resources/features/KilgardTechniques/LensFlare/
#include "function_registry.h"
#include "textures_3dw.h"

#include <stdio.h>
#include <stdlib.h>
#include <sstream>


struct Flare {

	int type; // flare texture index, 0..5
	float loc, scale;
	colorRGBA color;

	Flare(int type_, float loc_, float scale_, colorRGBA const &color_)
		: type(type_), loc(loc_), scale(scale_), color(color_) {}
};


unsigned const NUM_FLARE = 6;
unsigned const NUM_SHINE = 1;
int const num_flares     = 8;
int const num_flare_tex  = NUM_FLARE + NUM_SHINE;


bool tex_loaded(0);
GLuint flareTex[NUM_FLARE], shineTex[NUM_SHINE];
unsigned char *ft_buf[num_flare_tex];
int ft_width[num_flare_tex], ft_height[num_flare_tex], ft_components[num_flare_tex];


extern float brightness;
extern colorRGBA sun_color;


Flare flare[num_flares] = {
	Flare(-1, 1.0,   1.5, 	 colorRGBA(1.0, 0.7,  0.2)),
	Flare(-1, 1.0,   1.0, 	 colorRGBA(0.5, 0.35, 0.1)),
	Flare(-1, 1.0,   0.8, 	 colorRGBA(1.0, 0.7,  0.2)),
	Flare(0,  1.0,   1.0, 	 colorRGBA(0.2, 0.1,  0.0)),
	Flare(3,  0.004, 0.003,  colorRGBA(0.8, 0.2,  0.0)),
	Flare(3,  0.008, 0.005,  colorRGBA(0.9, 0.3,  0.0)),
	Flare(3,  0.025, 0.0055, colorRGBA(0.9, 0.4,  0.0)),
	Flare(0,  0.035, 0.01,   colorRGBA(0.2, 0.08, 0.0)),
};


unsigned char *load_luminance(std::string const &filename, int *width, int *height, int *components);



void DoFlares(point const &from, point const &at, point const &light, float near_clip, float size, float intensity, int start_ix) {

	if (brightness == 0.0) return;
	assert(brightness > 0.0);
	if (!tex_loaded) load_flare_textures();
	float global_scale(6.0);
	GLuint bound_to(0);

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
	enable_blend();
	glBlendFunc(GL_ONE, GL_ONE);

	// view_dir = normalize(at-from)
	vector3d const view_dir(vector3d(at, from).get_norm());

	// center = from + near_clip * view_dir
	point center(view_dir*near_clip + from);

	// axis = light - center
	vector3d const axis(light, center), dx2(axis.get_norm());
	vector3d const dx(vector3d(1.0, 1.0, -(dx2[0] + dx2[1])/dx2[2]).get_norm());

	// dy = cross(dx,view_dir)
	vector3d const dy(cross_product(dx2, dx).get_norm());
	float const cscale(intensity*pow((double)max(brightness, 0.6f), 1.5));

	for (int i = start_ix; i < num_flares; i++) {
		float const scale(flare[i].scale * global_scale * size);
		vector3d const sx(dx*scale), sy(dy*scale);
		colorRGBA c(flare[i].color);

		for (unsigned d = 0; d < 3; ++d) {
			c[d] += (sun_color[d] - SUN_LT_C[d]);
			CLIP_TO_01(c[d]);
		}
		c *= cscale;
		c.do_glColor();

		// Note logic below to eliminate duplicate texture binds.
		if (flare[i].type < 0 || bound_to != flareTex[flare[i].type]) {
			if (bound_to) glEnd();
			bound_to = ((flare[i].type < 0) ? shineTex[0] : flareTex[flare[i].type]);
			glBindTexture(GL_TEXTURE_2D, bound_to);
			glBegin(GL_TRIANGLES);
		}
		draw_billboard_quad((axis*flare[i].loc + center), sx, sy);
	}
	if (bound_to) glEnd();
	glEnable(GL_DEPTH_TEST);
	disable_blend();
	set_std_blend_mode();
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	//glEnable(GL_DITHER);
}


void gen_texture(std::string const &filename, GLuint &tid, int &id, bool init) {

	if (init) ft_buf[id] = load_luminance(filename, &ft_width[id], &ft_height[id], &ft_components[id]);
	setup_texture(tid, GL_MODULATE, 0, 0, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, ft_width[id], ft_height[id], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ft_buf[id]);
	id++;
}


void load_flare_textures() {

	int id(0);
	static bool init(1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned i = 0; i < NUM_SHINE; i++) {
		std::ostringstream oss;
		oss << "Shine" << (i+1) << ".bw";
		gen_texture(oss.str(), shineTex[i], id, init);
	}
	for (unsigned i = 0; i < NUM_FLARE; i++) {
		std::ostringstream oss;
		oss << "Flare" << (i+1) << ".bw";
		gen_texture(oss.str(), flareTex[i], id, init);
	}
	init       = 0;
	tex_loaded = 1;
}


void free_flare_textures() {

	for (unsigned i = 0; i < NUM_SHINE; i++) {
		free_texture(shineTex[i]);
	}
	for (unsigned i = 0; i < NUM_FLARE; i++) {
		free_texture(flareTex[i]);
	}
	tex_loaded = 0;
}

