#include "visual_script_submodule_nodes.h"

void VisualScriptSubmoduleNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_submodule", "submodule"), &VisualScriptSubmoduleNode::set_submodule);
	ClassDB::bind_method(D_METHOD("get_submodule"), &VisualScriptSubmoduleNode::get_submodule);

	ClassDB::bind_method(D_METHOD("get_submodule_name"), &VisualScriptSubmoduleNode::get_submodule_name);
	ClassDB::bind_method(D_METHOD("set_submodule_by_name", "submodule_name"), &VisualScriptSubmoduleNode::set_submodule_by_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "submodule_name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_submodule_by_name", "get_submodule_name");
}

void VisualScriptSubmoduleNode::set_submodule_by_name(const StringName &p_name) {
	Ref<VisualScript> vs = get_container();
	ERR_FAIL_COND(!vs.is_valid());

	submodule = vs->get_submodule(p_name);
	ERR_FAIL_COND(!submodule.is_valid());
}

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

PropertyInfo VisualScriptSubmoduleNode::get_input_value_port_info(int p_idx) const {
	return PropertyInfo();
}
PropertyInfo VisualScriptSubmoduleNode::get_output_value_port_info(int p_idx) const {
	return PropertyInfo();
}

String VisualScriptSubmoduleNode::get_caption() const {
	return "Submodule Node";
}
String VisualScriptSubmoduleNode::get_text() const {
	return "";
}

void VisualScriptSubmoduleNode::set_submodule(const StringName &p_name, Ref<VisualScriptSubmodule> p_mod) {
	submodule_name = p_name;
	submodule = p_mod;
}

Ref<VisualScriptSubmodule> VisualScriptSubmoduleNode::get_submodule() const {
	return submodule;
}

StringName VisualScriptSubmoduleNode::get_submodule_name() const {
	return submodule_name;
}

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

VisualScriptSubmoduleNode::VisualScriptSubmoduleNode() {
	submodule_name = "";
}

VisualScriptSubmoduleNode::~VisualScriptSubmoduleNode() {}

// VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

void register_visual_script_submodule_nodes() {
	VisualScriptLanguage::singleton->add_register_func("functions/submodule_node", create_node_generic<VisualScriptSubmoduleNode>);
	VisualScriptLanguage::singleton->add_register_func("submodule/entry_node", create_node_generic<VisualScriptSubmoduleEntryNode>);
	VisualScriptLanguage::singleton->add_register_func("submodule/exit_node", create_node_generic<VisualScriptSubmoduleExitNode>);
}

int VisualScriptSubmoduleEntryNode::get_output_value_port_count() const {
	return outputports.size();
}

PropertyInfo VisualScriptSubmoduleEntryNode::get_output_value_port_info(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, outputports.size(), PropertyInfo());

	PropertyInfo pi;
	pi.name = outputports[p_idx].name;
	pi.type = outputports[p_idx].type;
	return pi;
}

class VisualScriptSubmoduleEntryNodeInstance : public VisualScriptNodeInstance {
public:
	//VisualScriptSubmoduleNode *node;
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

VisualScriptNodeInstance *VisualScriptSubmoduleEntryNode::instance(VisualScriptInstance *p_instance) {
	VisualScriptSubmoduleEntryNodeInstance *instance = memnew(VisualScriptSubmoduleEntryNodeInstance);
	//instance->node = this;
	instance->instance = p_instance;
	return instance;
}

VisualScriptSubmoduleEntryNode::VisualScriptSubmoduleEntryNode() {}

VisualScriptSubmoduleEntryNode::~VisualScriptSubmoduleEntryNode() {}

int VisualScriptSubmoduleExitNode::get_input_value_port_count() const {
	return inputports.size();
}

PropertyInfo VisualScriptSubmoduleExitNode::get_input_value_port_info(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, inputports.size(), PropertyInfo());

	PropertyInfo pi;
	pi.name = inputports[p_idx].name;
	pi.type = inputports[p_idx].type;
	return pi;
}

class VisualScriptSubmoduleExitNodeInstance : public VisualScriptNodeInstance {
public:
	//VisualScriptSubmoduleNode *node;
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

VisualScriptNodeInstance *VisualScriptSubmoduleExitNode::instance(VisualScriptInstance *p_instance) {
	VisualScriptSubmoduleExitNodeInstance *instance = memnew(VisualScriptSubmoduleExitNodeInstance);
	//instance->node = this;
	instance->instance = p_instance;
	return instance;
}

VisualScriptSubmoduleExitNode::VisualScriptSubmoduleExitNode() {}

VisualScriptSubmoduleExitNode::~VisualScriptSubmoduleExitNode() {}
