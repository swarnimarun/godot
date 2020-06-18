#include "visual_script_submodules.h"

void VisualScriptSubmodule::_set_data(const Dictionary& p_data) {
    // used to load data for the resource
}
Dictionary VisualScriptSubmodule::_get_data() const {
    // used to save data
    Dictionary d;
    return d;
}

void VisualScriptSubmodule::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_data", "_get_data");
}