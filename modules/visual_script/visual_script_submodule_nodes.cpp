#include "visual_script_submodule_nodes.h"

void VisualScriptSubmoduleNode::_bind_methods() {}

int VisualScriptSubmoduleNode::get_output_sequence_port_count() const {
    return 1; // don't need much more
}
bool VisualScriptSubmoduleNode::has_input_sequence_port() const {
    return true; // change if required
}
String VisualScriptSubmoduleNode::get_output_sequence_port_text(int p_port) const {
    return ""; // not needed
}
int VisualScriptSubmoduleNode::get_input_value_port_count() const {
    return 0; // change as per submodule
}
int VisualScriptSubmoduleNode::get_output_value_port_count() const {
    return 1; // can't have more than one output value port
}

PropertyInfo VisualScriptSubmoduleNode::get_input_value_port_info(int p_idx) const { return PropertyInfo(); }
PropertyInfo VisualScriptSubmoduleNode::get_output_value_port_info(int p_idx) const { return PropertyInfo(); }

String VisualScriptSubmoduleNode::get_caption() const { return "submod node"; }
String VisualScriptSubmoduleNode::get_text() const { return ""; }

void VisualScriptSubmoduleNode::set_submodule(Ref<VisualScriptSubmodule> p_mod) {
    submodule = p_mod;
}
Ref<VisualScriptSubmodule> VisualScriptSubmoduleNode::get_submodule() const { return submodule; }

class VisualScriptSubmoduleNodeInstance : public VisualScriptNodeInstance {
public:
	VisualScriptSubmoduleNode *node;
	VisualScriptInstance *instance;

	//virtual int get_working_memory_size() const { return 0; }
	//virtual bool is_output_port_unsequenced(int p_idx) const { return false; }
	//virtual bool get_output_port_unsequenced(int p_idx,Variant* r_value,Variant* p_working_mem,String &r_error) const { return true; }

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Callable::CallError &r_error, String &r_error_str) {
        // not sure if I want to add the submodule call here
        // TODO: call submodule execution function
		return 0;
	}
};

	

VisualScriptNodeInstance *VisualScriptSubmoduleNode::instance(VisualScriptInstance *p_instance) {
    VisualScriptSubmoduleNodeInstance *instance = memnew(VisualScriptSubmoduleNodeInstance);
	instance->node = this;
	instance->instance = p_instance;
	return instance;
}


VisualScriptSubmoduleNode::VisualScriptSubmoduleNode() {}
VisualScriptSubmoduleNode::~VisualScriptSubmoduleNode() {}

// VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);


void register_visual_script_submodule_nodes() {
	VisualScriptLanguage::singleton->add_register_func("functions/submodule", create_node_generic<VisualScriptSubmoduleNode>);
}

