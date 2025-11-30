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
#include "core/os/os.h"
#include "scene/main/node.h"
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

Error QModLoader::install_qmod(const String &p_qmod_path) {
	if (!FileAccess::exists(p_qmod_path)) {
		ERR_PRINT("QMOD installation failed: File does not exist: " + p_qmod_path);
		return ERR_FILE_NOT_FOUND;
	}

	// Open the QMOD file (PCK format)
	Ref<FileAccess> qmod_file = FileAccess::open(p_qmod_path, FileAccess::READ);
	if (qmod_file.is_null()) {
		ERR_PRINT("QMOD installation failed: Could not open file: " + p_qmod_path);
		return ERR_FILE_CANT_READ;
	}

	// Read PCK header
	uint32_t magic = qmod_file->get_32();
	if (magic != 0x43504447) { // "GDPC" in little endian
		ERR_PRINT("QMOD installation failed: Invalid file format");
		return ERR_FILE_UNRECOGNIZED;
	}

	uint32_t version = qmod_file->get_32();
	uint32_t ver_major = qmod_file->get_32();
	uint32_t ver_minor = qmod_file->get_32();
	uint32_t ver_patch = qmod_file->get_32();

	for (int i = 0; i < 16; i++) {
		qmod_file->get_32(); // Reserved
	}

	uint32_t file_count = qmod_file->get_32();

	// Read file entries
	Dictionary files;
	String mod_name;
	String scene_file;

	for (uint32_t i = 0; i < file_count; i++) {
		uint32_t path_len = qmod_file->get_32();
		Vector<uint8_t> path_data;
		path_data.resize(path_len);
		qmod_file->get_buffer(path_data.ptrw(), path_len);
		String path = String::utf8((const char *)path_data.ptr(), path_len);

		uint64_t offset = qmod_file->get_64();
		uint64_t size = qmod_file->get_64();
		qmod_file->get_buffer(nullptr, 16); // MD5

		files[path] = Array::make(offset, size);
	}

	// Extract mod.json first to get mod info
	if (!files.has("mod.json")) {
		ERR_PRINT("QMOD installation failed: mod.json not found in package");
		return ERR_FILE_CORRUPT;
	}

	Array json_info = files["mod.json"];
	uint64_t json_offset = json_info[0];
	uint64_t json_size = json_info[1];

	qmod_file->seek(json_offset);
	Vector<uint8_t> json_data;
	json_data.resize(json_size);
	qmod_file->get_buffer(json_data.ptrw(), json_size);
	String json_string = String::utf8((const char *)json_data.ptr(), json_size);

	JSON json;
	Error err = json.parse(json_string);
	if (err != OK) {
		ERR_PRINT("QMOD installation failed: Could not parse mod.json");
		return err;
	}

	Dictionary metadata = json.get_data();
	if (!metadata.has("title")) {
		ERR_PRINT("QMOD installation failed: mod.json missing 'title' field");
		return ERR_FILE_CORRUPT;
	}

	mod_name = String(metadata["title"]).replace(" ", "_").to_lower();

	// Create mods directory if it doesn't exist
	String mods_dir = get_mods_directory();
	Ref<DirAccess> dir = DirAccess::open("user://");
	if (!dir->dir_exists("mods")) {
		dir->make_dir("mods");
	}

	// Create mod directory
	String mod_dir = mods_dir + "/" + mod_name;
	dir = DirAccess::open(mods_dir);
	if (dir->dir_exists(mod_name)) {
		ERR_PRINT("QMOD installation: Mod already exists, overwriting: " + mod_name);
		// Could add proper uninstall here if needed
	} else {
		dir->make_dir(mod_name);
	}

	// Extract all files
	for (const Variant *key = files.next(nullptr); key; key = files.next(key)) {
		String file_path = *key;
		Array info = files[file_path];
		uint64_t offset = info[0];
		uint64_t size = info[1];

		qmod_file->seek(offset);
		Vector<uint8_t> file_data;
		file_data.resize(size);
		qmod_file->get_buffer(file_data.ptrw(), size);

		String output_path = mod_dir + "/" + file_path;
		Ref<FileAccess> output_file = FileAccess::open(output_path, FileAccess::WRITE);
		if (output_file.is_null()) {
			ERR_PRINT("QMOD installation: Could not write file: " + output_path);
			continue;
		}

		output_file->store_buffer(file_data.ptr(), size);
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

	// Remove all files in the mod directory
	dir->list_dir_begin();
	String file_name = dir->get_next();
	while (!file_name.is_empty()) {
		if (!dir->current_is_dir()) {
			dir->remove(file_name);
		}
		file_name = dir->get_next();
	}
	dir->list_dir_end();

	// Remove the directory itself
	Ref<DirAccess> parent_dir = DirAccess::open(get_mods_directory());
	Error err = parent_dir->remove(p_mod_name);

	if (err == OK) {
		print_line("QMOD uninstalled successfully: " + p_mod_name);
	}

	return err;
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
