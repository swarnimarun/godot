#include "visual_script.h"

class VisualScriptSubmodule : public Resource {
	GDCLASS(VisualScriptSubmodule, Resource);
	RES_BASE_EXTENSION("vsmodule"); // TODO: To be discussed

	friend class VisualScriptInstance;

private:
    // should it have variables??
    struct NodeData {
        Point2 pos;
        Ref<VisualScriptNode> node;
    };
    HashMap<int, NodeData> nodes;

	Set<VisualScript::DataConnection> data_connections;
	Set<VisualScript::SequenceConnection> sequence_connections;

    Vector2 scroll;
protected:
	static void _bind_methods();

private:
	void _set_data(const Dictionary &p_data);
	Dictionary _get_data() const;

public:
    void add_node(int p_id, const Ref<VisualScriptNode> &p_node, const Point2 &p_pos = Point2());
    void remove_node(int p_id);
    bool has_node(int p_id) const;
    Ref<VisualScriptNode> get_node(int p_id) const;
    Vector2 get_node_position(int p_id) const;
    void set_node_position(Vector2 p_pos);
    void get_node_list(List<int> *r_nodes) const;

    void set_scroll(Vector2 p_scroll);
    Vector2 get_scroll() const;

    void sequence_connect(int p_from_node, int p_from_output, int p_to_node);
    void sequence_disconnect(int p_from_node, int p_from_output, int p_to_node);
    bool has_sequence_connection(int p_from_node, int p_from_output, int p_to_node) const;
    void get_sequence_connection_list(List<VisualScript::SequenceConnection> *r_connection) const;

    void data_connect(int p_from_node, int p_from_port, int p_to_node, int p_to_port);
    void data_disconnect(int p_from_node, int p_from_port, int p_to_node, int p_to_port);
    bool has_data_connection(int p_from_node, int p_from_port, int p_to_node, int p_to_port) const;
    void get_data_connection_list(List<VisualScript::DataConnection> *r_connection) const;

    VisualScriptSubmodule();
    ~VisualScriptSubmodule();
};