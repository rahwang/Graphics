#pragma once

#include <la.h>
#include <vector>

struct KdNode
{
    void init(float p, int a, int parent)
    {
        split_axis = p;
        split_pos = a;
        left = -1;
        right = -1;
        parent = -1;
    }

    float split_pos;
    int split_axis;
    int left;
    int right;
    int parent;
};

template <typename NodeData>
class KdTree
{
public:
    KdTree(std::vector<NodeData *> &data);

    // Search return index of the leaf node where given point should be inserted.
    int Search(const glm::vec3 &position, int node_idx) const;

    // Search for nearest neighbors
    void LookUp(const glm::vec3& position,
            int& neighbor_num,
            int &start_idx,
            float& max_dist,
            std::vector<NodeData>& out_neighbors) const;

private:
    void CreateTreeRecursive(std::vector<NodeData*> &build_data,
            unsigned int node_index,
            unsigned int start,
            unsigned int end
            );

    // List of kdNodes to keep track of split dimension
    std::vector<KdNode*> kd_nodes;

    // List od dataNodes to keep track of the actual data
    std::vector<NodeData*> data_nodes;

    // Keep track of the next free index for kd_nodes and data_nodes
    unsigned int next_free_index;
};
