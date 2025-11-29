/**************************************************************************/
/*  game_view_plugin.h                                                    */
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

#include "../debugger/editor_debugger_node.h"
#include "../editor_plugin.h"

class GameViewDebugger : public RefCounted {
        GDCLASS(GameViewDebugger, RefCounted);

protected:
        static void _bind_methods();

public:
        void set_suspend(bool p_enabled);
        void next_frame();
        void set_node_type(const String &p_type);
        void set_select_mode(int p_mode);
        void set_selection_visible(bool p_visible);
        void set_camera_override(bool p_enabled);
        void set_camera_manipulate_mode(EditorDebuggerNode::CameraOverride p_mode);
        void reset_camera_2d_position();
        void reset_camera_3d_position();
        void set_debug_mute_audio(bool p_enabled);
        void reset_time_scale();
        void set_time_scale(double p_scale);
};

class GameViewPluginBase : public EditorPlugin {
        GDCLASS(GameViewPluginBase, EditorPlugin);

protected:
        Ref<GameViewDebugger> debugger;

public:
        GameViewPluginBase();
        Ref<GameViewDebugger> get_debugger() const { return debugger; }
};

class GameViewPlugin : public GameViewPluginBase {
        GDCLASS(GameViewPlugin, GameViewPluginBase);

public:
        GameViewPlugin();
};
