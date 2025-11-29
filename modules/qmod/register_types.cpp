#include "register_types.h"

#include "core/config/engine.h"

#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#include "qmod_editor_plugin.h"
#endif

void initialize_qmod_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
        if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
                EditorPlugins::add_by_type<QModEditorPlugin>();
        }
#endif
}

void uninitialize_qmod_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
        if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
                // Nothing to clean up.
        }
#endif
}
