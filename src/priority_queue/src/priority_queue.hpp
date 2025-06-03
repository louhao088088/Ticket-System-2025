#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <functional>

namespace sjtu {

template <typename T> struct Node {
    T value;
    Node *left;
    Node *right;
    Node(const T &v) : value(v), left(nullptr), right(nullptr) {}
};

template <typename T, class Compare = std::less<T>> class priority_queue {
  private:
    Node<T> *root;
    Compare cmp;
    size_t current_size;

    Node<T> *merge(Node<T> *a, Node<T> *b) {
        if (!a)
            return b;
        if (!b)
            return a;
        if (cmp(a->value, b->value)) {
            std::swap(a, b);
        }
        a->right = merge(a->right, b);
        std::swap(a->left, a->right);
        return a;
    }

    Node<T> *copy(const Node<T> *node) {
        if (!node)
            return nullptr;
        Node<T> *newNode = new Node<T>(node->value);
        newNode->left = copy(node->left);
        newNode->right = copy(node->right);
        return newNode;
    }

    void clear(Node<T> *node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }

  public:
    priority_queue() : root(nullptr), current_size(0) {}
    priority_queue(const Compare &c = Compare())
        : root(nullptr), cmp(c), current_size(0) {}
    priority_queue(const priority_queue &other)
        : root(nullptr), current_size(other.current_size) {
        root = copy(other.root);
    }

    ~priority_queue() { clear(root); }

    priority_queue &operator=(const priority_queue &other) {
        if (this != &other) {
            clear(root);
            root = copy(other.root);
            current_size = other.current_size;
        }
        return *this;
    }

    const T &top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->value;
    }

    void push(const T &e) {
        Node<T> *newNode = new Node<T>(e);
        try {
            root = merge(root, newNode);
            ++current_size;
        } catch (...) {
            delete newNode;
            throw;
        }
    }

    void pop() {
        if (empty()) {
            throw container_is_empty();
        }
        Node<T> *oldRoot = root;
        Node<T> *left = oldRoot->left;
        Node<T> *right = oldRoot->right;
        try {
            root = merge(left, right);
        } catch (...) {
            root = oldRoot;
            throw;
        }
        delete oldRoot;
        --current_size;
    }

    size_t size() const { return current_size; }

    bool empty() const { return current_size == 0; }

    void merge(priority_queue &other) {
        root = merge(root, other.root);
        current_size += other.current_size;
        other.root = nullptr;
        other.current_size = 0;
    }
};

} // namespace sjtu

#endif