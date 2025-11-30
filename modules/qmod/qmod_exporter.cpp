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

 

Error QModExporter::_copy_file(const String &p_from, const String &p_to) {

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

 

	// Ensure output path ends with .qmod

	String output_dir = p_output_path;

	if (!output_dir.ends_with(".qmod")) {

		output_dir += ".qmod";

	}

 

	// Create output directory

	Ref<DirAccess> dir = DirAccess::create_for_path(output_dir);

	Error err = dir->make_dir_recursive(".");

	if (err != OK) {

		ERR_PRINT("QMOD export failed: Could not create output directory: " + output_dir);

		return err;

	}

 

	// Create metadata dictionary

	Dictionary metadata;

	metadata["title"] = title;

	metadata["description"] = description;

	metadata["icon"] = icon_path.get_file();

	metadata["type"] = mod_type == MOD_TYPE_LEVEL ? "level" : "character";

	metadata["scene"] = p_scene_path.get_file();

 

	// Write mod.json

	String json_text = JSON::stringify(metadata, "\t");

	String json_path = output_dir.path_join("mod.json");

	Ref<FileAccess> json_file = FileAccess::open(json_path, FileAccess::WRITE);

	if (json_file.is_null()) {

		ERR_PRINT("QMOD export failed: Could not write mod.json");

		return ERR_FILE_CANT_WRITE;

	}

	json_file->store_string(json_text);

 

	// Copy scene file

	String scene_dest = output_dir.path_join(p_scene_path.get_file());

	err = _copy_file(p_scene_path, scene_dest);

	if (err != OK) {

		ERR_PRINT("QMOD export failed: Could not copy scene file");

		return err;

	}

 

	// Copy icon if specified

	if (!icon_path.is_empty() && FileAccess::exists(icon_path)) {

		String icon_dest = output_dir.path_join(icon_path.get_file());

		err = _copy_file(icon_path, icon_dest);

		if (err != OK) {

			WARN_PRINT("Could not copy icon file, continuing anyway");

		}

	}

 

	print_line("QMOD exported successfully to: " + output_dir);

	return OK;

}

 

QModExporter::QModExporter() {

	title = "My Mod";

	description = "";

	icon_path = "";

	mod_type = MOD_TYPE_LEVEL;

}
