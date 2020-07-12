#include "visual_script_submodule_nodes.h"

void VisualScriptSubmoduleNode::_bind_methods() {}

int VisualScriptSubmoduleNode::get_output_sequence_port_count() const {}
bool VisualScriptSubmoduleNode::has_input_sequence_port() const {}
String VisualScriptSubmoduleNode::get_output_sequence_port_text(int p_port) const {}
int VisualScriptSubmoduleNode::get_input_value_port_count() const {}
int VisualScriptSubmoduleNode::get_output_value_port_count() const {}

PropertyInfo VisualScriptSubmoduleNode::get_input_value_port_info(int p_idx) const {}
PropertyInfo VisualScriptSubmoduleNode::get_output_value_port_info(int p_idx) const {}

String VisualScriptSubmoduleNode::get_caption() const {}
String VisualScriptSubmoduleNode::get_text() const {}

void VisualScriptSubmoduleNode::set_submodule(Ref<VisualScriptSubmodule> p_mod) {}
Ref<VisualScriptSubmodule> VisualScriptSubmoduleNode::get_submodule() const {}

VisualScriptSubmoduleNode::VisualScriptSubmoduleNode() {}
VisualScriptSubmoduleNode::~VisualScriptSubmoduleNode() {}

// VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);


void register_visual_script_submodule_nodes() {
	VisualScriptLanguage::singleton->add_register_func("functions/submodule", create_node_generic<VisualScriptSubmoduleNode>);
}

