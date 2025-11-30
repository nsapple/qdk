/**************************************************************************/
/*  qmod_exporter.h                                                       */
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

#ifndef QMOD_EXPORTER_H
#define QMOD_EXPORTER_H

#include "core/io/resource_saver.h"
#include "core/object/ref_counted.h"

class QModExporter : public RefCounted {
	GDCLASS(QModExporter, RefCounted);

public:
	enum ModType {
		MOD_TYPE_LEVEL,
		MOD_TYPE_CHARACTER,
	};

private:
        String title;
        String description;
        String icon_path;
        ModType mod_type = MOD_TYPE_LEVEL;

        Error _copy_file(const String &p_from, const String &p_to);

protected:
	static void _bind_methods();

public:
	void set_title(const String &p_title);
	String get_title() const;

	void set_description(const String &p_description);
	String get_description() const;

	void set_icon_path(const String &p_icon_path);
	String get_icon_path() const;

	void set_mod_type(ModType p_type);
	ModType get_mod_type() const;

	Error export_qmod(const String &p_scene_path, const String &p_output_path);

	QModExporter();
};

VARIANT_ENUM_CAST(QModExporter::ModType);

#endif // QMOD_EXPORTER_H
