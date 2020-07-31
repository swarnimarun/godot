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
	virtual int get_output_sequence_port_count() const override;
	virtual bool has_input_sequence_port() const override;

	virtual String get_output_sequence_port_text(int p_port) const override;

	virtual int get_input_value_port_count() const override;
	virtual int get_output_value_port_count() const override;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const override;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const override;

	virtual String get_caption() const override;
	virtual String get_text() const override;
	virtual String get_category() const override { return "functions"; }

	void set_submodule(const StringName &p_name, Ref<VisualScriptSubmodule> p_mod);
	Ref<VisualScriptSubmodule> get_submodule() const;
	StringName get_submodule_name() const;
	void set_submodule_by_name(const StringName &p_name);

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance) override;

	VisualScriptSubmoduleNode();
	~VisualScriptSubmoduleNode();
};

class VisualScriptSubmoduleEntryNode : public VisualScriptLists {
	GDCLASS(VisualScriptSubmoduleEntryNode, VisualScriptLists);

public:
	virtual bool is_output_port_editable() const override { return true; }
	virtual bool is_output_port_name_editable() const override { return true; }
	virtual bool is_output_port_type_editable() const override { return true; }

	virtual bool is_input_port_editable() const override { return false; }
	virtual bool is_input_port_name_editable() const override { return false; }
	virtual bool is_input_port_type_editable() const override { return false; }

	virtual int get_output_sequence_port_count() const override { return 1; }
	virtual bool has_input_sequence_port() const override { return false; }

	virtual String get_output_sequence_port_text(int p_port) const override { return ""; }

	virtual int get_input_value_port_count() const override { return 0; }
	virtual int get_output_value_port_count() const override;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const override { return PropertyInfo(); }
	virtual PropertyInfo get_output_value_port_info(int p_idx) const override;

	virtual String get_caption() const override { return "Entry"; }
	virtual String get_text() const override { return ""; }
	virtual String get_category() const override { return "submodule"; }

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance) override;

	VisualScriptSubmoduleEntryNode();
	~VisualScriptSubmoduleEntryNode();
};

class VisualScriptSubmoduleExitNode : public VisualScriptLists {
	GDCLASS(VisualScriptSubmoduleExitNode, VisualScriptLists);

public:
	virtual bool is_output_port_editable() const override { return false; }
	virtual bool is_output_port_name_editable() const override { return false; }
	virtual bool is_output_port_type_editable() const override { return false; }

	virtual bool is_input_port_editable() const override { return true; }
	virtual bool is_input_port_name_editable() const override { return true; }
	virtual bool is_input_port_type_editable() const override { return true; }

	virtual int get_output_sequence_port_count() const override { return 0; }
	virtual bool has_input_sequence_port() const override { return true; }

	virtual String get_output_sequence_port_text(int p_port) const override { return ""; }

	virtual int get_input_value_port_count() const override;
	virtual int get_output_value_port_count() const override { return 0; }

	virtual PropertyInfo get_input_value_port_info(int p_idx) const override;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const override { return PropertyInfo(); }

	virtual String get_caption() const override { return "Exit"; }
	virtual String get_text() const override { return ""; }
	virtual String get_category() const override { return "submodule"; }

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance) override;

	VisualScriptSubmoduleExitNode();
	~VisualScriptSubmoduleExitNode();
};

void register_visual_script_submodule_nodes();

#endif