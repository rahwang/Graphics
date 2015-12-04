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
    kd_nodes = std::vector<KdNode*>();
    kd_nodes.resize(data.size());
    data_nodes = std::vector<NodeData*>();
    data_nodes.resize(data.size());

    CreateTreeRecursive(data, 0, 0, data.size());
}

template <typename NodeData>
void KdTree<NodeData>::CreateTreeRecursive(

        std::vector<NodeData*>& build_data,
        unsigned int node_index,
        unsigned int start,
        unsigned int end,
        unsigned int parent
        )
{
    if (start + 1 == end) {
        // Get to leaf node, initialize data_nodes
        kd_nodes[node_index].init(0, 0, parent);
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
    kd_nodes[node_index].init(build_data[split_pos]->position[split_axis], split_axis, parent);
    data_nodes[node_index] = build_data[split_pos];

    if (start < split_pos) {
        // Build left branch
        int left_index = next_free_index++;
        kd_nodes[node_index]->left = left_index;
        CreateTreeRecursive(build_data, left_index, start, split_pos, node_index);
    }

    if (split_pos + 1 < end) {
        // Build right branch
        int right_index = next_free_index++;
        kd_nodes[node_index]->left = right_index;
        CreateTreeRecursive(build_data, right_index, split_pos + 1, end);
    }
}


int KdTree::Search(const glm::vec3 &position, int node_idx) const {
    // Shouldn't happen.
    if (node_idx == -1) {
        return -1;
    }

    // Get the node at the current index.
    KdNode *curr = kd_nodes[node_idx];

    // Check if position is to the left of the split plane.
    if (position[curr->split_axis] < curr->split_pos) {
        // If there is no left child, return current index.
        if (curr->left == -1) {
            return node_idx;
        }
        // Recursively search left sub-tree.
        return Search(position, curr->left);
    } else if (position[curr->split_axis] > curr->split_pos) {
        if (curr->right == -1) {
            return node_idx;
        }
        return Search(position, curr->right);
    }
    return node_idx;
}

// Get the idx to insert item to maintain sorted status of node vector.
int get_insert_idx(float curr_distance, const std::vector<float> &distances) {
    int idx = 0;
    while (curr_distance > distances[idx]) {
        ++idx;
    }
    return idx;
}

void KdTree::LookUp(
        const glm::vec3& position,
        int& neighbor_num,
        float& max_dist_squared,
        std::vector<NodeData>& out_neighbors) const {

    // Vector containing k best distance.
    std::vector<float> distances;
    // Vector containing k best nodes (index parallel to "distances").
    std::vector<int> nodes;

    // Begin by searching for appropriate place in the tree.
    int start_idx = Search(position, 0);

    // Get distance from position.
    distances.push_back((data_nodes(start_idx).position - position).length());
    nodes.push_back(start_idx);


}

