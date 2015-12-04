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
        int& start_idx,
        float& max_dist,
        std::vector<NodeData>& out_neighbors) const {

    // Vector containing k best distance.
    std::vector<float> distances;
    // Vector containing k best nodes (index parallel to "distances").
    std::vector<int> node_indicies;

    // Begin by searching for appropriate place in the tree.
    int curr_idx = Search(position, start_idx);

    // Get distance from position.
    distances.push_back((data_nodes(curr_idx).position - position).length());
    node_indicies.push_back(curr_idx);

    while (curr_index != 0) {
        // Get parent.
        KdNode *parent = kd_nodes[kd_nodes[curr_idx]->parent];

        // Check if parent should be inserted into candidate list.
        float tmp_distance = (data_nodes[kd_nodes[curr_idx]->parent].position - position).length();
        float insert_idx = get_insert_idx(distance, distances);
        if (insert_idx < neighbor_num) {
            // If the distance should be inserted before index k, it belongs in the list.
            distances.insert(insert_idx, 1, tmp_distance);
            node_indicies.insert(insert_idx, 1, kd_nodes[curr_idx]->parent);
            distances.pop_back();
            node_indicies.pop_back();
        }

        // Check if the sphere of radius position to kth candidate intersects the split plane.
        // If the plane intersects or we have less than k candidates, traverse other subtree.
        if (distances.back() > (parent->split_pos - position[parent->split_axis])
                || distances.size() < neighbor_num) {
            // Check whether should go down left or right subtree
            if (curr_idx == parent->left) {
                // Go right.
                LookUp(position, neighbor_num, parent->right, max_dist, out_neighbors);
            } else {
                // Go left.
                LookUp(position, neighbor_num, parent->left, max_dist, out_neighbors);
            }
        }
        curr_idx = kd_nodes[curr_idx]->parent;
    }

    // Fill out vector with actual data nodes based on indicies in "nodes" vector.
    for (int idx : node_indicies) {
        out_neighbors.push_back(data_nodes[idx]);
    }
    return;
}

