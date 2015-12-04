#include <raytracing/kdtree.h>
#include <scene/geometry/boundingbox.h>

// For comparing using n_th element
template <typename NodeData>
struct CompareData {
    CompareData(int axis) {
        axis = axis;
    }
    int axis;
    bool operator()(const NodeData *a, const NodeData *b) const {
        return a->position[axis] == b->position[axis] ?
                    a < b : a->position[axis] < b->position[axis];
    }
};

template <typename NodeData>
KdTree<NodeData>::KdTree(std::vector<NodeData*>& data)
{
    next_free_index = 1;
    kd_nodes = std::vector<KdNode*>(data.size());
    data_nodes = std::vector<NodeData*>(data.size());

    CreateTreeRecursive(data, 0, 0, data.size());
}

template <typename NodeData>
void KdTree<NodeData>::CreateTreeRecursive(
        std::vector<NodeData*>& build_data,
        unsigned int node_index,
        unsigned int start,
        unsigned int end
        )
{
    if (start + 1 == end) {
        // Get to leaf node, initialize data_nodes
        kd_nodes[node_index].init(0, 0);
        data_nodes[node_index] = build_data[start];
    }

    // Compute bound of points
    BoundingBox bound;
    for (int i = start; i < end; i++)
    {
        bound = BoundingBox::Union(bound, build_data);
    }

    // Partial sort all points less than to the left and greater than to the right
    int split_axis = bound.MaximumExtent();
    int split_pos = (start + end) / 2;
    std::nth_element(build_data[0], build_data[split_pos], build_data[end], CompareData<NodeData>(split_axis));

    // Initialize this node
    kd_nodes[node_index].init(build_data[split_pos]->position[split_axis], split_axis);
    data_nodes[node_index] = build_data[split_pos];

    if (start < split_pos) {
        // Build left branch
        int left_index = next_free_index++;
        kd_nodes[node_index]->left = left_index;
        CreateTreeRecursive(build_data, left_index, start, split_pos);
    }

    if (split_pos + 1 < end) {
        // Build right branch
        int right_index = next_free_index++;
        kd_nodes[node_index]->left = right_index;
        CreateTreeRecursive(build_data, right_index, split_pos + 1, end);
    }
}


