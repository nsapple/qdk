/**************************************************************************/
/*  editor_run_bar.cpp                                                    */
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

#include "editor_run_bar.h"

#include "core/config/project_settings.h"
#include "core/io/resource_uid.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"
#include "editor/gui/editor_toaster.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/button.h"
#include "scene/main/window.h"

EditorRunBar *EditorRunBar::singleton = nullptr;

void EditorRunBar::_bind_methods() {
        ADD_SIGNAL(MethodInfo("play_pressed", PropertyInfo(Variant::STRING, "scene"), PropertyInfo(Variant::STRING_ARRAY, "args"), PropertyInfo(Variant::BOOL, "from_native")));
        ADD_SIGNAL(MethodInfo("stop_pressed"));
}

EditorRunBar::EditorRunBar() {
        singleton = this;

        button_container = memnew(HBoxContainer);
        button_container->set_theme_type_variation("EditorRunBar");
        add_child(button_container);

        play_button = memnew(Button);
        play_button->set_toggle_mode(true);
        play_button->set_flat(true);
        play_button->set_focus_mode(FOCUS_NONE);
        play_button->set_tooltip_text(TTR("Play the main scene."));
        play_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::_play_button_pressed));
        button_container->add_child(play_button);

        pause_button = memnew(Button);
        pause_button->set_toggle_mode(true);
        pause_button->set_flat(true);
        pause_button->set_disabled(true);
        pause_button->set_focus_mode(FOCUS_NONE);
        pause_button->set_tooltip_text(TTR("Pause or unpause the running scene."));
        pause_button->connect(SceneStringName(toggled), callable_mp(this, &EditorRunBar::_pause_toggled));
        button_container->add_child(pause_button);

        stop_button = memnew(Button);
        stop_button->set_flat(true);
        stop_button->set_disabled(true);
        stop_button->set_focus_mode(FOCUS_NONE);
        stop_button->set_tooltip_text(TTR("Stop the currently playing scene."));
        stop_button->connect(SceneStringName(pressed), callable_mp(this, &EditorRunBar::_stop_button_pressed));
        button_container->add_child(stop_button);
}

EditorRunBar::~EditorRunBar() {
        if (singleton == this) {
                singleton = nullptr;
        }
}

EditorRunBar *EditorRunBar::get_singleton() {
        return singleton;
}

void EditorRunBar::_emit_play(const String &p_scene_path, const Vector<String> &p_args, bool p_from_native) {
        playing = true;
        playing_scene = p_scene_path;
        stop_button->set_disabled(false);
        pause_button->set_disabled(false);
        play_button->set_pressed(true);
        emit_signal(SceneStringName(play_pressed), p_scene_path, p_args, p_from_native);
}

void EditorRunBar::_play_button_pressed() {
        if (!playing) {
                play_main_scene();
        }
}

void EditorRunBar::_stop_button_pressed() {
        stop_playing();
}

void EditorRunBar::_pause_toggled(bool p_pressed) {
        if (!playing) {
                pause_button->set_pressed(false);
                pause_button->set_disabled(true);
                return;
        }
        EditorDebuggerNode::get_singleton()->set_breaked(p_pressed);
}

void EditorRunBar::play_main_scene(bool p_from_native, const Vector<String> &p_args) {
        String main_scene = ProjectSettings::get_singleton()->get("application/run/main_scene");
        if (main_scene.is_empty()) {
                EditorToaster::get_singleton()->popup_warning(TTR("No main scene has been defined."));
                return;
        }
        play_custom_scene(ResourceUID::get_singleton()->id_to_text(ResourceUID::get_singleton()->text_to_id(main_scene)), p_args, p_from_native);
}

void EditorRunBar::play_current_scene(bool p_from_native, const Vector<String> &p_args) {
        const EditorData &data = EditorNode::get_editor_data();
        int current = data.get_edited_scene();
        String scene_path = data.get_scene_path(current);
        if (scene_path.is_empty()) {
                EditorToaster::get_singleton()->popup_warning(TTR("Current scene was never saved."));
                return;
        }
        play_custom_scene(scene_path, p_args, p_from_native);
}

void EditorRunBar::play_custom_scene(const String &p_scene, const Vector<String> &p_args, bool p_from_native) {
        playing_scene = p_scene;
        _emit_play(p_scene, p_args, p_from_native);
}

void EditorRunBar::stop_playing() {
        if (!playing) {
                return;
        }
        playing = false;
        playing_scene = String();
        play_button->set_pressed(false);
        pause_button->set_pressed(false);
        pause_button->set_disabled(true);
        stop_button->set_disabled(true);
        emit_signal(SceneStringName(stop_pressed));
}

bool EditorRunBar::is_playing() const {
        return playing;
}

String EditorRunBar::get_playing_scene() const {
        return playing_scene;
}

Button *EditorRunBar::get_pause_button() const {
        return pause_button;
}

HBoxContainer *EditorRunBar::get_buttons_container() const {
        return button_container;
}

void EditorRunBar::set_movie_maker_enabled(bool p_enabled) {
        movie_maker_enabled = p_enabled;
}

bool EditorRunBar::is_movie_maker_enabled() const {
        return movie_maker_enabled;
}

void EditorRunBar::update_profiler_autostart_indicator() {
        // Profiler hint is not implemented in this fork; method kept for compatibility.
}
