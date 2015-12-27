#pragma once

#include <la.h>
#include <vector>
#include <scene/geometry/boundingbox.h>

enum {
    _X = 0,
    _Y = 1,
    _Z = 2,
    _NONE = 3
};


struct KdNode
{
    KdNode() :
        split_pos(0.f),
        split_axis(_NONE),
        left(-1),
        right(-1)
    {

    }


    KdNode(float po, int a)
    {
        split_pos = po;
        split_axis = a;
        left = -1;
        right = -1;
    }

    float split_pos;
    int split_axis;
    int left;
    int right;
};

template <typename NodeData>
class KdTree
{
public:
    KdTree(std::vector<NodeData> &data);

    // Search for nearest neighbors
    int LookUp(const glm::vec3& position,
            int& neighbor_num,
            float& max_dist,
            std::vector<NodeData>& out_neighbors) const;

private:
    void CreateTreeRecursive(std::vector<NodeData> &build_data,
            unsigned int node_index,
            unsigned int start,
            unsigned int end);

    void LookUpPrivate(const glm::vec3& position,
            int& neighbor_num,
            int node_idx,
            float& max_dist,
            std::vector<NodeData>& out_neighbors) const;


    // List of kdNodes to keep track of split dimension
    std::vector<KdNode*> kd_nodes;

    // List od dataNodes to keep track of the actual data
    std::vector<NodeData> data_nodes;

    // Keep track of the next free index for kd_nodes and data_nodes
    unsigned int next_free_index;
};

// For comparing using n_th element
template <typename NodeData>
struct CompareData {
    CompareData(int a) {
        axis = a;
    }
    int axis;
    bool operator()(const NodeData a, const NodeData b) const {
        return a.position[axis] < b.position[axis];
    }
};

template <typename NodeData>
KdTree<NodeData>::KdTree(std::vector<NodeData> &data)
{
    if (data.size() == 0) {
        next_free_index = 1;
    } else {
        next_free_index = 1;
        kd_nodes = std::vector<KdNode*>(data.size());
        data_nodes = std::vector<NodeData>(data.size());

        CreateTreeRecursive(data, 0, 0, data.size());
    }
}

template <typename NodeData>
void KdTree<NodeData>::CreateTreeRecursive(
        std::vector<NodeData>& build_data,
        unsigned int node_index,
        unsigned int start,
        unsigned int end
        )
{
    if (start + 1 == end) {
        // Get to leaf node, initialize data_nodes
        kd_nodes[node_index] = new KdNode(0.f, _NONE);
        data_nodes[node_index] = build_data[start];
    }

    // Compute bound of points
    BoundingBox bound;
    for (unsigned int i = start; i < end; i++)
    {
        bound = BoundingBox::Union(bound, build_data[i].position);
    }

    // Partial sort all points less than to the left and greater than to the right
    unsigned int split_axis = bound.MaximumExtent();
    unsigned int split_pos = (start + end) / 2;
    std::nth_element(&build_data[start], &build_data[split_pos], &build_data[end], CompareData<NodeData>(split_axis));

    // Initialize this node
    kd_nodes[node_index] = new KdNode(build_data[split_pos].position[split_axis], split_axis);
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

template <typename NodeData>
int KdTree<NodeData>::LookUp(
        const glm::vec3& position,
        int& neighbor_num,
        float& max_dist,
        std::vector<NodeData>& out_neighbors
        ) const
{
    if (data_nodes.size() == 0)
    {
        return 0;
    }

    LookUpPrivate(position, neighbor_num, 0, max_dist, out_neighbors);

    return out_neighbors.size();
}

template <typename NodeData>
void KdTree<NodeData>::LookUpPrivate(
        const glm::vec3& position,
        int& neighbor_num,
        int node_idx,
        float& max_dist,
        std::vector<NodeData>& out_neighbors
        ) const
{
    KdNode* node = kd_nodes[node_idx];
    int axis = node->split_axis;

    // axis = _NONE when at leaf node
    if (axis != _NONE)
    {
        float dist2 = pow((position[axis] - node->split_pos),2.f);

        // Position is less than split plane, go left
        if (position[axis] < node->split_pos)
        {
            if (node->left != -1)
            {
                LookUpPrivate(position, neighbor_num, node->left, max_dist, out_neighbors);
            }

            if (dist2 < max_dist && node->right != -1)
            {
                LookUpPrivate(position, neighbor_num, node->right, max_dist, out_neighbors);
            }
        }
        else
        {
            if (node->right != -1)
            {
                LookUpPrivate(position, neighbor_num, node->right, max_dist, out_neighbors);
            }

            if (dist2 < max_dist && node->left != -1)
            {
                LookUpPrivate(position, neighbor_num, node->left, max_dist, out_neighbors);
            }
        }
        dist2 = glm::distance2(data_nodes[node_idx].position, position);
        if (dist2 < max_dist && out_neighbors.size() < neighbor_num)
        {
            out_neighbors.push_back(data_nodes[node_idx]);
        }
    }

//    // Vector containing k best distance.
//    std::vector<float> distances;
//    // Vector containing k best nodes (index parallel to "distances").
//    std::vector<int> node_indicies;

//    // Begin by searching for appropriate place in the tree.
//    int curr_idx = Search(position, start_idx);

//    // Get distance from position.
//    float distance = glm::distance2(data_nodes[curr_idx].position, position);
//    if (distance < max_dist) {
//        distances.push_back(distance);
//        node_indicies.push_back(curr_idx);
//    }

//    while (curr_idx > 0) {
//        // Get parent.
//        KdNode *parent = kd_nodes[kd_nodes[curr_idx]->parent];

//        // Check if parent should be inserted into candidate list.
//        float tmp_distance = glm::distance2(data_nodes[kd_nodes[curr_idx]->parent].position, position);

//        int insert_idx = 0;
//        while (distances.size() && tmp_distance > distances[insert_idx]) {
//            ++insert_idx;
//        }

//        if (insert_idx < neighbor_num &&
//                insert_idx < node_indicies.size() &&
//                tmp_distance < max_dist
//                )
//        {
//            // If the distance should be inserted before index k, it belongs in the list.
//            distances.insert(distances.begin() + insert_idx, 1, tmp_distance);
//            node_indicies.insert(node_indicies.begin() + insert_idx, 1, kd_nodes[curr_idx]->parent);
//        }

//        while (node_indicies.size() >= neighbor_num) {
//            distances.pop_back();
//            node_indicies.pop_back();
//        }

//        // Check if the sphere of radius position to kth candidate intersects the split plane.
//        // If the plane intersects or we have less than k candidates, traverse other subtree.
//        if (distances.size() && distances.back() > (parent->split_pos - position[parent->split_axis])
//                || distances.size() < neighbor_num) {
//            // Check whether should go down left or right subtree
//            if (curr_idx == parent->left && parent->right != -1) {
//                // Go right.
//                LookUpPrivate(position, neighbor_num, parent->right, max_dist, out_neighbors);
//            } else if (curr_idx == parent->right && parent->left != -1) {
//                // Go left.
//                LookUpPrivate(position, neighbor_num, parent->left, max_dist, out_neighbors);
//            }
//        }
//        curr_idx = kd_nodes[curr_idx]->parent;
//    }

//    // Fill out vector with actual data nodes based on indicies in "nodes" vector.
//    for (int idx : node_indicies) {
//        out_neighbors.push_back(data_nodes[idx]);
//    }
//    return out_neighbors.size();
}

