/*************************************************************************/
/*  spine.cpp                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifdef MODULE_OCEAN_ENABLED

//#include "method_bind_ext.inc"
#include "osprite.h"
#include "osprite_collision.h"

void OSprite::_dispose() {

	if (playing) {
		// stop first
		stop();
	}
	res = RES();
	update();
}

void OSprite::_animation_draw() {

	if (!res.is_valid())
		return;

	OSpriteResource::Action action;
	int index = _get_frame(&action);
	if(index == -1)
		return;

	if(playing) {

		if(!forward) {
			int tmp = action.from;
			action.from = action.to;
			action.to = tmp;
		}
		//printf("%d %d %d\n", action.from, action.to, index);

		if(index == action.from) {

			emit_signal("animation_start", action.name);

		} else if(index == action.to) {

			emit_signal("animation_end", action.name, !loop);
			if(!loop)
				set_process(false);
		}
	}

	const OSpriteResource::Pool& pool = res->pools[index];
	const OSpriteResource::Frame& frame = res->frames[pool.frame];
	if(pool.frame == -1)
		return;

	if(frame.tex.is_valid()) {

		const Rect2& rect = pool.rect;
		const Rect2& src_rect = frame.region;

		const Vector2& shadow_pos = res->shadow_pos;
		if(shadow_pos.x != 0 && shadow_pos.y != 0)
			draw_texture_rect_region(frame.tex, pool.shadow_rect, src_rect, shadow_color);
		draw_texture_rect_region(frame.tex, rect, src_rect, modulate);
	}

	if(debug_collisions && pool.frame < res->blocks.size()) {

		const OSpriteResource::Blocks& blocks = res->blocks[pool.frame];
		for(int i = 0; i < blocks.boxes.size(); i++) {

			const OSprite::Box& rect = blocks.boxes[i];
			static Color color = Color(0, 1, 1, 0.5);
			draw_circle(rect.pos, rect.radius, color);
		}
	}
}

void OSprite::_animation_process(float p_delta) {

	if (speed_scale == 0)
		return;
	current_pos += p_delta *speed_scale;

	update();
}

void OSprite::_set_process(bool p_process, bool p_force) {

	switch (animation_process_mode) {

	case ANIMATION_PROCESS_FIXED: set_fixed_process(p_process); break;
	case ANIMATION_PROCESS_IDLE: set_process(p_process); break;
	}
}

int OSprite::_get_frame(OSprite::OSpriteResource::Action *p_action) const {

	if(!res.is_valid())
		return -1;

	if(!has(current_animation))
		return res->shown_frame;

	OSpriteResource::Action *action = res->action_names[current_animation];
	ERR_FAIL_COND_V(action == NULL, -1);
	if(p_action != NULL)
		*p_action = *action;

	int total_frames = (action->to - action->from) + 1;
	frame = int(current_pos / res->fps_delta) % int(total_frames);
	if(forward)
		frame += action->from;
	else
		frame = action->to - frame;

	return frame;
}

bool OSprite::_set(const StringName& p_name, const Variant& p_value) {

	String name = p_name;

	if (name == "playback/play") {

		String which = p_value;
		if (res.is_valid()) {

			if (which == "[stop]")
				stop();
			else if (has(which)) {
				reset();
				play(which, loop);
			}
		} else
			current_animation = which;
	}
	else if (name == "playback/loop") {

		loop = p_value;
		if (res.is_valid() && has(current_animation))
			play(current_animation, loop);
	}
	else if (name == "playback/forward") {

		forward = p_value;
	}
	return true;
}

bool OSprite::_get(const StringName& p_name, Variant &r_ret) const {

	String name = p_name;

	if (name == "playback/play") {

		r_ret = current_animation;
	}
	else if (name == "playback/loop")
		r_ret = loop;
	else if (name == "playback/forward")
		r_ret = forward;

	return true;
}

void OSprite::_get_property_list(List<PropertyInfo> *p_list) const {

	List<String> names;

	if (res.is_valid()) {

		for(int i = 0; i < res->actions.size(); i++)
			names.push_back(res->actions[i].name);
	}
	{
		names.sort();
		names.push_front("[stop]");
		String hint;
		for(List<String>::Element *E = names.front(); E; E = E->next()) {

			if (E != names.front())
				hint += ",";
			hint += E->get();
		}

		p_list->push_back(PropertyInfo(Variant::STRING, "playback/play", PROPERTY_HINT_ENUM, hint));
		p_list->push_back(PropertyInfo(Variant::BOOL, "playback/loop", PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::BOOL, "playback/forward", PROPERTY_HINT_NONE));
	}
}

void OSprite::_notification(int p_what) {

	switch (p_what) {

	case NOTIFICATION_ENTER_TREE: {

		if (!active) {
			//make sure that a previous process state was not saved
			//only process if "processing" is set
			set_fixed_process(false);
			set_process(false);
		}
		OSpriteCollision::get_singleton()->add(this);

	} break;
	case NOTIFICATION_READY: {

		if (active && has(current_animation)) {
			play(current_animation, loop);
		}
	} break;
	case NOTIFICATION_PROCESS: {
		if (animation_process_mode == ANIMATION_PROCESS_FIXED)
			break;

		_animation_process(get_process_delta_time());
	} break;
	case NOTIFICATION_FIXED_PROCESS: {

		if (animation_process_mode == ANIMATION_PROCESS_IDLE)
			break;

		_animation_process(get_fixed_process_delta_time());
	} break;

	case NOTIFICATION_DRAW: {

		_animation_draw();
	} break;

	case NOTIFICATION_EXIT_TREE: {

		stop();
		OSpriteCollision::get_singleton()->remove(this);
	} break;
	}
}

void OSprite::set_resource(Ref<OSprite::OSpriteResource> p_data) {

	String anim = current_animation;
	// cleanup
	_dispose();

	res = p_data;
	if (res.is_null())
		return;

	if(has(anim))
		current_animation = anim;

	if (current_animation != "[stop]")
		play(current_animation, loop);
	else
		reset();

	_change_notify();
}

Ref<OSprite::OSpriteResource> OSprite::get_resource() const {

	return res;
}

bool OSprite::has(const String& p_name) const {

	ERR_FAIL_COND_V(!res.is_valid(), false);
	return res->action_names.has(p_name);
}

bool OSprite::play(const String& p_name, bool p_loop, int p_delay) {

	ERR_FAIL_COND_V(!res.is_valid(), false);
	ERR_FAIL_COND_V(!res->action_names.has(p_name), false);
	current_animation = p_name;
	playing = true;
	loop = p_loop;
	delay = p_delay;
	current_pos = 0;
	frame = 0;

	_set_process(true);

	return true;
}

void OSprite::stop() {

	_set_process(false);
	playing = false;
	//current_animation = "[stop]";
	reset();
}

bool OSprite::is_playing(const String& p_name) const {

	return playing && (p_name == "" || p_name == current_animation);
}

void OSprite::set_forward(bool p_forward) {

	forward = p_forward;
}

bool OSprite::is_forward() const {

	return forward;
}

String OSprite::get_current_animation() const {

	ERR_FAIL_COND_V(!res.is_valid(), "");
	return current_animation;
}

void OSprite::reset() {

	ERR_FAIL_COND(!res.is_valid());
	current_pos = 0;
}

void OSprite::seek(float p_pos) {

	_animation_process(p_pos - current_pos);
}

float OSprite::tell() const {

	return current_pos;
}

void OSprite::set_speed(float p_speed) {

	speed_scale = p_speed;
}

float OSprite::get_speed() const {

	return speed_scale;
}

void OSprite::set_active(bool p_value) {

	if (active == p_value)
		return;

	active = p_value;
	_set_process(active, true);
}

bool OSprite::is_active() const {

	return active;
}

void OSprite::set_modulate(const Color& p_color) {

	modulate = p_color;
	update();
}

Color OSprite::get_modulate() const{

	return modulate;
}

void OSprite::set_shadow_color(const Color& p_color) {

	shadow_color = p_color;
	update();
}

Color OSprite::get_shadow_color() const {

	return shadow_color;
}

void OSprite::set_flip_x(bool p_flip) {

	flip_x = p_flip;
	update();
}

void OSprite::set_flip_y(bool p_flip) {

	flip_y = p_flip;
	update();
}

bool OSprite::is_flip_x() const {

	return flip_x;
}

bool OSprite::is_flip_y() const {

	return flip_y;
}

Array OSprite::get_animation_names() const {

	ERR_FAIL_COND_V(!res.is_valid(), Array());

	Array names;
	for(int i = 0; i < res->actions.size(); i++)
		names.push_back(res->actions[i].name);
	return names;
}

void OSprite::set_animation_process_mode(OSprite::AnimationProcessMode p_mode) {

	if (animation_process_mode == p_mode)
		return;

	//bool pr = processing;
	bool pr = playing;
	if (pr)
		_set_process(false);
	animation_process_mode = p_mode;
	if (pr)
		_set_process(true);
}

OSprite::AnimationProcessMode OSprite::get_animation_process_mode() const {

	return animation_process_mode;
}

void OSprite::set_collision_mode(CollisionMode p_mode) {

	if(is_inside_tree())
		OSpriteCollision::get_singleton()->mode_changed(this, collision_mode, p_mode);
	collision_mode = p_mode;
}

OSprite::CollisionMode OSprite::get_collision_mode() const {

	return collision_mode;
}

void OSprite::set_debug_collisions(bool p_enable) {

	debug_collisions = p_enable;
	update();
}

bool OSprite::is_debug_collisions() const {

	return debug_collisions;
}

void OSprite::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_resource", "res"), &OSprite::set_resource);
	ObjectTypeDB::bind_method(_MD("get_resource"), &OSprite::get_resource);

	ObjectTypeDB::bind_method(_MD("has", "name"), &OSprite::has);
	ObjectTypeDB::bind_method(_MD("play", "name", "loop", "delay"), &OSprite::play, false, 0);
	ObjectTypeDB::bind_method(_MD("stop"), &OSprite::stop);
	ObjectTypeDB::bind_method(_MD("is_playing", "track"), &OSprite::is_playing, String(""));
	ObjectTypeDB::bind_method(_MD("get_current_animation"), &OSprite::get_current_animation);
	ObjectTypeDB::bind_method(_MD("reset"), &OSprite::reset);
	ObjectTypeDB::bind_method(_MD("seek", "pos"), &OSprite::seek);
	ObjectTypeDB::bind_method(_MD("tell"), &OSprite::tell);
	ObjectTypeDB::bind_method(_MD("set_active", "active"), &OSprite::set_active);
	ObjectTypeDB::bind_method(_MD("is_active"), &OSprite::is_active);
	ObjectTypeDB::bind_method(_MD("set_speed", "speed"), &OSprite::set_speed);
	ObjectTypeDB::bind_method(_MD("get_speed"), &OSprite::get_speed);
	ObjectTypeDB::bind_method(_MD("set_modulate", "modulate"), &OSprite::set_modulate);
	ObjectTypeDB::bind_method(_MD("get_modulate"), &OSprite::get_modulate);
	ObjectTypeDB::bind_method(_MD("set_shadow_color", "shadow_color"), &OSprite::set_shadow_color);
	ObjectTypeDB::bind_method(_MD("get_shadow_color"), &OSprite::get_shadow_color);
	ObjectTypeDB::bind_method(_MD("set_flip_x", "flip"), &OSprite::set_flip_x);
	ObjectTypeDB::bind_method(_MD("is_flip_x"), &OSprite::is_flip_x);
	ObjectTypeDB::bind_method(_MD("set_flip_y", "flip"), &OSprite::set_flip_y);
	ObjectTypeDB::bind_method(_MD("is_flip_y"), &OSprite::is_flip_y);
	ObjectTypeDB::bind_method(_MD("set_animation_process_mode","mode"),&OSprite::set_animation_process_mode);
	ObjectTypeDB::bind_method(_MD("get_animation_process_mode"),&OSprite::get_animation_process_mode);
	ObjectTypeDB::bind_method(_MD("set_collision_mode","mode"),&OSprite::set_collision_mode);
	ObjectTypeDB::bind_method(_MD("get_collision_mode"),&OSprite::get_collision_mode);
	ObjectTypeDB::bind_method(_MD("get_animation_names"), &OSprite::get_animation_names);

	ObjectTypeDB::bind_method(_MD("set_debug_collisions", "enable"), &OSprite::set_debug_collisions);
	ObjectTypeDB::bind_method(_MD("is_debug_collisions"), &OSprite::is_debug_collisions);

	ADD_PROPERTY( PropertyInfo( Variant::INT, "playback/process_mode", PROPERTY_HINT_ENUM, "Fixed,Idle"), _SCS("set_animation_process_mode"), _SCS("get_animation_process_mode"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback/speed", PROPERTY_HINT_RANGE, "-64,64,0.01"), _SCS("set_speed"), _SCS("get_speed"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playback/active"), _SCS("set_active"), _SCS("is_active"));

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "misc/show_collision"), _SCS("set_debug_collisions"), _SCS("is_debug_collisions"));
	ADD_PROPERTY( PropertyInfo( Variant::INT, "misc/collision_mode", PROPERTY_HINT_ENUM, "Ignored,Fish,Bullet"), _SCS("set_collision_mode"), _SCS("get_collision_mode"));

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "modulate"), _SCS("set_modulate"), _SCS("get_modulate"));
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "shadow_color"), _SCS("set_shadow_color"), _SCS("get_shadow_color"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_x"), _SCS("set_flip_x"), _SCS("is_flip_x"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_y"), _SCS("set_flip_y"), _SCS("is_flip_y"));
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "resource", PROPERTY_HINT_RESOURCE_TYPE, "OSpriteResource"), _SCS("set_resource"), _SCS("get_resource"));

	ADD_SIGNAL(MethodInfo("animation_start", PropertyInfo(Variant::STRING, "action")));
	ADD_SIGNAL(MethodInfo("animation_end", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::BOOL, "finish")));
	ADD_SIGNAL(MethodInfo("collision_enter", PropertyInfo(Variant::OBJECT, "owner"), PropertyInfo(Variant::OBJECT, "body")));
	ADD_SIGNAL(MethodInfo("collision_leave", PropertyInfo(Variant::OBJECT, "owner"), PropertyInfo(Variant::OBJECT, "body")));

	BIND_CONSTANT(ANIMATION_PROCESS_FIXED);
	BIND_CONSTANT(ANIMATION_PROCESS_IDLE);

	BIND_CONSTANT(COLLISION_IGNORED);
	BIND_CONSTANT(COLLISION_FISH);
	BIND_CONSTANT(COLLISION_BULLET);
}

Rect2 OSprite::get_item_rect() const {

	int frame = _get_frame();
	if (!res.is_valid() || frame == -1)
		return Node2D::get_item_rect();

	const OSpriteResource::Pool& pool = res->pools[frame];
	return pool.rect;
}

const Vector<OSprite::Box>& OSprite::get_collision() const {

	static Vector<OSprite::Box> empty;
	ERR_FAIL_COND_V(!res.is_valid(), empty);

	const OSpriteResource::Pool& pool = res->pools[frame];
	if(pool.frame >= res->blocks.size())
		return empty;
	const OSpriteResource::Blocks& blocks = res->blocks[pool.frame];
	return blocks.boxes;
}

OSprite::OSprite() {

	res = RES();

	speed_scale = 1;
	active = false;
	animation_process_mode = ANIMATION_PROCESS_IDLE;
	collision_mode = COLLISION_IGNORED;
	playing = false;
	forward = true;
	debug_collisions = false;
	current_animation = "[stop]";
	loop = true;

	modulate = Color(1, 1, 1, 1);
	shadow_color = Color(0, 0, 0, 0.6);
	flip_x = false;
	flip_y = false;
	current_pos = 0;
	frame = 0;
}

OSprite::~OSprite() {

	// cleanup
	_dispose();
}

#endif // MODULE_OCEAN_ENABLED