#pragma once
#include "Hash.h"
#include "map/map.hpp"
#include "priority_queue/src/priority_queue.hpp"
#include "vector/vector.hpp"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

using std::cerr;
using std::cin;
using std::cout;
using std::string;

const int BLOCK_SIZE = 1024;
const int KEY_SIZE = 12;

const int MAX_LEAF_KEYS = (BLOCK_SIZE - 12) / KEY_SIZE - 1;
const int MAX_INTERNAL_KEYS = (BLOCK_SIZE - 12) / (KEY_SIZE + 4) - 1;
const int MIN_LEAF_KEYS = std::max(1, (MAX_LEAF_KEYS + 1) / 2 - 1);
const int MIN_INTERNAL_KEYS = std::max(1, MAX_INTERNAL_KEYS / 2 - 1);

template <typename T> using vector = sjtu::vector<T>;

template <typename Key, typename Value, typename Compare = std::less<Key>>
using map = sjtu::map<Key, Value, Compare>;

template <typename T, class Compare = std::less<T>>
using priority_queue = sjtu::priority_queue<T, Compare>;

//根
struct FileHeader {
    int root_block;
    int first_leaf_block;
    int block_count;
};

//内部节点
struct InternalNode {
    int type;
    int num_keys;
    int children[MAX_INTERNAL_KEYS + 2];
    long long keys[MAX_INTERNAL_KEYS + 1];
    int values[MAX_INTERNAL_KEYS + 1];
};

//叶子节点
struct LeafNode {
    int type;
    int num_keys;
    int next_leaf;
    long long keys[MAX_LEAF_KEYS + 1];
    int values[MAX_LEAF_KEYS + 1];
};

// 缓存
struct CacheNode {
    int block_num;
    char data[BLOCK_SIZE];
    bool is_dirty;

    CacheNode *prev;
    CacheNode *next;

    CacheNode(int bn) : block_num(bn), is_dirty(false), prev(nullptr), next(nullptr) {
        memset(data, 0, BLOCK_SIZE);
    }
};

// 缓存
class LRUCache {
  private:
    int capacity;
    map<int, CacheNode *> cache_map;
    CacheNode *head;
    CacheNode *tail;

    //更新访问顺序
    void move_to_head(CacheNode *node) {
        assert(node != nullptr);
        if (node == head)
            return;

        if (node->prev)
            node->prev->next = node->next;
        if (node->next)
            node->next->prev = node->prev;

        if (node == tail)
            tail = node->prev;

        node->prev = nullptr;
        node->next = head;
        if (head)
            head->prev = node;
        head = node;

        if (!tail)
            tail = head;
    }

    // 淘汰最久未使用节点
    void remove_tail(std::fstream &file) {
        if (!tail)
            return;

        CacheNode *old_tail = tail;
        CacheNode *prev = old_tail->prev;

        if (old_tail->is_dirty) {
            file.seekp(old_tail->block_num * BLOCK_SIZE);
            file.write(old_tail->data, BLOCK_SIZE);
        }

        cache_map.erase(cache_map.find(old_tail->block_num));

        if (prev)
            prev->next = nullptr;
        else
            head = nullptr;

        tail = prev;
        old_tail->prev = nullptr;
        old_tail->next = nullptr;
        delete old_tail;
    }

  public:
    LRUCache(int cap) : capacity(cap), head(nullptr), tail(nullptr) {}

    ~LRUCache() {
        CacheNode *curr = head;
        while (curr) {
            CacheNode *next = curr->next;
            curr->prev = nullptr;
            curr->next = nullptr;
            delete curr;
            curr = next;
        }
        head = tail = nullptr;
        cache_map.clear();
    }

    // 获取缓存数据，返回nullptr表示未命中
    char *get(int block_num) {
        auto it = cache_map.find(block_num);
        if (it == cache_map.end())
            return nullptr;
        CacheNode *node = it->second;
        move_to_head(node);
        return node->data;
    }

    // 添加或更新缓存
    void put(int block_num, const char *data, std::fstream &file, bool is_dirty = false) {
        auto it = cache_map.find(block_num);
        if (it != cache_map.end()) {

            CacheNode *node = it->second;
            memcpy(node->data, data, BLOCK_SIZE);
            node->is_dirty = is_dirty;
            move_to_head(node);
        } else {

            if (int(cache_map.size()) >= capacity) {
                remove_tail(file);
            }
            CacheNode *node = new CacheNode(block_num);
            memcpy(node->data, data, BLOCK_SIZE);
            node->is_dirty = is_dirty;
            cache_map[block_num] = node;
            move_to_head(node);
        }
    }

    // 标记节点为脏
    void mark_dirty(int block_num) {
        auto it = cache_map.find(block_num);
        if (it != cache_map.end()) {
            it->second->is_dirty = true;
        }
    }

    // 将所有脏数据写回磁盘
    void flush(std::fstream &file) {
        CacheNode *curr = head;
        while (curr) {
            if (curr->is_dirty) {
                file.seekp(curr->block_num * BLOCK_SIZE);
                file.write(curr->data, BLOCK_SIZE);
                curr->is_dirty = false;
            }
            curr = curr->next;
        }
    }
};

class BPlusTree {
  private:
    std::fstream file;
    std::string filename;
    FileHeader header;
    LRUCache block_cache;

    //文件读写操作
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
        char *cached_data = block_cache.get(block_num);
        if (cached_data) {
            memcpy(data, cached_data, BLOCK_SIZE);
        } else {
            file.seekg(block_num * BLOCK_SIZE);
            file.read(data, BLOCK_SIZE);
            block_cache.put(block_num, data, file);
        }
    }

    void write_block(int block_num, const char *data) {
        block_cache.put(block_num, data, file, true);
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
        memcpy(leaf.keys, data + 3 * sizeof(int),
               (MAX_LEAF_KEYS + 1) * sizeof(long long));
        memcpy(leaf.values,
               data + 3 * sizeof(int) + (MAX_LEAF_KEYS + 1) * sizeof(long long),
               (MAX_LEAF_KEYS + 1) * sizeof(int));
    }

    void serialize_leaf(const LeafNode &leaf, char *data) {
        memset(data, 0, BLOCK_SIZE);
        memcpy(data, &leaf.type, sizeof(int));
        memcpy(data + sizeof(int), &leaf.num_keys, sizeof(int));
        memcpy(data + 2 * sizeof(int), &leaf.next_leaf, sizeof(int));
        memcpy(data + 3 * sizeof(int), leaf.keys, leaf.num_keys * sizeof(long long));
        memcpy(data + 3 * sizeof(int) + (MAX_LEAF_KEYS + 1) * sizeof(long long),
               leaf.values, leaf.num_keys * sizeof(int));
    }

    void parse_internal(const char *data, InternalNode &node) {
        memcpy(&node.type, data, sizeof(int));
        memcpy(&node.num_keys, data + sizeof(int), sizeof(int));
        memcpy(node.children, data + 2 * sizeof(int),
               (MAX_INTERNAL_KEYS + 2) * sizeof(int));
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

    //把键值插入内部点
    void insert_into_parent(vector<int> &path, int child_block, long long key,
                            int value) {
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

        int pos = 0;
        while (pos < parent.num_keys &&
               (parent.keys[pos] < key ||
                (parent.keys[pos] == key && parent.values[pos] <= value)))
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
            memcpy(new_node.keys, parent.keys + split + 1,
                   new_node.num_keys * sizeof(long long));
            memcpy(new_node.values, parent.values + split + 1,
                   new_node.num_keys * sizeof(int));

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

    //从叶子中移除某个键值
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

        //从左右兄弟中借一个值
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

        //和左右兄弟合并
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

    //删除内部点的某个键值
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

        //从左右兄弟中借一个值
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

        //和左右兄弟合并
        if (left_sibling != -1) {

            InternalNode left;
            char left_data[BLOCK_SIZE];
            read_block(left_sibling, left_data);
            parse_internal(left_data, left);

            memcpy(node.keys + left.num_keys + 1, node.keys,
                   node.num_keys * sizeof(long long));
            memcpy(node.values + left.num_keys + 1, node.values,
                   node.num_keys * sizeof(int));

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

            memcpy(node.keys + node.num_keys + 1, right.keys,
                   right.num_keys * sizeof(long long));
            memcpy(node.values + node.num_keys + 1, right.values,
                   right.num_keys * sizeof(int));

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
    BPlusTree(const string &filename) : filename(filename), block_cache(2048) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

        if (!file) {
            std::ofstream create(filename, std::ios::out | std::ios::binary);
            if (!create)
                throw std::runtime_error("cannot create db file");
            create.close();

            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            if (!file)
                throw std::runtime_error("cannot reopen created file");
            header.root_block = 0;
            header.first_leaf_block = 0;
            header.block_count = 1;
            write_header();
        } else {
            read_header();
            // std::cerr << header.root_block << " " << filename << "\n";
        }
    }
    //清空缓存
    void flush() { block_cache.flush(file); }

    //插入
    bool empty() {
        if (header.root_block == 0)
            return 1;
        else
            return 0;
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
                while (pos < node.num_keys &&
                       (node.keys[pos] < key ||
                        (node.keys[pos] == key && node.values[pos] <= value)))
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
        while (
            pos < leaf.num_keys &&
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

            memcpy(new_leaf.keys, leaf.keys + leaf.num_keys,
                   new_leaf.num_keys * sizeof(long long));
            memcpy(new_leaf.values, leaf.values + leaf.num_keys,
                   new_leaf.num_keys * sizeof(int));

            char leaf_data[BLOCK_SIZE], new_leaf_data[BLOCK_SIZE];
            serialize_leaf(leaf, leaf_data);
            serialize_leaf(new_leaf, new_leaf_data);
            write_block(current_block, leaf_data);
            write_block(leaf.next_leaf, new_leaf_data);

            insert_into_parent(path, leaf.next_leaf, new_leaf.keys[0],
                               new_leaf.values[0]);
        } else {
            char data[BLOCK_SIZE];
            serialize_leaf(leaf, data);
            write_block(current_block, data);
        }
    }

    //查找
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

    //删除
    void remove(long long key, int value) {
        vector<int> path;
        int current_block = header.root_block;
        if (header.root_block == 0) {
            return;
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
                   (key > node.keys[pos] ||
                    (key == node.keys[pos] && value >= node.values[pos])))
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
        int node_type;
        memcpy(&node_type, data, sizeof(int));
        if (node_type == 1) {
            LeafNode leaf;
            parse_leaf(data, leaf);
            std::cout << std::string(level * PRINT_INDENT, ' ');
            std::cout << "[L" << block_num << "]->" << leaf.next_leaf << ": ";
            for (int i = 0; i < leaf.num_keys; ++i) {

                std::cout << "[" << leaf.keys[i] << ":" << leaf.values[i] << "] ";
            }
            std::cout << std::endl;
        } else {
            InternalNode node;
            parse_internal(data, node);
            std::cout << std::string(level * PRINT_INDENT, ' ');

            std::cout << "[N" << block_num << "]: ";
            for (int i = 0; i < node.num_keys; ++i) {

                std::cout << "|" << std::setw(KEY_WIDTH) << std::left << node.keys[i]
                          << ":" << node.values[i];
            }
            std::cout << "|" << std::endl;
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