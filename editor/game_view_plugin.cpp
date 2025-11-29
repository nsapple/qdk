/**************************************************************************/
/*  game_view_plugin.cpp                                                  */
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

#include "run/game_view_plugin.h"

void GameViewDebugger::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_suspend", "enabled"), &GameViewDebugger::set_suspend);
        ClassDB::bind_method(D_METHOD("next_frame"), &GameViewDebugger::next_frame);
        ClassDB::bind_method(D_METHOD("set_node_type", "type"), &GameViewDebugger::set_node_type);
        ClassDB::bind_method(D_METHOD("set_select_mode", "mode"), &GameViewDebugger::set_select_mode);
        ClassDB::bind_method(D_METHOD("set_selection_visible", "visible"), &GameViewDebugger::set_selection_visible);
        ClassDB::bind_method(D_METHOD("set_camera_override", "enabled"), &GameViewDebugger::set_camera_override);
        ClassDB::bind_method(D_METHOD("set_camera_manipulate_mode", "mode"), &GameViewDebugger::set_camera_manipulate_mode);
        ClassDB::bind_method(D_METHOD("reset_camera_2d_position"), &GameViewDebugger::reset_camera_2d_position);
        ClassDB::bind_method(D_METHOD("reset_camera_3d_position"), &GameViewDebugger::reset_camera_3d_position);
        ClassDB::bind_method(D_METHOD("set_debug_mute_audio", "enabled"), &GameViewDebugger::set_debug_mute_audio);
        ClassDB::bind_method(D_METHOD("reset_time_scale"), &GameViewDebugger::reset_time_scale);
        ClassDB::bind_method(D_METHOD("set_time_scale", "scale"), &GameViewDebugger::set_time_scale);
}

void GameViewDebugger::set_suspend(bool p_enabled) {
        // Stub implementation for platform builds without a dedicated game view debugger.
        (void)p_enabled;
}

void GameViewDebugger::next_frame() {}

void GameViewDebugger::set_node_type(const String &p_type) {
        (void)p_type;
}

void GameViewDebugger::set_select_mode(int p_mode) {
        (void)p_mode;
}

void GameViewDebugger::set_selection_visible(bool p_visible) {
        (void)p_visible;
}

void GameViewDebugger::set_camera_override(bool p_enabled) {
        (void)p_enabled;
}

void GameViewDebugger::set_camera_manipulate_mode(EditorDebuggerNode::CameraOverride p_mode) {
        (void)p_mode;
}

void GameViewDebugger::reset_camera_2d_position() {}

void GameViewDebugger::reset_camera_3d_position() {}

void GameViewDebugger::set_debug_mute_audio(bool p_enabled) {
        (void)p_enabled;
}

void GameViewDebugger::reset_time_scale() {}

void GameViewDebugger::set_time_scale(double p_scale) {
        (void)p_scale;
}

GameViewPluginBase::GameViewPluginBase() {
        debugger.instantiate();
}

GameViewPlugin::GameViewPlugin() {
        set_plugin_name("Game");
}
