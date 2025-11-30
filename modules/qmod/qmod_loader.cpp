/**************************************************************************/

/*  qmod_loader.cpp                                                       */

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

 

#include "qmod_loader.h"

 

#include "core/io/dir_access.h"

#include "core/io/file_access.h"

#include "core/io/json.h"

#include "core/io/resource_loader.h"

#include "scene/resources/packed_scene.h"

 

void QModLoader::_bind_methods() {

	ClassDB::bind_method(D_METHOD("install_qmod", "qmod_path"), &QModLoader::install_qmod);

	ClassDB::bind_method(D_METHOD("uninstall_qmod", "mod_name"), &QModLoader::uninstall_qmod);

	ClassDB::bind_method(D_METHOD("get_installed_mods"), &QModLoader::get_installed_mods);

	ClassDB::bind_method(D_METHOD("get_mod_info", "mod_name"), &QModLoader::get_mod_info);

	ClassDB::bind_method(D_METHOD("load_mod_scene", "mod_name"), &QModLoader::load_mod_scene);

	ClassDB::bind_static_method("QModLoader", D_METHOD("get_mods_directory"), &QModLoader::get_mods_directory);

}

 

String QModLoader::get_mods_directory() {

	return "user://mods";

}

 

Error QModLoader::_copy_dir(const String &p_from, const String &p_to) {

	Ref<DirAccess> src = DirAccess::open(p_from);

	if (src.is_null()) {

		ERR_PRINT("Could not open source directory: " + p_from);

		return ERR_FILE_CANT_READ;

	}

 

	Ref<DirAccess> dst = DirAccess::create_for_path(p_to);

	Error err = dst->make_dir_recursive(".");

	if (err != OK) {

		ERR_PRINT("Could not create destination directory: " + p_to);

		return err;

	}

 

	src->list_dir_begin();

	String file_name = src->get_next();

	while (!file_name.is_empty()) {

		if (file_name != "." && file_name != "..") {

			String src_path = p_from.path_join(file_name);

			String dst_path = p_to.path_join(file_name);

 

			if (src->current_is_dir()) {

				err = _copy_dir(src_path, dst_path);

				if (err != OK) {

					src->list_dir_end();

					return err;

				}

			} else {

				err = _copy_file(src_path, dst_path);

				if (err != OK) {

					src->list_dir_end();

					return err;

				}

			}

		}

		file_name = src->get_next();

	}

	src->list_dir_end();

 

	return OK;

}

 

Error QModLoader::_copy_file(const String &p_from, const String &p_to) {

	Ref<FileAccess> read = FileAccess::open(p_from, FileAccess::READ);

	if (read.is_null()) {

		ERR_PRINT("Could not open source file: " + p_from);

		return ERR_FILE_CANT_READ;

	}

 

	Ref<FileAccess> write = FileAccess::open(p_to, FileAccess::WRITE);

	if (write.is_null()) {

		ERR_PRINT("Could not create destination file: " + p_to);

		return ERR_FILE_CANT_WRITE;

	}

 

	uint64_t len = read->get_length();

	Vector<uint8_t> buffer;

	buffer.resize(len);

	read->get_buffer(buffer.ptrw(), len);

	write->store_buffer(buffer.ptr(), len);

 

	return OK;

}

 

Error QModLoader::install_qmod(const String &p_qmod_path) {

	Ref<DirAccess> qmod_dir = DirAccess::open(p_qmod_path);

	if (qmod_dir.is_null()) {

		ERR_PRINT("QMOD installation failed: Directory does not exist: " + p_qmod_path);

		return ERR_FILE_NOT_FOUND;

	}

 

	// Read mod.json to get mod name

	String json_path = p_qmod_path.path_join("mod.json");

	if (!FileAccess::exists(json_path)) {

		ERR_PRINT("QMOD installation failed: mod.json not found in: " + p_qmod_path);

		return ERR_FILE_NOT_FOUND;

	}

 

	Ref<FileAccess> json_file = FileAccess::open(json_path, FileAccess::READ);

	if (json_file.is_null()) {

		ERR_PRINT("QMOD installation failed: Could not read mod.json");

		return ERR_FILE_CANT_READ;

	}

 

	String json_string = json_file->get_as_text();

 

	JSON json;

	Error err = json.parse(json_string);

	if (err != OK) {

		ERR_PRINT("QMOD installation failed: Could not parse mod.json");

		return err;

	}

 

	Dictionary metadata = json.get_data();

	if (!metadata.has("title")) {

		ERR_PRINT("QMOD installation failed: mod.json missing 'title' field");

		return ERR_INVALID_DATA;

	}

 

	String mod_name = String(metadata["title"]).replace(" ", "_").to_lower();

 

	// Create mods directory if it doesn't exist

	String mods_dir = get_mods_directory();

	Ref<DirAccess> dir = DirAccess::open("user://");

	if (!dir->dir_exists("mods")) {

		dir->make_dir("mods");

	}

 

	// Create mod directory

	String mod_dir = mods_dir + "/" + mod_name;

 

	// Check if mod already exists

	if (DirAccess::exists(mod_dir)) {

		print_line("QMOD installation: Mod already exists, overwriting: " + mod_name);

		// Could delete old version here if needed

	}

 

	// Copy the entire qmod directory to the mods folder

	err = _copy_dir(p_qmod_path, mod_dir);

	if (err != OK) {

		ERR_PRINT("QMOD installation failed: Could not copy mod files");

		return err;

	}

 

	print_line("QMOD installed successfully: " + mod_name + " to " + mod_dir);

	return OK;

}

 

Error QModLoader::uninstall_qmod(const String &p_mod_name) {

	String mod_dir = get_mods_directory() + "/" + p_mod_name;

	Ref<DirAccess> dir = DirAccess::open(mod_dir);

 

	if (dir.is_null()) {

		ERR_PRINT("QMOD uninstall failed: Mod not found: " + p_mod_name);

		return ERR_FILE_NOT_FOUND;

	}

 

	// Remove all files in the mod directory recursively

	Error err = _remove_dir_recursive(mod_dir);

	if (err != OK) {

		ERR_PRINT("QMOD uninstall failed: Could not remove mod directory");

		return err;

	}

 

	print_line("QMOD uninstalled successfully: " + p_mod_name);

	return OK;

}

 

Error QModLoader::_remove_dir_recursive(const String &p_dir) {

	Ref<DirAccess> dir = DirAccess::open(p_dir);

	if (dir.is_null()) {

		return ERR_FILE_NOT_FOUND;

	}

 

	dir->list_dir_begin();

	String file_name = dir->get_next();

	while (!file_name.is_empty()) {

		if (file_name != "." && file_name != "..") {

			String path = p_dir.path_join(file_name);

			if (dir->current_is_dir()) {

				Error err = _remove_dir_recursive(path);

				if (err != OK) {

					dir->list_dir_end();

					return err;

				}

			} else {

				Error err = dir->remove(file_name);

				if (err != OK) {

					dir->list_dir_end();

					return err;

				}

			}

		}

		file_name = dir->get_next();

	}

	dir->list_dir_end();

 

	// Remove the directory itself

	Ref<DirAccess> parent_dir = DirAccess::open(p_dir.get_base_dir());

	return parent_dir->remove(p_dir.get_file());

}

 

Array QModLoader::get_installed_mods() {

	Array mods;

	String mods_dir = get_mods_directory();

 

	Ref<DirAccess> dir = DirAccess::open(mods_dir);

	if (dir.is_null()) {

		return mods;

	}

 

	dir->list_dir_begin();

	String dir_name = dir->get_next();

	while (!dir_name.is_empty()) {

		if (dir->current_is_dir() && !dir_name.begins_with(".")) {

			mods.append(dir_name);

		}

		dir_name = dir->get_next();

	}

	dir->list_dir_end();

 

	return mods;

}

 

Dictionary QModLoader::get_mod_info(const String &p_mod_name) {

	Dictionary info;

	String json_path = get_mods_directory() + "/" + p_mod_name + "/mod.json";

 

	Ref<FileAccess> file = FileAccess::open(json_path, FileAccess::READ);

	if (file.is_null()) {

		ERR_PRINT("Could not read mod.json for: " + p_mod_name);

		return info;

	}

 

	String json_string = file->get_as_text();

 

	JSON json;

	Error err = json.parse(json_string);

	if (err != OK) {

		ERR_PRINT("Could not parse mod.json for: " + p_mod_name);

		return info;

	}

 

	info = json.get_data();

	info["mod_directory"] = get_mods_directory() + "/" + p_mod_name;

 

	return info;

}

 

Error QModLoader::load_mod_scene(const String &p_mod_name) {

	Dictionary info = get_mod_info(p_mod_name);

	if (info.is_empty()) {

		return ERR_FILE_NOT_FOUND;

	}

 

	String scene_file = info.get("scene", "");

	if (scene_file.is_empty()) {

		ERR_PRINT("Mod scene file not specified in mod.json");

		return ERR_FILE_NOT_FOUND;

	}

 

	String scene_path = String(info["mod_directory"]) + "/" + scene_file;

 

	Ref<PackedScene> scene = ResourceLoader::load(scene_path);

	if (scene.is_null()) {

		ERR_PRINT("Could not load mod scene: " + scene_path);

		return ERR_FILE_CANT_READ;

	}

 

	// Scene loaded successfully - the user can instantiate it as needed

	print_line("Mod scene loaded successfully: " + scene_path);

	return OK;

}

 

QModLoader::QModLoader() {

}
