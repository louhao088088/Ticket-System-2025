#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include <iostream>
#include <string>
#include <cstdio>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less<Key>
> 
class map {
public:
    typedef pair<const Key, T> value_type;

    // 红黑树节点定义
    enum Color { RED, BLACK };
    struct Node {
        value_type data;  // 键值对
        Node* left;       // 左子节点
        Node* right;      // 右子节点
        Node* parent;     // 父节点
        Color color;      // 颜色

        Node(const value_type& val) : data(val), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
    };

    // 红黑树实现
    class RBTree {
    private:
        Node* root;       // 根节点
        Compare comp;     // 比较器
        size_t count;        // 计数器
        // 左旋操作
        void leftRotate(Node* x) {
            Node* y = x->right;
            x->right = y->left;
            if (y->left) y->left->parent = x;
            y->parent = x->parent;
            if (!x->parent) root = y;
            else if (x == x->parent->left) x->parent->left = y;
            else x->parent->right = y;
            y->left = x;
            x->parent = y;
        }

        // 右旋操作
        void rightRotate(Node* y) {
            Node* x = y->left;
            y->left = x->right;
            if (x->right) x->right->parent = y;
            x->parent = y->parent;
            if (!y->parent) root = x;
            else if (y == y->parent->left) y->parent->left = x;
            else y->parent->right = x;
            x->right = y;
            y->parent = x;
        }

        // 插入修复
        void insertFixup(Node* z) {
            while (z->parent && z->parent->color == RED) {
                if (z->parent == z->parent->parent->left) {
                    Node* y = z->parent->parent->right;
                    if (y && y->color == RED) {
                        z->parent->color = BLACK;
                        y->color = BLACK;
                        z->parent->parent->color = RED;
                        z = z->parent->parent;
                    } else {
                        if (z == z->parent->right) {
                            z = z->parent;
                            leftRotate(z);
                        }
                        z->parent->color = BLACK;
                        z->parent->parent->color = RED;
                        rightRotate(z->parent->parent);
                    }
                } else {
                    Node* y = z->parent->parent->left;
                    if (y && y->color == RED) {
                        z->parent->color = BLACK;
                        y->color = BLACK;
                        z->parent->parent->color = RED;
                        z = z->parent->parent;
                    } else {
                        if (z == z->parent->left) {
                            z = z->parent;
                            rightRotate(z);
                        }
                        z->parent->color = BLACK;
                        z->parent->parent->color = RED;
                        leftRotate(z->parent->parent);
                    }
                }
            }
            root->color = BLACK;
        }

        // 移植节点
        void transplant(Node* u, Node* v) {
            if (!u->parent) root = v;
            else if (u == u->parent->left) u->parent->left = v;
            else u->parent->right = v;
            if (v) v->parent = u->parent;
        }

        // 删除修复
        void deleteFixup(Node* x, Node* x_parent) {
            while (x != root && (!x || x->color == BLACK)) {
                if (x == x_parent->left) {
                    Node* w = x_parent->right;
                    if (w->color == RED) {
                        w->color = BLACK;
                        x_parent->color = RED;
                        leftRotate(x_parent);
                        w = x_parent->right;
                    }
                    if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                        w->color = RED;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    } else {
                        if (!w->right || w->right->color == BLACK) {
                            if (w->left) w->left->color = BLACK;
                            w->color = RED;
                            rightRotate(w);
                            w = x_parent->right;
                        }
                        w->color = x_parent->color;
                        x_parent->color = BLACK;
                        if (w->right) w->right->color = BLACK;
                        leftRotate(x_parent);
                        x = root;
                    }
                } else {
                    Node* w = x_parent->left;
                    if (w->color == RED) {
                        w->color = BLACK;
                        x_parent->color = RED;
                        rightRotate(x_parent);
                        w = x_parent->left;
                    }
                    if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
                        w->color = RED;
                        x = x_parent;
                        x_parent = x_parent->parent;
                    } else {
                        if (!w->left || w->left->color == BLACK) {
                            if (w->right) w->right->color = BLACK;
                            w->color = RED;
                            leftRotate(w);
                            w = x_parent->left;
                        }
                        w->color = x_parent->color;
                        x_parent->color = BLACK;
                        if (w->left) w->left->color = BLACK;
                        rightRotate(x_parent);
                        x = root;
                    }
                }
            }
            if (x) x->color = BLACK;
        }

        // 查找最小节点
        Node* minimum(Node* node) {
            while (node->left) node = node->left;
            return node;
        }

        // 查找节点
        Node* findNode(const Key& key) const {
            Node* current = root;
            while (current) {
                if (comp(key, current->data.first)) current = current->left;
                else if (comp(current->data.first, key)) current = current->right;
                else return current;
            }
            return nullptr;
        }
        
        // 清空
        void dfsclear(Node *p) {
            if(p==nullptr) return;
            if(p->left) dfsclear(p->left);
            if(p->right) dfsclear(p->right);
            delete p;
        }

    public:
        RBTree() : root(nullptr), count(0) {}
        Node* getRoot() const { return root; }
        void changeRoot(Node *p){root=p;}
        // 插入操作
        pair<Node*, bool> insert(const value_type& value) {
            
            Node* z = new Node(value);
            Node* y = nullptr;
            Node* x = root;
            while (x) {
                y = x;
                if (comp(z->data.first, x->data.first)) x = x->left;
                else if (comp(x->data.first, z->data.first)) x = x->right;
                else {
                    delete z;
                    return pair<Node*, bool>(x, false);
                }
            }count++;
            z->parent = y;
            if (!y) root = z;
            else if (comp(z->data.first, y->data.first)) y->left = z;
            else y->right = z;
            insertFixup(z);
            return pair<Node*, bool>(z, true);
        }

        // 删除操作
        void erase(Node* z) {
            count--;
            Node* y = z;
            Color y_original_color = y->color;
            Node* x;
            Node* x_parent;
            if (!z->left) {
                x = z->right;
                x_parent = z->parent;
                transplant(z, z->right);
            } else if (!z->right) {
                x = z->left;
                x_parent = z->parent;
                transplant(z, z->left);
            } else {
                y = minimum(z->right);
                y_original_color = y->color;
                x = y->right;
                if (y->parent == z) {
                    if (x) x->parent = y;
                    x_parent = y;
                } else {
                    transplant(y, y->right);
                    x_parent = y->parent;
                    y->right = z->right;
                    y->right->parent = y;
                    
                }
                transplant(z, y);
                y->left = z->left;
                y->left->parent = y;
                y->color = z->color;
            }
            //if(x==nullptr)puts("A");
            //exit(0);
            if (y_original_color == BLACK) deleteFixup(x, x_parent);
            delete z;
        }

        // 查找操作
        Node* find(const Key& key) const {
            return findNode(key);
        }

        // 获取起始节点
        Node* begin() const {
            if (!root) return nullptr;
            Node* node = root;
            while (node->left) node = node->left;
            return node;
        }

        // 结束标志
        Node* end() const {
            return nullptr;
        }

        // 检查是否为空
        bool empty() const {
            return root == nullptr;
        }

        // 获取大小
        size_t size() const {
            return count; 
        }

        void changesize(size_t new_count) {
            count = new_count;
        }

        // 清空
        
        void clear() {
            dfsclear(root);
            count = 0;
            root = nullptr;
        }
    };

private:
    RBTree tree; // 红黑树实例
    Node* copyTree(Node* node, Node* parent) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->data);
        newNode->color = node->color;
        newNode->parent = parent;
        newNode->left = copyTree(node->left, newNode);
        newNode->right = copyTree(node->right, newNode);
        return newNode;
    }

public:
    // 迭代器
    class iterator;
    class const_iterator;

    class iterator {
    private:
        Node* node;
        const RBTree* tree;

        iterator(Node* n, const RBTree* t) : node(n), tree(t) {}

        friend class map;

    public:
        iterator() : node(nullptr), tree(nullptr) {}

        iterator(const iterator& other) : node(other.node), tree(other.tree) {}

        iterator& operator++() {
            if (!node) throw invalid_iterator();
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                Node* parent = node->parent;
                while (parent && node == parent->right) {
                    node = parent;
                    parent = parent->parent;
                }
                node = parent;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() {
            if(node == tree->begin()) throw invalid_iterator();
            if (!node) {
                node = tree->getRoot();
                while (node->right) node = node->right;
            } else if (node->left) {
                node = node->left;
                while (node->right) node = node->right;
            } else {
                Node* parent = node->parent;
                while (parent && node == parent->left) {
                    node = parent;
                    parent = parent->parent;
                }
                node = parent;
            }
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        value_type& operator*() const {
            return node->data;
        }

        value_type* operator->() const {
            return &(node->data);
        }

        bool operator==(const const_iterator& rhs) const {
            return (node == rhs.node) & (tree == rhs.tree);
        }

        bool operator!=(const const_iterator& rhs) const {
            return (node != rhs.node) | (tree != rhs.tree) ;
        }

        bool operator==(const iterator& rhs) const {
          return (node == rhs.node) & (tree == rhs.tree);
        }
        bool operator!=(const iterator& rhs) const {
          return (node != rhs.node) | (tree != rhs.tree) ;
        }
    };

    // 常量迭代器
    class const_iterator {
    private:
        const Node* node;
        const RBTree* tree;

        const_iterator(const Node* n, const RBTree* t) : node(n), tree(t) {}

        friend class map;

    public:
        const_iterator() : node(nullptr), tree(nullptr) {}

        const_iterator(const const_iterator& other) : node(other.node), tree(other.tree) {}

        const_iterator(const iterator& other) : node(other.node), tree(other.tree) {}

        const_iterator& operator++() {
            if (!node) throw invalid_iterator();
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } else {
                const Node* parent = node->parent;
                while (parent && node == parent->right) {
                    node = parent;
                    parent = parent->parent;
                }
                node = parent;
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() {
            if(node == tree->begin()) throw invalid_iterator();
            if (!node) {
                node = tree->getRoot();
                while (node->right) node = node->right;
            } else if (node->left) {
                node = node->left;
                while (node->right) node = node->right;
            } else {
                const Node* parent = node->parent;
                while (parent && node == parent->left) {
                    node = parent;
                    parent = parent->parent;
                }
                node = parent;
            }
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        const value_type& operator*() const {
            return node->data;
        }

        const value_type* operator->() const {
            return &(node->data);
        }

        bool operator==(const const_iterator& rhs) const {
            return (node == rhs.node) & (tree == rhs.tree);
        }

        bool operator!=(const const_iterator& rhs) const {
            return (node != rhs.node) | (tree != rhs.tree) ;
        }

        bool operator==(const iterator& rhs) const {
          return (node == rhs.node) & (tree == rhs.tree);
        }
        bool operator!=(const iterator& rhs) const {
          return (node != rhs.node) | (tree != rhs.tree) ;
        }
    };

    // 构造函数
    // Default constructor
    map() : tree() {}

    // Copy constructor
    map(const map& other) {
        if (other.tree.getRoot()) {
            tree.changeRoot(copyTree(other.tree.getRoot(), nullptr));
            tree.changesize(other.tree.size());
        }
    }

    // Assignment operator
    map& operator=(const map& other) {
       
        if (this != &other) {
            tree.clear();
            if (other.tree.getRoot()) {
                tree.changeRoot(copyTree(other.tree.getRoot(), nullptr));
                tree.changesize(other.tree.size());
            }
        }
        return *this;
    }

    // 析构函数
    ~map() {
        tree.clear();
    }

    // 访问元素
    T& at(const Key& key) {
        Node* node = tree.find(key);
        if (!node) throw index_out_of_bound();
        return node->data.second;
    }

    const T& at(const Key& key) const {
        Node* node = tree.find(key);
        if (!node) throw index_out_of_bound();
        return node->data.second;
    }

    T& operator[](const Key& key) {
        Node* node = tree.find(key);
        if (!node) {
            auto result = tree.insert(value_type(key, T()));
            return result.first->data.second;
        }
        return node->data.second;
    }

    const T& operator[](const Key& key) const {
        Node* node = tree.find(key);
        if (!node) throw index_out_of_bound();
        return node->data.second;
    }

    // 迭代器相关
    iterator begin() {
        return iterator(tree.begin(), &tree);
    }

    const_iterator cbegin() const {
        return const_iterator(tree.begin(), &tree);
    }

    iterator end() {
        return iterator(tree.end(), &tree);
    }

    const_iterator cend() const {
        return const_iterator(tree.end(), &tree);
    }

    // 容器状态
    bool empty() const {
        return tree.empty();
    }

    size_t size() const {
        return tree.size();
    }

    void clear() {
        tree.clear();
    }

    // 插入和删除
    pair<iterator, bool> insert(const value_type& value) {
        auto result = tree.insert(value);
        return pair<iterator, bool>(iterator(result.first, &tree), result.second);
    }

    void erase(iterator pos) {
        if (pos.node == nullptr || pos.tree != &tree) throw invalid_iterator();
        tree.erase(pos.node);
    }

    // 查询
    size_t count(const Key& key) const {
        return tree.find(key) ? 1 : 0;
    }

    iterator find(const Key& key) {
        Node* node = tree.find(key);
        return iterator(node, &tree);
    }

    const_iterator find(const Key& key) const {
        Node* node = tree.find(key);
        return const_iterator(node, &tree);
    }
};

}

#endif