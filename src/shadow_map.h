// 3D World - Shared shadow map related classes
// by Frank Gennari
// 5/23/14

#ifndef _SHADOW_MAP_H_
#define _SHADOW_MAP_H_

#include "transform_obj.h" // for xform_matrix

unsigned const DEF_LOCAL_SMAP_SZ      = 1024;
unsigned const LOCAL_SMAP_START_TU_ID = 16;


struct smap_data_state_t {

	unsigned tid, fbo_id;
	smap_data_state_t() : tid(0), fbo_id(0) {}
	bool is_allocated() const {return (tid > 0);}
	void free_gl_state();
};

struct smap_data_t : public smap_data_state_t {

	unsigned tu_id, smap_sz;
	pos_dir_up pdu;
	point last_lpos;
	xform_matrix texture_matrix;

	smap_data_t(unsigned tu_id_, unsigned smap_sz_, smap_data_state_t const &init_state=smap_data_state_t())
		: smap_data_state_t(init_state), tu_id(tu_id_), smap_sz(smap_sz_), last_lpos(all_zeros) {}
	virtual ~smap_data_t() {} // free_gl_state()?
	bool set_smap_shader_for_light(shader_t &s, int light, xform_matrix const *const mvm=nullptr) const;
	void bind_smap_texture(bool light_valid=1) const;
	void create_shadow_map_for_light(point const &lpos, cube_t const *const bounds);
	virtual void render_scene_shadow_pass(point const &lpos) = 0;
	virtual bool needs_update(point const &lpos);
};

struct cached_dynamic_smap_data_t : public smap_data_t {

	bool last_has_dynamic;
	cached_dynamic_smap_data_t(unsigned tu_id_, unsigned smap_sz_) : smap_data_t(tu_id_, smap_sz_), last_has_dynamic(0) {}
};

struct local_smap_data_t : public cached_dynamic_smap_data_t {

	bool used;

	local_smap_data_t(unsigned tu_id_, unsigned smap_sz_=DEF_LOCAL_SMAP_SZ) : cached_dynamic_smap_data_t(tu_id_, smap_sz_), used(0) {}
	bool set_smap_shader_for_light(shader_t &s) const;
	virtual void render_scene_shadow_pass(point const &lpos);
	virtual bool needs_update(point const &lpos);
};

struct local_cube_map_smap_data_t : public local_smap_data_t { // to be implemented/used later
	local_cube_map_smap_data_t(unsigned tu_id_, unsigned smap_sz_=DEF_LOCAL_SMAP_SZ) : local_smap_data_t(tu_id_, smap_sz_) {}
};


template<class SD> struct vect_smap_t : public vector<SD> { // one per light source (sun, moon)
	void set_for_all_lights(shader_t &s, xform_matrix const *const mvm) const {
		for (unsigned i = 0; i < size(); ++i) {operator[](i).set_smap_shader_for_light(s, i, mvm);}
	}
	void clear() {
		for (iterator i = begin(); i != end(); ++i) {i->free_gl_state();}
		vector<SD>::clear();
	}
	void create_if_needed(cube_t const &bcube) {
		for (unsigned i = 0; i < size(); ++i) {
			point lpos;
			if (!light_valid_and_enabled(i, lpos)) continue;
			operator[](i).create_shadow_map_for_light(lpos, &bcube);
		}
	}
};


#endif // _SHADOW_MAP_H_


