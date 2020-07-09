#include "visual_script_submodules.h"

void VisualScriptSubmodule::_set_data(const Dictionary& p_data) {
    // used to load data for the resource
    Array nds = p_data["nodes"];
    for (int i = 0; i < nds.size(); i++) {
        Dictionary nd = nds[i];
        add_node(nd["id"], nd["node"], nd["pos"]);
    }
    Array sequence_connections = p_data["sequence_connections"];
    for (int j = 0; j < sequence_connections.size(); j += 3) {
        sequence_connect(sequence_connections[j + 0], sequence_connections[j + 1], sequence_connections[j + 2]);
    }

    Array data_connections = p_data["data_connections"];
    for (int j = 0; j < data_connections.size(); j += 4) {
        data_connect(data_connections[j + 0], data_connections[j + 1], data_connections[j + 2], data_connections[j + 3]);
    }

	scroll = p_data["scroll"];
}
Dictionary VisualScriptSubmodule::_get_data() const {
    // used to save data
    Dictionary d;

    Array nds;
    List<int> keys;
    nodes.get_key_list(&keys);
    for (const List<int>::Element *E = keys.front(); E; E = E->next()) {
        Dictionary nd;
        nd["id"] = E->get();
        nd["node"] = nodes[E->get()].node;
        nd["pos"] = nodes[E->get()].pos;
        nds.push_back(nd);
    }
    d["nodes"] = nds;


	Array seqconns;
	for (const Set<VisualScript::SequenceConnection>::Element *F = sequence_connections.front(); F; F = F->next()) {
		seqconns.push_back(F->get().from_node);
		seqconns.push_back(F->get().from_output);
		seqconns.push_back(F->get().to_node);
	}
	d["sequence_connections"] = seqconns;

	Array dataconns;
	for (const Set<VisualScript::DataConnection>::Element *F = data_connections.front(); F; F = F->next()) {
		dataconns.push_back(F->get().from_node);
		dataconns.push_back(F->get().from_port);
		dataconns.push_back(F->get().to_node);
		dataconns.push_back(F->get().to_port);
	}
	d["data_connections"] = dataconns;

	d["scroll"] = scroll;
    return d;
}

void VisualScriptSubmodule::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_data", "_get_data");
}