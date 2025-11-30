/**************************************************************************/
/*  qmod_exporter.cpp                                                     */
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

#include "qmod_exporter.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/io/pck_packer.h"
#include "core/io/resource_loader.h"

void QModExporter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_title", "title"), &QModExporter::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &QModExporter::get_title);

	ClassDB::bind_method(D_METHOD("set_description", "description"), &QModExporter::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &QModExporter::get_description);

	ClassDB::bind_method(D_METHOD("set_icon_path", "icon_path"), &QModExporter::set_icon_path);
	ClassDB::bind_method(D_METHOD("get_icon_path"), &QModExporter::get_icon_path);

	ClassDB::bind_method(D_METHOD("set_mod_type", "type"), &QModExporter::set_mod_type);
	ClassDB::bind_method(D_METHOD("get_mod_type"), &QModExporter::get_mod_type);

	ClassDB::bind_method(D_METHOD("export_qmod", "scene_path", "output_path"), &QModExporter::export_qmod);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "icon_path", PROPERTY_HINT_FILE, "*.png,*.jpg,*.svg"), "set_icon_path", "get_icon_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mod_type", PROPERTY_HINT_ENUM, "Level,Character"), "set_mod_type", "get_mod_type");

	BIND_ENUM_CONSTANT(MOD_TYPE_LEVEL);
	BIND_ENUM_CONSTANT(MOD_TYPE_CHARACTER);
}

void QModExporter::set_title(const String &p_title) {
	title = p_title;
}

String QModExporter::get_title() const {
	return title;
}

void QModExporter::set_description(const String &p_description) {
	description = p_description;
}

String QModExporter::get_description() const {
	return description;
}

void QModExporter::set_icon_path(const String &p_icon_path) {
	icon_path = p_icon_path;
}

String QModExporter::get_icon_path() const {
	return icon_path;
}

void QModExporter::set_mod_type(ModType p_type) {
	mod_type = p_type;
}

QModExporter::ModType QModExporter::get_mod_type() const {
	return mod_type;
}

Error QModExporter::export_qmod(const String &p_scene_path, const String &p_output_path) {
	// Validate inputs
	if (title.is_empty()) {
		ERR_PRINT("QMOD export failed: Title cannot be empty");
		return ERR_INVALID_PARAMETER;
	}

	if (!FileAccess::exists(p_scene_path)) {
		ERR_PRINT("QMOD export failed: Scene file does not exist: " + p_scene_path);
		return ERR_FILE_NOT_FOUND;
	}

	// Create metadata JSON
	Dictionary metadata;
	metadata["title"] = title;
	metadata["description"] = description;
	metadata["icon"] = icon_path.get_file();
	metadata["type"] = mod_type == MOD_TYPE_LEVEL ? "level" : "character";
	metadata["scene"] = p_scene_path.get_file();

	String json_string = JSON::stringify(metadata, "\t");

	// Create PCK packer
	Ref<PCKPacker> packer;
	packer.instantiate();

	Error err = packer->pck_start(p_output_path);
	if (err != OK) {
		ERR_PRINT("QMOD export failed: Could not create output file: " + p_output_path);
		return err;
	}

	// Add metadata JSON
	Vector<uint8_t> json_data;
	json_data.resize(json_string.length());
	memcpy(json_data.ptrw(), json_string.utf8().get_data(), json_string.length());
	packer->add_file("mod.json", json_data);

	// Add scene file
	Ref<FileAccess> scene_file = FileAccess::open(p_scene_path, FileAccess::READ);
	if (scene_file.is_null()) {
		ERR_PRINT("QMOD export failed: Could not read scene file: " + p_scene_path);
		packer->flush();
		return ERR_FILE_CANT_READ;
	}

	Vector<uint8_t> scene_data;
	uint64_t scene_size = scene_file->get_length();
	scene_data.resize(scene_size);
	scene_file->get_buffer(scene_data.ptrw(), scene_size);
	packer->add_file(p_scene_path.get_file(), scene_data);

	// Add icon if specified
	if (!icon_path.is_empty() && FileAccess::exists(icon_path)) {
		Ref<FileAccess> icon_file = FileAccess::open(icon_path, FileAccess::READ);
		if (icon_file.is_valid()) {
			Vector<uint8_t> icon_data;
			uint64_t icon_size = icon_file->get_length();
			icon_data.resize(icon_size);
			icon_file->get_buffer(icon_data.ptrw(), icon_size);
			packer->add_file(icon_path.get_file(), icon_data);
		}
	}

	err = packer->flush();
	if (err != OK) {
		ERR_PRINT("QMOD export failed: Could not finalize pack file");
		return err;
	}

	print_line("QMOD exported successfully to: " + p_output_path);
	return OK;
}

QModExporter::QModExporter() {
	title = "My Mod";
	description = "";
	icon_path = "";
	mod_type = MOD_TYPE_LEVEL;
}
