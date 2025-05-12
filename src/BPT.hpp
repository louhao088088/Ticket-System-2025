#include "vector.hpp"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

const int BLOCK_SIZE = 512;
const int KEY_SIZE = 12;

const int MAX_LEAF_KEYS = (BLOCK_SIZE - 12) / KEY_SIZE - 1;
const int MAX_INTERNAL_KEYS = (BLOCK_SIZE - 12) / (KEY_SIZE + 4) - 1;
const int MIN_LEAF_KEYS = std::max(1, (MAX_LEAF_KEYS + 1) / 2 - 1);
const int MIN_INTERNAL_KEYS = std::max(1, MAX_INTERNAL_KEYS / 2 - 1);

const int mod1 = 998244353, mod2 = 1019260817, base1 = 233, base2 = 279;

template <typename T> using vector = sjtu::vector<T>;

struct FileHeader {
    int root_block;
    int first_leaf_block;
    int block_count;
};

struct InternalNode {
    int type;
    int num_keys;
    int children[MAX_INTERNAL_KEYS + 2];
    long long keys[MAX_INTERNAL_KEYS + 1];
    int values[MAX_INTERNAL_KEYS + 1];
};

struct LeafNode {
    int type;
    int num_keys;
    int next_leaf;
    long long keys[MAX_LEAF_KEYS + 1];
    int values[MAX_LEAF_KEYS + 1];
};
long long Hash(const char *data) {
    long long res1 = 1, res2 = 1;
    for (int i = 0; i < 64; i++) {
        res1 = res1 * base1 + data[i], res1 %= mod1;
        res2 = res2 * base2 + data[i], res2 %= mod2;
    }
    return res1 * mod2 + res2;
}

class BPlusTree {
  private:
    std::fstream &file;
    FileHeader header;

    void write_header() {
        file.seekp(0);
        file.write(reinterpret_cast<char *>(&header), sizeof(FileHeader));
    }

    void read_header() {
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(FileHeader));
    }

    void read_block(int block_num, char *data) {
        memset(data, 0, BLOCK_SIZE);
        file.seekg(block_num * BLOCK_SIZE);
        file.read(data, BLOCK_SIZE);
    }

    void write_block(int block_num, const char *data) {
        file.seekp(block_num * BLOCK_SIZE);
        file.write(data, BLOCK_SIZE);
    }

    int allocate_block() {
        int new_block = header.block_count++;
        char empty_block[BLOCK_SIZE] = {0};
        write_block(new_block, empty_block);
        write_header();
        return new_block;
    }

    void parse_leaf(const char *data, LeafNode &leaf) {
        memcpy(&leaf.type, data, sizeof(int));
        memcpy(&leaf.num_keys, data + sizeof(int), sizeof(int));
        memcpy(&leaf.next_leaf, data + 2 * sizeof(int), sizeof(int));
        memcpy(leaf.keys, data + 3 * sizeof(int), (MAX_LEAF_KEYS + 1) * sizeof(long long));
        memcpy(leaf.values, data + 3 * sizeof(int) + (MAX_LEAF_KEYS + 1) * sizeof(long long),
               (MAX_LEAF_KEYS + 1) * sizeof(int));
    }

    void serialize_leaf(const LeafNode &leaf, char *data) {
        memset(data, 0, BLOCK_SIZE);
        memcpy(data, &leaf.type, sizeof(int));
        memcpy(data + sizeof(int), &leaf.num_keys, sizeof(int));
        memcpy(data + 2 * sizeof(int), &leaf.next_leaf, sizeof(int));
        memcpy(data + 3 * sizeof(int), leaf.keys, leaf.num_keys * sizeof(long long));
        memcpy(data + 3 * sizeof(int) + (MAX_LEAF_KEYS + 1) * sizeof(long long), leaf.values,
               leaf.num_keys * sizeof(int));
    }

    void parse_internal(const char *data, InternalNode &node) {
        memcpy(&node.type, data, sizeof(int));
        memcpy(&node.num_keys, data + sizeof(int), sizeof(int));
        memcpy(node.children, data + 2 * sizeof(int), (MAX_INTERNAL_KEYS + 2) * sizeof(int));
        memcpy(node.keys, data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int),
               (MAX_INTERNAL_KEYS + 1) * sizeof(long long));
        memcpy(node.values,
               data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int) +
                   (MAX_INTERNAL_KEYS + 1) * sizeof(long long),
               (MAX_INTERNAL_KEYS + 1) * sizeof(int));
    }

    void serialize_internal(const InternalNode &node, char *data) {
        memset(data, 0, BLOCK_SIZE);
        memcpy(data, &node.type, sizeof(int));
        memcpy(data + sizeof(int), &node.num_keys, sizeof(int));
        memcpy(data + 2 * sizeof(int), node.children, (node.num_keys + 1) * sizeof(int));
        memcpy(data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int), node.keys,
               node.num_keys * sizeof(long long));
        memcpy(data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int) +
                   (MAX_INTERNAL_KEYS + 1) * sizeof(long long),
               node.values, node.num_keys * sizeof(int));
    }

    void insert_into_parent(vector<int> &path, int child_block, long long key, int value) {
        if (path.empty()) {
            InternalNode new_root;
            new_root.type = 0;
            new_root.num_keys = 1;
            new_root.children[0] = header.root_block;

            new_root.keys[0] = key;
            new_root.values[0] = value;

            new_root.children[1] = child_block;
            int new_root_block = allocate_block();
            char data[BLOCK_SIZE];
            serialize_internal(new_root, data);
            write_block(new_root_block, data);
            header.root_block = new_root_block;
            write_header();
            return;
        }

        int parent_block = path.back();
        InternalNode parent;
        char parent_data[BLOCK_SIZE];
        read_block(parent_block, parent_data);
        parse_internal(parent_data, parent);

        // assert(parent.type == 0);

        int pos = 0;
        while (pos < parent.num_keys &&
               (parent.keys[pos] < key || (parent.keys[pos] == key && parent.values[pos] <= value)))
            pos++;

        for (int i = parent.num_keys; i > pos; i--) {
            parent.keys[i] = parent.keys[i - 1], parent.values[i] = parent.values[i - 1];
            parent.children[i + 1] = parent.children[i];
        }
        parent.keys[pos] = key, parent.values[pos] = value;
        parent.children[pos + 1] = child_block;
        parent.num_keys++;

        if (parent.num_keys > MAX_INTERNAL_KEYS) {

            InternalNode new_node;
            new_node.type = 0;
            int split = (parent.num_keys) / 2;
            new_node.num_keys = parent.num_keys - split - 1;
            parent.num_keys = split;

            memcpy(new_node.children, parent.children + split + 1,
                   (new_node.num_keys + 1) * sizeof(int));
            memcpy(new_node.keys, parent.keys + split + 1, new_node.num_keys * sizeof(long long));
            memcpy(new_node.values, parent.values + split + 1, new_node.num_keys * sizeof(int));

            int new_block = allocate_block();
            char new_data[BLOCK_SIZE];

            serialize_internal(new_node, new_data);
            write_block(new_block, new_data);

            char parent_data[BLOCK_SIZE];
            serialize_internal(parent, parent_data);
            write_block(parent_block, parent_data);
            path.pop_back();
            insert_into_parent(path, new_block, parent.keys[split], parent.values[split]);

        } else {
            char data[BLOCK_SIZE];
            serialize_internal(parent, data);
            write_block(parent_block, data);
        }
    }

    void remove_from_leaf(int leaf_block, int pos, vector<int> &path) {
        LeafNode leaf;
        char leaf_data[BLOCK_SIZE];
        read_block(leaf_block, leaf_data);
        parse_leaf(leaf_data, leaf);

        for (int i = pos; i < leaf.num_keys - 1; i++) {
            leaf.keys[i] = leaf.keys[i + 1];
            leaf.values[i] = leaf.values[i + 1];
        }
        leaf.num_keys--;

        if (leaf.num_keys >= MIN_LEAF_KEYS) {
            serialize_leaf(leaf, leaf_data);
            write_block(leaf_block, leaf_data);
            return;
        }

        if (path.empty()) {

            if (leaf.num_keys == 0) {
                header.root_block = 0;
                header.first_leaf_block = 0;
                write_header();
            } else {
                serialize_leaf(leaf, leaf_data);
                write_block(leaf_block, leaf_data);
            }
            return;
        }

        int parent_block = path.back();
        InternalNode parent;
        char parent_data[BLOCK_SIZE];
        read_block(parent_block, parent_data);
        parse_internal(parent_data, parent);

        if (parent.num_keys == 0) {

            serialize_leaf(leaf, leaf_data);
            write_block(leaf_block, leaf_data);
            if (path.size() == 1) {
                header.root_block = leaf_block;
                write_header();
            }
            return;
        }
        int parent_pos = -1;
        for (int i = 0; i <= parent.num_keys; i++) {
            if (parent.children[i] == leaf_block) {
                parent_pos = i - 1;
                break;
            }
            assert(i != parent.num_keys);
        }

        int left_sibling = -1, right_sibling = -1;
        if (parent_pos >= 0)
            left_sibling = parent.children[parent_pos];
        if (parent_pos < parent.num_keys - 1)
            right_sibling = parent.children[parent_pos + 2];

        assert(left_sibling != -1 || right_sibling != -1);

        if (left_sibling != -1) {
            LeafNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_leaf(left_data, left);

            if (left.num_keys > MIN_LEAF_KEYS) {
                for (int i = leaf.num_keys; i > 0; i--) {
                    leaf.keys[i] = leaf.keys[i - 1];
                    leaf.values[i] = leaf.values[i - 1];
                }
                leaf.keys[0] = left.keys[left.num_keys - 1];
                leaf.values[0] = left.values[left.num_keys - 1];

                leaf.num_keys++;
                left.num_keys--;

                parent.keys[parent_pos] = leaf.keys[0];
                parent.values[parent_pos] = leaf.values[0];

                serialize_leaf(left, left_data);
                write_block(left_sibling, left_data);
                serialize_leaf(leaf, leaf_data);
                write_block(leaf_block, leaf_data);
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                return;
            }
        }

        if (right_sibling != -1) {
            LeafNode right;
            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_leaf(right_data, right);

            if (right.num_keys > MIN_LEAF_KEYS) {
                leaf.keys[leaf.num_keys] = right.keys[0];
                leaf.values[leaf.num_keys] = right.values[0];

                leaf.num_keys++;
                for (int i = 0; i < right.num_keys - 1; i++) {
                    right.keys[i] = right.keys[i + 1];
                    right.values[i] = right.values[i + 1];
                }
                right.num_keys--;

                parent.keys[parent_pos + 1] = right.keys[0];
                parent.values[parent_pos + 1] = right.values[0];

                serialize_leaf(right, right_data);
                write_block(right_sibling, right_data);
                serialize_leaf(leaf, leaf_data);
                write_block(leaf_block, leaf_data);
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                return;
            }
        }

        if (left_sibling != -1) {
            LeafNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_leaf(left_data, left);

            for (int i = 0; i < leaf.num_keys; i++) {
                left.keys[left.num_keys + i] = leaf.keys[i];
                left.values[left.num_keys + i] = leaf.values[i];
            }
            left.num_keys += leaf.num_keys;
            left.next_leaf = leaf.next_leaf;

            assert(left.num_keys <= MAX_LEAF_KEYS);

            serialize_leaf(left, left_data);
            write_block(left_sibling, left_data);

            for (int i = parent_pos; i < parent.num_keys - 1; i++) {
                parent.keys[i] = parent.keys[i + 1];
                parent.values[i] = parent.values[i + 1];
                parent.children[i + 1] = parent.children[i + 2];
            }
            parent.num_keys--;
            if (parent.num_keys >= MIN_INTERNAL_KEYS || path.size() == 1) {
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
            } else {
                path.pop_back();
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                coalesce_internal_nodes(path, parent_block);
            }
        } else if (right_sibling != -1) {

            LeafNode right;
            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_leaf(right_data, right);

            for (int i = 0; i < right.num_keys; i++) {
                leaf.keys[leaf.num_keys + i] = right.keys[i];
                leaf.values[leaf.num_keys + i] = right.values[i];
            }
            leaf.num_keys += right.num_keys;
            leaf.next_leaf = right.next_leaf;

            assert(leaf.num_keys <= MAX_LEAF_KEYS);

            serialize_leaf(leaf, leaf_data);
            write_block(leaf_block, leaf_data);

            for (int i = parent_pos + 1; i < parent.num_keys - 1; i++) {
                parent.keys[i] = parent.keys[i + 1];
                parent.values[i] = parent.values[i + 1];

                parent.children[i + 1] = parent.children[i + 2];
            }
            parent.num_keys--;

            if (parent.num_keys >= MIN_INTERNAL_KEYS || path.size() == 1) {
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
            } else {
                path.pop_back();
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                coalesce_internal_nodes(path, parent_block);
            }
        }
    }

    void coalesce_internal_nodes(vector<int> &path, int node_block) {
        InternalNode node;
        char node_data[BLOCK_SIZE];
        read_block(node_block, node_data);
        parse_internal(node_data, node);

        int parent_block = path.back();
        InternalNode parent;
        char parent_data[BLOCK_SIZE];
        read_block(parent_block, parent_data);
        parse_internal(parent_data, parent);
        if (path.size() == 1 && parent.num_keys == 0) {
            header.root_block = node_block;
            write_header();
            return;
        }
        int parent_pos = -1;
        for (int i = 0; i <= parent.num_keys; i++) {
            if (parent.children[i] == node_block) {
                parent_pos = i - 1;
                break;
            }
            assert(i != parent.num_keys);
        }
        int left_sibling = -1, right_sibling = -1;

        if (parent_pos >= 0) {
            left_sibling = parent.children[parent_pos];
        } else if (parent_pos < parent.num_keys - 1) {
            right_sibling = parent.children[parent_pos + 2];
        }

        assert(left_sibling != -1 || right_sibling != -1);

        if (left_sibling != -1) {
            InternalNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_internal(left_data, left);

            if (left.num_keys > MIN_INTERNAL_KEYS) {

                node.children[node.num_keys + 1] = node.children[node.num_keys];
                for (int i = node.num_keys; i > 0; i--) {
                    node.keys[i] = node.keys[i - 1];
                    node.values[i] = node.values[i - 1];
                    node.children[i] = node.children[i - 1];
                }
                node.keys[0] = parent.keys[parent_pos];
                node.values[0] = parent.values[parent_pos];

                parent.keys[parent_pos] = left.keys[left.num_keys - 1];
                parent.values[parent_pos] = left.values[left.num_keys - 1];

                node.children[0] = left.children[left.num_keys];

                left.num_keys--;
                node.num_keys++;

                serialize_internal(left, left_data);
                write_block(left_sibling, left_data);
                serialize_internal(node, node_data);
                write_block(node_block, node_data);
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                return;
            }
        }

        if (right_sibling != -1) {
            InternalNode right;
            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_internal(right_data, right);

            if (right.num_keys > MIN_INTERNAL_KEYS) {
                node.keys[node.num_keys] = parent.keys[parent_pos + 1];
                node.values[node.num_keys] = parent.values[parent_pos + 1];

                parent.keys[parent_pos + 1] = right.keys[0];
                parent.values[parent_pos + 1] = right.values[0];

                node.children[node.num_keys + 1] = right.children[0];
                for (int i = 0; i < right.num_keys - 1; i++) {
                    right.keys[i] = right.keys[i + 1];
                    right.values[i] = right.values[i + 1];

                    right.children[i] = right.children[i + 1];
                }
                right.children[right.num_keys - 1] = right.children[right.num_keys];

                right.num_keys--;
                node.num_keys++;

                serialize_internal(right, right_data);
                write_block(right_sibling, right_data);
                serialize_internal(node, node_data);
                write_block(node_block, node_data);
                serialize_internal(parent, parent_data);
                write_block(parent_block, parent_data);
                return;
            }
        }

        if (left_sibling != -1) {

            InternalNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_internal(left_data, left);

            memcpy(node.keys + left.num_keys + 1, node.keys, node.num_keys * sizeof(long long));
            memcpy(node.values + left.num_keys + 1, node.values, node.num_keys * sizeof(int));

            memcpy(node.children + left.num_keys + 1, node.children,
                   (node.num_keys + 1) * sizeof(int));

            memcpy(node.keys, left.keys, left.num_keys * sizeof(long long));
            memcpy(node.values, left.values, left.num_keys * sizeof(int));

            memcpy(node.children, left.children, (left.num_keys + 1) * sizeof(int));

            node.keys[left.num_keys] = parent.keys[parent_pos];
            node.values[left.num_keys] = parent.values[parent_pos];

            node.num_keys += left.num_keys + 1;

            assert(node.num_keys <= MAX_INTERNAL_KEYS);

            for (int i = parent_pos - 1; i < parent.num_keys - 1; i++) {
                if (i != parent_pos - 1) {
                    parent.keys[i] = parent.keys[i + 1];
                    parent.values[i] = parent.values[i + 1];
                }

                parent.children[i + 1] = parent.children[i + 2];
            }

            parent.num_keys--;

            serialize_internal(node, node_data);
            write_block(node_block, node_data);
            serialize_internal(parent, parent_data);
            write_block(parent_block, parent_data);

            if (parent.num_keys >= MIN_INTERNAL_KEYS || path.size() == 1) {

                return;
            } else {
                path.pop_back();
                coalesce_internal_nodes(path, parent_block);
            }
            return;
        }
        if (right_sibling != -1) {
            InternalNode right;

            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_internal(right_data, right);

            node.keys[node.num_keys] = parent.keys[parent_pos + 1];
            node.values[node.num_keys] = parent.values[parent_pos + 1];

            memcpy(node.keys + node.num_keys + 1, right.keys, right.num_keys * sizeof(long long));
            memcpy(node.values + node.num_keys + 1, right.values, right.num_keys * sizeof(int));

            memcpy(node.children + node.num_keys + 1, right.children,
                   (right.num_keys + 1) * sizeof(int));
            node.num_keys += right.num_keys + 1;

            assert(node.num_keys <= MAX_INTERNAL_KEYS);

            for (int i = parent_pos + 1; i < parent.num_keys - 1; i++) {
                parent.keys[i] = parent.keys[i + 1];
                parent.values[i] = parent.values[i + 1];

                parent.children[i + 1] = parent.children[i + 2];
            }

            parent.num_keys--;

            serialize_internal(node, node_data);
            write_block(node_block, node_data);
            serialize_internal(parent, parent_data);
            write_block(parent_block, parent_data);

            if (parent.num_keys >= MIN_INTERNAL_KEYS || path.size() == 1) {
                return;
            } else {
                path.pop_back();
                coalesce_internal_nodes(path, parent_block);
            }
            return;
        }
    }

  public:
    BPlusTree(std::fstream &file) : file(file) {

        if (file) {
            read_header();
        } else

        {
            file.open("database.bin", std::ios::out | std::ios::binary);
            file.close();
            file.open("database.bin", std::ios::in | std::ios::out | std::ios::binary);
            if (!file)
                throw std::runtime_error("cannot create db file");
            header.root_block = 0;
            header.first_leaf_block = 0;
            header.block_count = 1;
            write_header();
        }
    }

    void insert(long long key, int value) {

        if (header.root_block == 0) {
            LeafNode root;
            root.type = 1;
            root.num_keys = 1;
            root.next_leaf = 0;
            root.keys[0] = key;
            root.values[0] = value;
            int new_block = allocate_block();
            char data[BLOCK_SIZE];
            serialize_leaf(root, data);
            write_block(new_block, data);
            header.root_block = new_block;
            header.first_leaf_block = new_block;
            write_header();
            return;
        }

        vector<int> path;
        int current_block = header.root_block;

        while (true) {
            char data[BLOCK_SIZE];
            read_block(current_block, data);

            if (reinterpret_cast<int *>(data)[0] == 1) {
                break;
            } else {
                InternalNode node;
                parse_internal(data, node);
                int pos = 0;
                while (pos < node.num_keys && (node.keys[pos] < key || (node.keys[pos] == key &&
                                                                        node.values[pos] <= value)))
                    pos++;
                path.push_back(current_block);
                current_block = node.children[pos];
            }
        }
        LeafNode leaf;
        char leaf_data[BLOCK_SIZE];
        read_block(current_block, leaf_data);
        parse_leaf(leaf_data, leaf);

        int pos = 0;
        while (pos < leaf.num_keys &&
               (leaf.keys[pos] < key || (leaf.keys[pos] == key && leaf.values[pos] < value)))
            pos++;

        if (pos < leaf.num_keys && leaf.keys[pos] == key && leaf.values[pos] == value)
            return;

        for (int i = leaf.num_keys; i > pos; i--) {
            leaf.keys[i] = leaf.keys[i - 1];
            leaf.values[i] = leaf.values[i - 1];
        }

        leaf.keys[pos] = key;
        leaf.values[pos] = value;

        leaf.num_keys++;

        if (leaf.num_keys > MAX_LEAF_KEYS) {

            LeafNode new_leaf;
            new_leaf.type = 1;
            new_leaf.num_keys = leaf.num_keys / 2;
            leaf.num_keys -= new_leaf.num_keys;
            new_leaf.next_leaf = leaf.next_leaf;
            leaf.next_leaf = allocate_block();

            memcpy(new_leaf.keys, leaf.keys + leaf.num_keys, new_leaf.num_keys * sizeof(long long));
            memcpy(new_leaf.values, leaf.values + leaf.num_keys, new_leaf.num_keys * sizeof(int));

            char leaf_data[BLOCK_SIZE], new_leaf_data[BLOCK_SIZE];
            serialize_leaf(leaf, leaf_data);
            serialize_leaf(new_leaf, new_leaf_data);
            write_block(current_block, leaf_data);
            write_block(leaf.next_leaf, new_leaf_data);

            insert_into_parent(path, leaf.next_leaf, new_leaf.keys[0], new_leaf.values[0]);
        } else {
            char data[BLOCK_SIZE];
            serialize_leaf(leaf, data);
            write_block(current_block, data);
        }
    }

    vector<int> find(long long key) {
        vector<int> result;

        if (header.root_block == 0)
            return result;

        int current_block = header.root_block;

        while (true) {

            char data[BLOCK_SIZE];
            read_block(current_block, data);
            if (reinterpret_cast<int *>(data)[0] == 1)
                break;
            InternalNode node;
            parse_internal(data, node);
            int pos = 0;
            while (pos < node.num_keys && key > node.keys[pos])
                pos++;
            current_block = node.children[pos];
        }

        bool F = true;
        while (current_block != 0) {
            char data[BLOCK_SIZE];
            read_block(current_block, data);
            LeafNode leaf;
            parse_leaf(data, leaf);
            bool found = false;
            for (int i = 0; i < leaf.num_keys; ++i) {

                if (leaf.keys[i] == key) {

                    result.push_back(leaf.values[i]);
                    found = true;
                } else if (found) {
                    break;
                }
            }

            if (!found && !F)
                break;
            F = false;
            current_block = leaf.next_leaf;
        }

        return result;
    }

    void remove(long long key, int value) {
        vector<int> path;
        int current_block = header.root_block;
        if (head.root_block == 0) {
            return 0;
        }
        char data[BLOCK_SIZE];
        read_block(current_block, data);
        InternalNode node;
        parse_internal(data, node);

        while (true) {
            path.push_back(current_block);
            char data[BLOCK_SIZE];
            read_block(current_block, data);

            if (reinterpret_cast<int *>(data)[0] == 1)
                break;

            InternalNode node;
            parse_internal(data, node);
            int pos = 0;

            while (pos < node.num_keys &&
                   (key > node.keys[pos] || (key == node.keys[pos] && value >= node.values[pos])))
                pos++;

            current_block = node.children[pos];
        }

        LeafNode leaf;
        char leaf_data[BLOCK_SIZE];
        read_block(current_block, leaf_data);
        parse_leaf(leaf_data, leaf);
        int pos = -1;
        for (int i = 0; i < leaf.num_keys; i++) {
            if (leaf.keys[i] == key && leaf.values[i] == value) {
                pos = i;
                break;
            }
        }

        if (pos == -1) {
            return;
        }

        path.pop_back();
        remove_from_leaf(current_block, pos, path);
    }

    const int PRINT_INDENT = 4;
    const int KEY_WIDTH = 20;

    // 辅助函数：递归打印B+树结构
    void print_tree(int block_num, int level, std::fstream &file) {
        char data[BLOCK_SIZE];
        file.seekg(block_num * BLOCK_SIZE);
        file.read(data, BLOCK_SIZE);

        // 解析节点类型
        int node_type;
        memcpy(&node_type, data, sizeof(int));

        // 根据节点类型处理
        if (node_type == 1) { // 叶子节点
            LeafNode leaf;
            parse_leaf(data, leaf);

            // 打印缩进
            std::cout << std::string(level * PRINT_INDENT, ' ');

            // 打印节点类型和指针信息
            std::cout << "[L" << block_num << "]->" << leaf.next_leaf << ": ";

            // 打印所有键值
            for (int i = 0; i < leaf.num_keys; ++i) {

                std::cout << "[" << leaf.keys[i] << ":" << leaf.values[i] << "] ";
            }
            std::cout << std::endl;
        } else { // 内部节点
            InternalNode node;
            parse_internal(data, node);

            // 打印缩进
            std::cout << std::string(level * PRINT_INDENT, ' ');

            // 打印节点类型和键信息
            std::cout << "[N" << block_num << "]: ";
            for (int i = 0; i < node.num_keys; ++i) {

                std::cout << "|" << std::setw(KEY_WIDTH) << std::left << node.keys[i] << ":"
                          << node.values[i];
            }
            std::cout << "|" << std::endl;

            // 递归打印子节点

            for (int i = 0; i <= node.num_keys; ++i) {
                print_tree(node.children[i], level + 1, file);
            }
        }
    }

    // 公开接口函数
    void print_bptree_structure(std::fstream &file) {
        FileHeader header;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(FileHeader));

        if (header.root_block == 0) {
            std::cout << "Empty Tree" << std::endl;
            return;
        }

        std::cout << "B+ Tree Structure (Block Size: " << BLOCK_SIZE
                  << ", Max Leaf Keys: " << MAX_LEAF_KEYS << ")" << std::endl
                  << std::string(80, '-') << std::endl;

        print_tree(header.root_block, 0, file);

        std::cout << std::string(80, '-') << std::endl;
    }
};