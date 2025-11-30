#include "register_types.h"

#include "core/object/class_db.h"
#include "qmod_exporter.h"
#include "qmod_loader.h"

#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#include "qmod_editor_plugin.h"
#endif

void initialize_qmod_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(QModExporter);
		GDREGISTER_CLASS(QModLoader);
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<QModEditorPlugin>();
	}
#endif
}

void uninitialize_qmod_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Nothing to clean up for scene level.
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Nothing to clean up for editor level.
	}
#endif
}
