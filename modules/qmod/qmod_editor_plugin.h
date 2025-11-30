/**************************************************************************/
/*  qmod_editor_plugin.h                                                  */
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

#include "editor/plugins/editor_plugin.h"

#ifdef TOOLS_ENABLED
class Button;
class ConfirmationDialog;
class EditorFileDialog;
class LineEdit;
class OptionButton;
class TextEdit;

class QModEditorPlugin : public EditorPlugin {
        GDCLASS(QModEditorPlugin, EditorPlugin);

        ConfirmationDialog *export_dialog = nullptr;
        EditorFileDialog *file_dialog = nullptr;
        EditorFileDialog *import_dialog = nullptr;

        LineEdit *title_edit = nullptr;
        TextEdit *description_edit = nullptr;
        LineEdit *icon_edit = nullptr;
        LineEdit *scene_edit = nullptr;
        LineEdit *output_edit = nullptr;
        OptionButton *type_option = nullptr;

        enum FileDialogPurpose {
                FILE_DIALOG_ICON,
                FILE_DIALOG_SCENE,
                FILE_DIALOG_OUTPUT,
        };

        FileDialogPurpose pending_dialog = FILE_DIALOG_ICON;

        void _open_export_dialog();
        void _open_import_dialog();
        void _open_icon_picker();
        void _open_scene_picker();
        void _open_output_picker();
        void _dialog_file_selected(const String &p_path);
        void _import_file_selected(const String &p_path);
        void _confirm_export();
        Error _copy_dir(const String &p_from, const String &p_to);
        Error _copy_file(const String &p_from, const String &p_to);
        Error _write_json(const String &p_path, const Dictionary &p_dict);
        String _resolve_mod_folder(const String &p_target) const;

public:
        String get_plugin_name() const override { return "QModExporter"; }
        bool has_main_screen() const override { return false; }
        void _notification(int p_what);
};
#endif
