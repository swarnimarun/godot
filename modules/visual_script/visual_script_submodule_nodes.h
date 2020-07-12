#ifndef VISUAL_SCRIPT_SUBMODULE_NODES_H
#define VISUAL_SCRIPT_SUBMODULE_NODES_H

#include "visual_script.h"

class VisualScriptSubmoduleNode : public VisualScriptNode {
	GDCLASS(VisualScriptSubmoduleNode, VisualScriptNode);

    Ref<VisualScriptSubmodule> submodule;
protected:
	static void _bind_methods();

public:
	virtual int get_output_sequence_port_count() const;
	virtual bool has_input_sequence_port() const;

	virtual String get_output_sequence_port_text(int p_port) const;

	virtual int get_input_value_port_count() const;
	virtual int get_output_value_port_count() const;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const;

	virtual String get_caption() const;
	virtual String get_text() const;
	virtual String get_category() const { return "functions"; }

	void set_submodule(Ref<VisualScriptSubmodule> p_mod);
	Ref<VisualScriptSubmodule> get_submodule() const;

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	VisualScriptSubmoduleNode();
    ~VisualScriptSubmoduleNode();
};

void register_visual_script_submodule_nodes();

#endif