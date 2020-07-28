#ifndef VISUAL_SCRIPT_SUBMODULE_NODES_H
#define VISUAL_SCRIPT_SUBMODULE_NODES_H

#include "visual_script.h"
#include "visual_script_nodes.h"

class VisualScriptSubmoduleNode : public VisualScriptNode {
	GDCLASS(VisualScriptSubmoduleNode, VisualScriptNode);

	Ref<VisualScriptSubmodule> submodule;
	StringName submodule_name;
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

	void set_submodule(const StringName &p_name, Ref<VisualScriptSubmodule> p_mod);
	Ref<VisualScriptSubmodule> get_submodule() const;
	StringName get_submodule_name() const;
	void set_submodule_by_name(const StringName &p_name);

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	VisualScriptSubmoduleNode();
	~VisualScriptSubmoduleNode();
};

class VisualScriptSubmoduleEntryNode : public VisualScriptLists {
	GDCLASS(VisualScriptSubmoduleEntryNode, VisualScriptLists);

public:
	virtual bool is_output_port_editable() const { return true; }
	virtual bool is_output_port_name_editable() const { return true; }
	virtual bool is_output_port_type_editable() const { return true; }

	virtual bool is_input_port_editable() const { return false; }
	virtual bool is_input_port_name_editable() const { return false; }
	virtual bool is_input_port_type_editable() const { return false; }

	virtual int get_output_sequence_port_count() const { return 1; }
	virtual bool has_input_sequence_port() const { return false; }

	virtual String get_output_sequence_port_text(int p_port) const { return ""; }

	virtual int get_input_value_port_count() const { return 0; }
	virtual int get_output_value_port_count() const;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const { return PropertyInfo(); }
	virtual PropertyInfo get_output_value_port_info(int p_idx) const;

	virtual String get_caption() const { return "Entry"; }
	virtual String get_text() const { return ""; }
	virtual String get_category() const { return "submodule"; }

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	VisualScriptSubmoduleEntryNode();
	~VisualScriptSubmoduleEntryNode();
};

class VisualScriptSubmoduleExitNode : public VisualScriptLists {
	GDCLASS(VisualScriptSubmoduleExitNode, VisualScriptLists);

public:
	virtual bool is_output_port_editable() const { return false; }
	virtual bool is_output_port_name_editable() const { return false; }
	virtual bool is_output_port_type_editable() const { return false; }

	virtual bool is_input_port_editable() const { return true; }
	virtual bool is_input_port_name_editable() const { return true; }
	virtual bool is_input_port_type_editable() const { return true; }

	virtual int get_output_sequence_port_count() const { return 0; }
	virtual bool has_input_sequence_port() const { return true; }

	virtual String get_output_sequence_port_text(int p_port) const { return ""; }

	virtual int get_input_value_port_count() const;
	virtual int get_output_value_port_count() const { return 0; }

	virtual PropertyInfo get_input_value_port_info(int p_idx) const;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const { return PropertyInfo(); }

	virtual String get_caption() const { return "Exit"; }
	virtual String get_text() const { return ""; }
	virtual String get_category() const { return "submodule"; }

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	VisualScriptSubmoduleExitNode();
	~VisualScriptSubmoduleExitNode();
};

void register_visual_script_submodule_nodes();

#endif