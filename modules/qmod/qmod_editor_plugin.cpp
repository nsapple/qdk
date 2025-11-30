#include "qmod_editor_plugin.h"

#ifdef TOOLS_ENABLED

#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/os/os.h"
#include "editor/editor_data.h"
#include "editor/gui/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/editor_paths.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/separator.h"
#include "scene/gui/text_edit.h"

void QModEditorPlugin::_open_export_dialog() {
        export_dialog->popup_centered_ratio(0.35);
}

void QModEditorPlugin::_open_import_dialog() {
        import_dialog->popup_file_dialog();
}

void QModEditorPlugin::_open_icon_picker() {
        pending_dialog = FILE_DIALOG_ICON;
        file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
        file_dialog->clear_filters();
        file_dialog->add_filter("*.png;*.jpg;*.jpeg;*.svg", TTR("Images"));
        file_dialog->popup_file_dialog();
}

void QModEditorPlugin::_open_scene_picker() {
        pending_dialog = FILE_DIALOG_SCENE;
        file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
        file_dialog->clear_filters();
        file_dialog->add_filter("*.tscn;*.scn", TTR("Scenes"));
        file_dialog->popup_file_dialog();
}

void QModEditorPlugin::_open_output_picker() {
        pending_dialog = FILE_DIALOG_OUTPUT;
        file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_DIR);
        file_dialog->clear_filters();
        file_dialog->popup_file_dialog();
}

void QModEditorPlugin::_dialog_file_selected(const String &p_path) {
        switch (pending_dialog) {
                case FILE_DIALOG_ICON:
                        icon_edit->set_text(p_path);
                        break;
                case FILE_DIALOG_SCENE:
                        scene_edit->set_text(p_path);
                        break;
                case FILE_DIALOG_OUTPUT:
                        output_edit->set_text(_resolve_mod_folder(p_path));
                        break;
                default:
                        break;
        }
}

void QModEditorPlugin::_import_file_selected(const String &p_path) {
        String mods_dir = OS::get_singleton()->get_user_data_dir().path_join("mods");
        DirAccess::create(mods_dir);

        String target_dir = mods_dir.path_join(p_path.get_file());
        Ref<DirAccess> dir = DirAccess::create_for_path(target_dir);
        if (dir.is_valid() && dir->dir_exists(target_dir)) {
                dir->remove_recursive(target_dir);
        }
        _copy_dir(p_path, target_dir);
}

Error QModEditorPlugin::_copy_dir(const String &p_from, const String &p_to) {
        Ref<DirAccess> src = DirAccess::create_for_path(p_from);
        ERR_FAIL_COND_V(src.is_null(), ERR_CANT_OPEN);
        Ref<DirAccess> dst = DirAccess::create_for_path(p_to);
        ERR_FAIL_COND_V(dst.is_null(), ERR_CANT_CREATE);
        dst->make_dir_recursive(".");

        src->list_dir_begin();
        String item;
        while (!(item = src->get_next()).is_empty()) {
                if (item == "." || item == "..") {
                        continue;
                }
                const String src_path = p_from.path_join(item);
                const String dst_path = p_to.path_join(item);
                if (src->current_is_dir()) {
                        Error err = _copy_dir(src_path, dst_path);
                        if (err != OK) {
                                src->list_dir_end();
                                return err;
                        }
                } else {
                        Error err = _copy_file(src_path, dst_path);
                        if (err != OK) {
                                src->list_dir_end();
                                return err;
                        }
                }
        }
        src->list_dir_end();
        return OK;
}

Error QModEditorPlugin::_copy_file(const String &p_from, const String &p_to) {
        Ref<FileAccess> read = FileAccess::open(p_from, FileAccess::READ);
        ERR_FAIL_COND_V(read.is_null(), ERR_CANT_OPEN);

        Ref<FileAccess> write = FileAccess::open(p_to, FileAccess::WRITE);
        ERR_FAIL_COND_V(write.is_null(), ERR_CANT_OPEN);

        const uint64_t len = read->get_length();
        Vector<uint8_t> buffer;
        buffer.resize(len);
        read->get_buffer(buffer.ptrw(), len);
        write->store_buffer(buffer.ptr(), len);
        return OK;
}

Error QModEditorPlugin::_write_json(const String &p_path, const Dictionary &p_dict) {
        String json_text = JSON::stringify(p_dict, "\t");
        Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
        ERR_FAIL_COND_V(file.is_null(), ERR_CANT_CREATE);
        file->store_string(json_text);
        return OK;
}

String QModEditorPlugin::_resolve_mod_folder(const String &p_target) const {
        if (p_target.ends_with(".qmod")) {
                return p_target;
        }
        return p_target + ".qmod";
}

void QModEditorPlugin::_confirm_export() {
        String title = title_edit->get_text().strip_edges();
        String description = description_edit->get_text().strip_edges();
        String icon_path = icon_edit->get_text().strip_edges();
        String scene_path = scene_edit->get_text().strip_edges();
        String output_path = output_edit->get_text().strip_edges();

        if (scene_path.is_empty()) {
                const EditorData &data = EditorNode::get_editor_data();
                scene_path = data.get_scene_path(data.get_edited_scene());
        }

        if (output_path.is_empty()) {
                output_path = EditorPaths::get_singleton()->get_project_settings_dir().path_join("mods");
                DirAccess::create(output_path);
                output_path = output_path.path_join(title.is_empty() ? String("new_mod") : title);
        }

        output_path = _resolve_mod_folder(output_path);

        Ref<DirAccess> dir = DirAccess::create_for_path(output_path);
        dir->make_dir_recursive(".");

        Dictionary metadata;
        metadata["title"] = title;
        metadata["description"] = description;
        metadata["icon"] = icon_path.get_file();
        metadata["type"] = type_option->get_selected_id() == 0 ? "level" : "character";

        String json_path = output_path.path_join("mod.json");
        _write_json(json_path, metadata);

        if (!icon_path.is_empty()) {
                _copy_file(icon_path, output_path.path_join(icon_path.get_file()));
        }

        if (!scene_path.is_empty()) {
                _copy_file(scene_path, output_path.path_join(scene_path.get_file()));
        }
}

void QModEditorPlugin::_notification(int p_what) {
        if (p_what == NOTIFICATION_ENTER_TREE) {
                export_dialog = memnew(ConfirmationDialog);
                export_dialog->set_title(TTR("Export QMod"));
                add_child(export_dialog);

                VBoxContainer *main_vb = memnew(VBoxContainer);
                export_dialog->add_child(main_vb);

                auto add_labeled = [&](const String &p_label, Control *p_field) {
                        HBoxContainer *hb = memnew(HBoxContainer);
                        Label *lbl = memnew(Label);
                        lbl->set_text(p_label);
                        hb->add_child(lbl);
                        hb->add_child(p_field);
                        p_field->set_h_size_flags(Control::SIZE_EXPAND_FILL);
                        main_vb->add_child(hb);
                };

                title_edit = memnew(LineEdit);
                add_labeled(TTR("Title"), title_edit);

                description_edit = memnew(TextEdit);
                description_edit->set_custom_minimum_size(Size2(0, 80 * EDSCALE));
                add_labeled(TTR("Description"), description_edit);

                icon_edit = memnew(LineEdit);
                Button *icon_button = memnew(Button);
                icon_button->set_text(TTR("Browse"));
                icon_button->connect(SceneStringName(pressed), callable_mp(this, &QModEditorPlugin::_open_icon_picker));

                HBoxContainer *icon_hb = memnew(HBoxContainer);
                icon_hb->add_child(icon_edit);
                icon_hb->add_child(icon_button);
                main_vb->add_child(icon_hb);

                scene_edit = memnew(LineEdit);
                Button *scene_button = memnew(Button);
                scene_button->set_text(TTR("Pick Scene"));
                scene_button->connect(SceneStringName(pressed), callable_mp(this, &QModEditorPlugin::_open_scene_picker));
                HBoxContainer *scene_hb = memnew(HBoxContainer);
                scene_hb->add_child(scene_edit);
                scene_hb->add_child(scene_button);
                main_vb->add_child(scene_hb);

                output_edit = memnew(LineEdit);
                Button *output_button = memnew(Button);
                output_button->set_text(TTR("Choose Output"));
                output_button->connect(SceneStringName(pressed), callable_mp(this, &QModEditorPlugin::_open_output_picker));
                HBoxContainer *output_hb = memnew(HBoxContainer);
                output_hb->add_child(output_edit);
                output_hb->add_child(output_button);
                main_vb->add_child(output_hb);

                type_option = memnew(OptionButton);
                type_option->add_item(TTR("Level"), 0);
                type_option->add_item(TTR("Character"), 1);
                add_labeled(TTR("Type"), type_option);

                export_dialog->connect(SceneStringName(confirmed), callable_mp(this, &QModEditorPlugin::_confirm_export));

                file_dialog = memnew(EditorFileDialog);
                file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
                file_dialog->connect(SceneStringName(file_selected), callable_mp(this, &QModEditorPlugin::_dialog_file_selected));
                add_child(file_dialog);

                import_dialog = memnew(EditorFileDialog);
                import_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_DIR);
                import_dialog->connect(SceneStringName(file_selected), callable_mp(this, &QModEditorPlugin::_import_file_selected));
                add_child(import_dialog);

                add_tool_menu_item(TTR("Export QMod"), callable_mp(this, &QModEditorPlugin::_open_export_dialog));
                add_tool_menu_item(TTR("Import QMod"), callable_mp(this, &QModEditorPlugin::_open_import_dialog));
        }
}

#endif
