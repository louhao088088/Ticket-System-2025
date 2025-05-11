#include "vector.hpp"

#include <algorithm>
#include <bits/stdc++.h>
#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

const int BLOCK_SIZE = 1024;
const int KEY_SIZE = 68;

const int MAX_LEAF_KEYS = (BLOCK_SIZE - 12) / KEY_SIZE - 1;
const int MAX_INTERNAL_KEYS = (BLOCK_SIZE - 12) / (KEY_SIZE + 4) - 1;
const int MIN_LEAF_KEYS = max(1, (MAX_LEAF_KEYS + 1) / 2 - 1);
const int MIN_INTERNAL_KEYS = max(1, MAX_INTERNAL_KEYS / 2 - 1);

struct FileHeader {
    int root_block;
    int first_leaf_block;
    int block_count;
};

struct InternalNode {
    int type;
    int num_keys;
    int children[MAX_INTERNAL_KEYS + 2];
    char keys[MAX_INTERNAL_KEYS + 1][KEY_SIZE];
};

struct LeafNode {
    int type;
    int num_keys;
    int next_leaf;
    char keys[MAX_LEAF_KEYS + 1][KEY_SIZE];
};

class BPlusTree {
  private:
    fstream &file;
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
        memset(data, 0, sizeof data);
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
        memcpy(leaf.keys, data + 3 * sizeof(int), (MAX_LEAF_KEYS + 1) * KEY_SIZE);
    }

    void serialize_leaf(const LeafNode &leaf, char *data) {
        memset(data, 0, sizeof data);
        memcpy(data, &leaf.type, sizeof(int));
        memcpy(data + sizeof(int), &leaf.num_keys, sizeof(int));
        memcpy(data + 2 * sizeof(int), &leaf.next_leaf, sizeof(int));
        memcpy(data + 3 * sizeof(int), leaf.keys, leaf.num_keys * KEY_SIZE);
    }

    void parse_internal(const char *data, InternalNode &node) {
        memcpy(&node.type, data, sizeof(int));
        memcpy(&node.num_keys, data + sizeof(int), sizeof(int));
        memcpy(node.children, data + 2 * sizeof(int), (MAX_INTERNAL_KEYS + 2) * sizeof(int));
        memcpy(node.keys, data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int),
               (MAX_INTERNAL_KEYS + 1) * KEY_SIZE);
    }

    void serialize_internal(const InternalNode &node, char *data) {
        memset(data, 0, sizeof data);
        memcpy(data, &node.type, sizeof(int));
        memcpy(data + sizeof(int), &node.num_keys, sizeof(int));
        memcpy(data + 2 * sizeof(int), node.children, (node.num_keys + 1) * sizeof(int));
        memcpy(data + 2 * sizeof(int) + (MAX_INTERNAL_KEYS + 2) * sizeof(int), node.keys,
               node.num_keys * KEY_SIZE);
    }

    int compare_keys(const char *a, const char *b) {
        int cmp = memcmp(a, b, 64);
        if (cmp != 0)
            return cmp;
        int val_a, val_b;
        memcpy(&val_a, a + 64, sizeof(int));
        memcpy(&val_b, b + 64, sizeof(int));
        return (val_a < val_b) ? -1 : (val_a > val_b) ? 1 : 0;
    }

    void insert_into_parent(const vector<int> &path, int child_block, const char *key) {
        if (path.empty()) {
            InternalNode new_root;
            new_root.type = 0;
            new_root.num_keys = 1;
            new_root.children[0] = header.root_block;
            memcpy(new_root.keys[0], key, KEY_SIZE);
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
        while (pos < parent.num_keys && compare_keys(parent.keys[pos], key) < 0)
            pos++;

        for (int i = parent.num_keys; i > pos; i--) {
            memcpy(parent.keys[i], parent.keys[i - 1], KEY_SIZE);
            parent.children[i + 1] = parent.children[i];
        }
        memcpy(parent.keys[pos], key, KEY_SIZE);
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
            memcpy(new_node.keys, parent.keys + split + 1, new_node.num_keys * KEY_SIZE);

            int new_block = allocate_block();
            char new_data[BLOCK_SIZE];

            serialize_internal(new_node, new_data);
            write_block(new_block, new_data);

            char parent_data[BLOCK_SIZE];
            serialize_internal(parent, parent_data);
            write_block(parent_block, parent_data);

            insert_into_parent(vector<int>(path.begin(), path.end() - 1), new_block,
                               parent.keys[split]);

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
            memcpy(leaf.keys[i], leaf.keys[i + 1], KEY_SIZE);
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

        if (left_sibling != -1) {
            LeafNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_leaf(left_data, left);

            if (left.num_keys > MIN_LEAF_KEYS) {
                for (int i = leaf.num_keys; i > 0; i--) {
                    memcpy(leaf.keys[i], leaf.keys[i - 1], KEY_SIZE);
                }
                memcpy(leaf.keys[0], left.keys[left.num_keys - 1], KEY_SIZE);
                leaf.num_keys++;
                left.num_keys--;

                memcpy(parent.keys[parent_pos], leaf.keys[0], KEY_SIZE);

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
                memcpy(leaf.keys[leaf.num_keys], right.keys[0], KEY_SIZE);
                leaf.num_keys++;
                for (int i = 0; i < right.num_keys - 1; i++) {
                    memcpy(right.keys[i], right.keys[i + 1], KEY_SIZE);
                }
                right.num_keys--;

                memcpy(parent.keys[parent_pos + 1], right.keys[0], KEY_SIZE);

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
                memcpy(left.keys[left.num_keys + i], leaf.keys[i], KEY_SIZE);
            }
            left.num_keys += leaf.num_keys;
            left.next_leaf = leaf.next_leaf;

            assert(left.num_keys <= MAX_LEAF_KEYS);

            serialize_leaf(left, left_data);
            write_block(left_sibling, left_data);

            for (int i = parent_pos; i < parent.num_keys - 1; i++) {
                memcpy(parent.keys[i], parent.keys[i + 1], KEY_SIZE);
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
                coalesce_internal_nodes(path, parent_block, parent_pos);
            }
        } else if (right_sibling != -1) {

            LeafNode right;
            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_leaf(right_data, right);

            for (int i = 0; i < right.num_keys; i++) {
                memcpy(leaf.keys[leaf.num_keys + i], right.keys[i], KEY_SIZE);
            }
            leaf.num_keys += right.num_keys;
            leaf.next_leaf = right.next_leaf;

            assert(leaf.num_keys <= MAX_LEAF_KEYS);

            serialize_leaf(leaf, leaf_data);
            write_block(leaf_block, leaf_data);

            for (int i = parent_pos + 1; i < parent.num_keys - 1; i++) {
                memcpy(parent.keys[i], parent.keys[i + 1], KEY_SIZE);
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
                coalesce_internal_nodes(path, parent_block, parent_pos + 1);
            }
        }
    }

    void coalesce_internal_nodes(vector<int> &path, int node_block, int pos) {
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

        if (left_sibling != -1) {
            InternalNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_internal(left_data, left);

            if (left.num_keys > MIN_INTERNAL_KEYS) {

                node.children[node.num_keys + 1] = node.children[node.num_keys];
                for (int i = node.num_keys; i > 0; i--) {
                    memcpy(node.keys[i], node.keys[i - 1], KEY_SIZE);
                    node.children[i] = node.children[i - 1];
                }
                memcpy(node.keys[0], parent.keys[parent_pos], KEY_SIZE);
                memcpy(parent.keys[parent_pos], left.keys[left.num_keys - 1], KEY_SIZE);
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
                memcpy(node.keys[node.num_keys], parent.keys[parent_pos + 1], KEY_SIZE);
                memcpy(parent.keys[parent_pos + 1], right.keys[0], KEY_SIZE);
                node.children[node.num_keys + 1] = right.children[0];
                for (int i = 0; i < right.num_keys - 1; i++) {
                    memcpy(right.keys[i], right.keys[i + 1], KEY_SIZE);
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

            memcpy(node.keys + left.num_keys + 1, node.keys, node.num_keys * KEY_SIZE);
            memcpy(node.children + left.num_keys + 1, node.children,
                   (node.num_keys + 1) * sizeof(int));
            memcpy(node.keys, left.keys, left.num_keys * KEY_SIZE);
            memcpy(node.children, left.children, (left.num_keys + 1) * sizeof(int));
            memcpy(node.keys[left.num_keys], parent.keys[parent_pos], KEY_SIZE);
            node.num_keys += left.num_keys + 1;

            assert(node.num_keys <= MAX_INTERNAL_KEYS);

            for (int i = parent_pos - 1; i < parent.num_keys - 1; i++) {
                if (i != parent_pos - 1)
                    memcpy(parent.keys[i], parent.keys[i + 1], KEY_SIZE);
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
                coalesce_internal_nodes(path, parent_block, parent_pos);
            }
            return;
        }
        if (right_sibling != -1) {
            InternalNode right;

            char right_data[BLOCK_SIZE];
            read_block(right_sibling, right_data);
            parse_internal(right_data, right);

            memcpy(node.keys[node.num_keys], parent.keys[parent_pos + 1], KEY_SIZE);
            memcpy(node.keys + node.num_keys + 1, right.keys, right.num_keys * KEY_SIZE);
            memcpy(node.children + node.num_keys + 1, right.children,
                   (right.num_keys + 1) * sizeof(int));
            node.num_keys += right.num_keys + 1;

            assert(node.num_keys <= MAX_INTERNAL_KEYS);

            for (int i = parent_pos + 1; i < parent.num_keys - 1; i++) {
                memcpy(parent.keys[i], parent.keys[i + 1], KEY_SIZE);
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
                coalesce_internal_nodes(path, parent_block, parent_pos);
            }
            return;
        }
    }

  public:
    BPlusTree(fstream &file) : file(file) {

        if (file) {
            read_header();
        } else

        {
            file.open("database.bin", ios::out | ios::binary);
            file.close();
            file.open("database.bin", ios::in | ios::out | ios::binary);
            if (!file)
                throw std::runtime_error("cannot create db file");
            header.root_block = 0;
            header.first_leaf_block = 0;
            header.block_count = 1;
            write_header();
        }
    }

    void insert(const char *key) {

        if (header.root_block == 0) {
            LeafNode root;
            root.type = 1;
            root.num_keys = 1;
            root.next_leaf = 0;
            memcpy(root.keys[0], key, KEY_SIZE);
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
                while (pos < node.num_keys && compare_keys(key, node.keys[pos]) >= 0)
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
        while (pos < leaf.num_keys && compare_keys(leaf.keys[pos], key) < 0)
            pos++;

        if (pos < leaf.num_keys && compare_keys(leaf.keys[pos], key) == 0)
            return;

        for (int i = leaf.num_keys; i > pos; i--)
            memcpy(leaf.keys[i], leaf.keys[i - 1], KEY_SIZE);
        memcpy(leaf.keys[pos], key, KEY_SIZE);

        leaf.num_keys++;

        if (leaf.num_keys > MAX_LEAF_KEYS) {

            LeafNode new_leaf;
            new_leaf.type = 1;
            new_leaf.num_keys = leaf.num_keys / 2;
            leaf.num_keys -= new_leaf.num_keys;
            new_leaf.next_leaf = leaf.next_leaf;
            leaf.next_leaf = allocate_block();

            memcpy(new_leaf.keys, leaf.keys + leaf.num_keys, new_leaf.num_keys * KEY_SIZE);

            char leaf_data[BLOCK_SIZE], new_leaf_data[BLOCK_SIZE];
            serialize_leaf(leaf, leaf_data);
            serialize_leaf(new_leaf, new_leaf_data);
            write_block(current_block, leaf_data);
            write_block(leaf.next_leaf, new_leaf_data);

            insert_into_parent(path, leaf.next_leaf, new_leaf.keys[0]);
        } else {
            char data[BLOCK_SIZE];
            serialize_leaf(leaf, data);
            write_block(current_block, data);
        }
    }

    vector<int> find(const char *index) {
        vector<int> result;

        if (header.root_block == 0)
            return result;

        char start_key[KEY_SIZE];
        memset(start_key, 0, 64);
        strncpy(start_key, index, 64);
        int min_val = INT_MIN;

        memcpy(start_key + 64, &min_val, sizeof(int));

        int current_block = header.root_block;

        while (true) {

            char data[BLOCK_SIZE];
            read_block(current_block, data);
            if (reinterpret_cast<int *>(data)[0] == 1)
                break;
            InternalNode node;
            parse_internal(data, node);
            int pos = 0;
            while (pos < node.num_keys && compare_keys(start_key, node.keys[pos]) >= 0)
                pos++;
            current_block = node.children[pos];
        }

        // cout << "Find" << current_block << endl;
        bool F = true;
        while (current_block != 0) {
            char data[BLOCK_SIZE];
            read_block(current_block, data);
            LeafNode leaf;
            parse_leaf(data, leaf);
            bool found = false;
            for (int i = 0; i < leaf.num_keys; ++i) {
                // cout << leaf.keys[i] << " " << index << endl;
                if (memcmp(leaf.keys[i], index, 64) == 0) {
                    int value;
                    memcpy(&value, leaf.keys[i] + 64, sizeof(int));
                    result.push_back(value);
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

        // std::sort(result.begin(), result.end());
        return result;
    }

    void remove(const char *key) {
        vector<int> path;
        int current_block = header.root_block;
        // cout << header.root_block << endl;

        char data[BLOCK_SIZE];
        read_block(current_block, data);
        InternalNode node;
        parse_internal(data, node);

        // cout << " " << node.type << " " << node.num_keys << " " << node.children[0] << endl;

        while (true) {
            path.push_back(current_block);
            char data[BLOCK_SIZE];
            read_block(current_block, data);

            if (reinterpret_cast<int *>(data)[0] == 1)
                break;

            InternalNode node;
            parse_internal(data, node);
            int pos = 0;
            // cout << pos << " " << node.type << " " << node.num_keys << " " << node.children[pos]
            //  << endl;
            while (pos < node.num_keys && compare_keys(key, node.keys[pos]) >= 0)
                pos++;

            current_block = node.children[pos];
        }

        LeafNode leaf;
        char leaf_data[BLOCK_SIZE];
        read_block(current_block, leaf_data);
        parse_leaf(leaf_data, leaf);
        int pos = -1;
        for (int i = 0; i < leaf.num_keys; i++) {
            if (compare_keys(leaf.keys[i], key) == 0) {
                pos = i;
                break;
            }
        }
        // cout << pos << endl;
        if (pos == -1) {
            return;
        }

        path.pop_back();
        remove_from_leaf(current_block, pos, path);
    }

    const int PRINT_INDENT = 4;
    const int KEY_WIDTH = 20;

    // 辅助函数：递归打印B+树结构
    void print_tree(int block_num, int level, fstream &file) {
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
            cout << string(level * PRINT_INDENT, ' ');

            // 打印节点类型和指针信息
            cout << "[L" << block_num << "]->" << leaf.next_leaf << ": ";

            // 打印所有键值
            for (int i = 0; i < leaf.num_keys; ++i) {
                char index[65] = {0};
                int value;
                memcpy(index, leaf.keys[i], 64);
                memcpy(&value, leaf.keys[i] + 64, sizeof(int));
                cout << "[" << index << ":" << value << "] ";
            }
            cout << endl;
        } else { // 内部节点
            InternalNode node;
            parse_internal(data, node);

            // 打印缩进
            cout << string(level * PRINT_INDENT, ' ');

            // 打印节点类型和键信息
            cout << "[N" << block_num << "]: ";
            for (int i = 0; i < node.num_keys; ++i) {
                char index[65] = {0};
                int value;
                memcpy(index, node.keys[i], 64);
                memcpy(&value, node.keys[i] + 64, sizeof(int));
                cout << "|" << setw(KEY_WIDTH) << left << (string(index) + ":" + to_string(value));
            }
            cout << "|" << endl;

            // 递归打印子节点

            // cout << node.num_keys << " " << node.children[0] << endl;
            for (int i = 0; i <= node.num_keys; ++i) {
                print_tree(node.children[i], level + 1, file);
            }
        }
    }

    // 公开接口函数
    void print_bptree_structure(fstream &file) {
        FileHeader header;
        file.seekg(0);
        file.read(reinterpret_cast<char *>(&header), sizeof(FileHeader));

        if (header.root_block == 0) {
            cout << "Empty Tree" << endl;
            return;
        }

        cout << "B+ Tree Structure (Block Size: " << BLOCK_SIZE
             << ", Max Leaf Keys: " << MAX_LEAF_KEYS << ")" << endl
             << string(80, '-') << endl;

        print_tree(header.root_block, 0, file);

        cout << string(80, '-') << endl;
        // cout << "Legend: [Lx] = Leaf Block x, [Nx] = Internal Node x" << endl;
    }
};