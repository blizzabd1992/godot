#ifndef BEHAVIOR_LIBRARY_REGISTER_TYPES_H
#define BEHAVIOR_LIBRARY_REGISTER_TYPES_H

#define BEHAVIOR_LIBRARY_REGISTER_TYPES_H

void register_behavior_tree_types();
void unregister_behavior_tree_types();

#if defined(TOOLS_ENABLED) && defined(MODULE_BEHAVIOR_TREE_ENABLED)
	#define MODULE_BEHAVIOR_TREE_ICONS_ENABLED
	#include "editor_icons.h"
#endif

#endif