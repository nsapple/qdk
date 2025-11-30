/**************************************************************************/
/*  editor_run_bar.h                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"

class EditorRunBar : public HBoxContainer {
        GDCLASS(EditorRunBar, HBoxContainer);

        static EditorRunBar *singleton;

        Button *play_button = nullptr;
        Button *pause_button = nullptr;
        Button *stop_button = nullptr;
        HBoxContainer *button_container = nullptr;

        bool playing = false;
        bool movie_maker_enabled = false;
        String playing_scene;

        void _emit_play(const String &p_scene_path, const Vector<String> &p_args, bool p_from_native);

protected:
        static void _bind_methods();
        void _play_button_pressed();
        void _stop_button_pressed();
        void _pause_toggled(bool p_pressed);

public:
        static EditorRunBar *get_singleton();

        EditorRunBar();
        ~EditorRunBar();

        void play_main_scene(bool p_from_native = false, const Vector<String> &p_args = Vector<String>());
        void play_current_scene(bool p_from_native = false, const Vector<String> &p_args = Vector<String>());
        void play_custom_scene(const String &p_scene, const Vector<String> &p_args = Vector<String>(), bool p_from_native = false);
        void stop_playing();

        bool is_playing() const;
        String get_playing_scene() const;

        Button *get_pause_button() const;
        HBoxContainer *get_buttons_container() const;

        void set_movie_maker_enabled(bool p_enabled);
        bool is_movie_maker_enabled() const;

        void update_profiler_autostart_indicator();
};
