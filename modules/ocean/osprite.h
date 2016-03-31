/*************************************************************************/
/*  osprite.h                                                            */
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
#ifndef OSPRITE_H
#define OSPRITE_H

#include "scene/2d/node_2d.h"
#include "scene/resources/shape_2d.h"

class OSprite : public Node2D {

	OBJ_TYPE(OSprite, Node2D);
public:
	enum AnimationProcessMode {
		ANIMATION_PROCESS_FIXED,
		ANIMATION_PROCESS_IDLE,
	};
	enum CollisionMode {
		COLLISION_IGNORED,
		COLLISION_FISH,
		COLLISION_BULLET,
	};

	class OSpriteResource : public Resource {
		OBJ_TYPE(OSpriteResource, Resource);
	public:
		OSpriteResource();
		~OSpriteResource();

		Error load(const String& p_path);

		struct Action {
			String name;
			int from;
			int to;
		};

		struct Blocks {
			int index;
			Vector<Rect2> boxes;
		};

		struct Pool {
			int index;
			int frame;
			Rect2 rect;
			Rect2 shadow_rect;
		};

		struct Frame {
			Ref<Texture> tex;
			Rect2 region;
		};

		float fps_delta;
		float scale;
		Vector2 shadow_pos;
		float shadow_scale;
		Vector<Frame> frames;
		Vector<Action> actions;
		HashMap<String,Action*> action_names;
		Vector<Blocks> blocks;
		Vector<Pool> pools;
		int shown_frame;

	private:
		void _fixup_rects();
	};

private:

	Ref<OSpriteResource> res;

	float speed_scale;
	bool active;
	AnimationProcessMode animation_process_mode;
	CollisionMode collision_mode;
	bool playing;
	bool forward;
	bool debug_collisions;
	String current_animation;
	bool loop;

	Color modulate;
	Color shadow_color;
	bool flip_x, flip_y;

	float delay;
	float current_pos;

	void _dispose();
	void _animation_process(float p_delta);
	void _animation_draw();
	void _set_process(bool p_process, bool p_force = false);
	int _get_frame(OSpriteResource::Action *p_action = NULL) const;

protected:
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);

	static void _bind_methods();

public:

	// set/get osprite resource
	void set_resource(Ref<OSpriteResource> p_data);
	Ref<OSpriteResource> get_resource() const;

	bool has(const String& p_name) const;
	bool play(const String& p_name, bool p_loop = false, int p_delay = 0);
	void stop();
	bool is_playing(const String& p_name = "") const;
	void set_forward(bool p_forward = true);
	bool is_forward() const;
	String get_current_animation() const;
	void reset();
	void seek(float p_pos);
	float tell() const;

	void set_speed(float p_speed);
	float get_speed() const;

	void set_active(bool p_value);
	bool is_active() const;

	void set_modulate(const Color& p_color);
	Color get_modulate() const;

	void set_shadow_color(const Color& p_color);
	Color get_shadow_color() const;

	void set_flip_x(bool p_flip);
	void set_flip_y(bool p_flip);
	bool is_flip_x() const;
	bool is_flip_y() const;

	void set_animation_process_mode(AnimationProcessMode p_mode);
	AnimationProcessMode get_animation_process_mode() const;

	void set_collision_mode(CollisionMode p_mode);
	CollisionMode get_collision_mode() const;

	Array get_animation_names() const;

	void set_debug_collisions(bool p_enable);
	bool is_debug_collisions() const;

	virtual Rect2 get_item_rect() const;
	
	OSprite();
	virtual ~OSprite();
};

VARIANT_ENUM_CAST(OSprite::AnimationProcessMode);
VARIANT_ENUM_CAST(OSprite::CollisionMode);

#endif // OCEAN_H
#endif // MODULE_OCEAN_ENABLED