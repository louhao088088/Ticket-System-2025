#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <iostream>

namespace sjtu {

template <typename T> class vector {
  private:
    T *data;
    size_t current_size;
    size_t current_capacity;
    void reserve(size_t new_capacity) {
        if (new_capacity <= current_capacity)
            return;
        T *new_data = static_cast<T *>(operator new(new_capacity * sizeof(T)));
        for (size_t i = 0; i < current_size; ++i) {
            new (new_data + i) T(std::move(data[i]));
            data[i].~T();
        }
        operator delete(data);
        data = new_data;
        current_capacity = new_capacity;
    }

  public:
    class const_iterator;
    class iterator {

      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

      private:
        T *ptr;
        vector *parent;

      public:
        /**
         * return a new iterator which pointer n-next elements
         * as well as operator-
         */
        iterator(T *ptr = nullptr, vector *parent = nullptr) : ptr(ptr), parent(parent) {}

        iterator operator+(const int &n) const { return iterator(ptr + n, parent); }
        iterator operator-(const int &n) const { return iterator(ptr - n, parent); }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const iterator &rhs) const {
            if (parent != rhs.parent)
                throw invalid_iterator();
            return ptr - rhs.ptr;
        }
        iterator &operator+=(const int &n) {
            ptr += n;
            return *this;
        }
        iterator &operator-=(const int &n) {
            ptr -= n;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = iterator(ptr, parent);
            ptr++;
            return tmp;
        }
        iterator &operator++() {
            ptr++;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = iterator(ptr, parent);
            ptr--;
            return tmp;
        }
        iterator &operator--() {
            ptr--;
            return *this;
        }
        T &operator*() const { return *ptr; }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory
         * address).
         */
        bool operator==(const iterator &rhs) const {
            return (rhs.parent == parent && rhs.ptr == ptr);
        }
        bool operator==(const const_iterator &rhs) const {
            return (rhs.parent == parent && rhs.ptr == ptr);
        }
        bool operator<(const iterator &rhs) const { return rhs.ptr < ptr; }
        bool operator<=(const iterator &rhs) const { return rhs.ptr <= ptr; }
        bool operator<(const const_iterator &rhs) const { return rhs.ptr < ptr; }
        bool operator<=(const const_iterator &rhs) const { return rhs.ptr <= ptr; }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     */
    class const_iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

      private:
        const T *ptr;
        const vector *parent;

      public:
        /**
         * return a new iterator which pointer n-next elements
         * as well as operator-
         */
        const_iterator(const T *ptr = nullptr, const vector *parent = nullptr)
            : ptr(ptr), parent(parent) {}

        const_iterator operator+(const int &n) const {
            return const_iterator(ptr + n, parent);
        }
        const_iterator operator-(const int &n) const {
            return const_iterator(ptr - n, parent);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const const_iterator &rhs) const {
            if (parent != rhs.parent)
                throw invalid_iterator();
            return ptr - rhs.ptr;
        }
        const_iterator &operator+=(const int &n) {
            ptr += n;
            return *this;
        }
        const_iterator &operator-=(const int &n) {
            ptr -= n;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = const_iterator(ptr, parent);
            ptr++;
            return tmp;
        }
        const_iterator &operator++() {
            ptr++;
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator tmp = const_iterator(ptr, parent);
            ptr--;
            return tmp;
        }
        const_iterator &operator--() {
            ptr--;
            return *this;
        }
        const T &operator*() const { return *ptr; }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory
         * address).
         */
        bool operator==(const iterator &rhs) const {
            return (rhs.parent == parent && rhs.ptr == ptr);
        }
        bool operator==(const const_iterator &rhs) const {
            return (rhs.parent == parent && rhs.ptr == ptr);
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
        bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
    };

    vector() : data(nullptr), current_size(0), current_capacity(0) {}
    vector(const vector &other) : data(nullptr), current_size(0), current_capacity(0) {

        reserve(other.current_capacity);
        current_size = other.current_size;
        for (size_t i = 0; i < current_size; ++i) {
            new (data + i) T(other.data[i]);
        }
    }
    ~vector() {
        for (size_t i = 0; i < current_size; i++) {
            data[i].~T();
        }
        operator delete(data);
        current_size = 0;
        current_capacity = 0;
    }
    vector &operator=(const vector &other) {
        if (this == &other)
            return *this;
        for (size_t i = 0; i < current_size; ++i) {
            data[i].~T();
        }
        operator delete(data);
        current_capacity = other.current_capacity;
        current_size = other.current_size;
        data = static_cast<T *>(operator new(current_capacity * sizeof(T)));
        for (size_t i = 0; i < current_size; ++i) {
            new (data + i) T(other.data[i]);
        }
        return *this;
    }
    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     */
    T &at(const size_t &pos) {
        if (pos >= current_size) {
            std::cerr << pos << " " << current_size << "\n";
            exit(0);
            throw index_out_of_bound();
        }

        return data[pos];
    }
    const T &at(const size_t &pos) const {
        if (pos >= current_size) {
            std::cerr << pos << " " << current_size << "\n";
            exit(0);
            throw index_out_of_bound();
        }

        return data[pos];
    }
    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     * !!! Pay attentions
     *   In STL this operator does not check the boundary but I want you to do.
     */
    T &operator[](const size_t &pos) { return at(pos); }
    const T &operator[](const size_t &pos) const { return at(pos); }
    /**
     * access the first element.
     * throw container_is_empty if size == 0
     */
    const T &front() const {
        if (current_size == 0)
            throw container_is_empty();
        return data[0];
    }
    /**
     * access the last element.
     * throw container_is_empty if size == 0
     */
    const T &back() const {
        if (current_size == 0)
            throw container_is_empty();
        return data[current_size - 1];
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() { return iterator(data, this); }
    const_iterator begin() const { return const_iterator(data, this); }
    const_iterator cbegin() const { return const_iterator(data, this); }
    /**
     * returns an iterator to the end.
     */
    iterator end() { return iterator(data + current_size, this); }
    const_iterator end() const { return const_iterator(data + current_size, this); }
    const_iterator cend() const { return const_iterator(data + current_size, this); }
    /**
     * checks whether the container is empty
     */
    bool empty() const {
        if (current_size == 0)
            return 1;
        return 0;
    }
    /**
     * returns the number of elements
     */
    size_t size() const { return current_size; }
    /**
     * clears the contents
     */
    void clear() {
        for (int i = 0; i < current_size; i++)
            data[i].~T();
        operator delete(data);
        current_size = 0;
        current_capacity = 0;

        data = nullptr;
    }
    /**
     * inserts value before pos
     * returns an iterator pointing to the inserted value.
     */
    iterator insert(iterator pos, const T &value) {
        size_t index = pos - begin();
        if (current_size == current_capacity) {
            reserve(current_capacity ? current_capacity * 2 : 4);
        }
        T *pos_ptr = data + index;
        for (size_t i = current_size; i > index; --i) {
            new (data + i) T(std::move(data[i - 1]));
            data[i - 1].~T();
        }
        new (data + index) T(value);
        ++current_size;
        return iterator(pos_ptr, this);
    }
    /**
     * inserts value at index ind.
     * after inserting, this->at(ind) == value
     * returns an iterator pointing to the inserted value.
     * throw index_out_of_bound if ind > size (in this situation ind can be size because
     * after inserting the size will increase 1.)
     */
    iterator insert(const size_t &ind, const T &value) {
        if (ind > current_size) {
            std::cerr << ind << " " << current_size << "\n";
            exit(0);
            throw index_out_of_bound();
        }

        return insert(begin() + ind, value);
    }
    /**
     * removes the element at pos.
     * return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     */
    iterator erase(iterator pos) {
        size_t index = pos - begin();
        data[index].~T();
        for (size_t i = index + 1; i < current_size; ++i) {
            new (data + i - 1) T(std::move(data[i]));
            data[i].~T();
        }
        --current_size;
        return iterator(data + index, this);
    }
    /**
     * removes the element with index ind.
     * return an iterator pointing to the following element.
     * throw index_out_of_bound if ind >= size
     */
    iterator erase(const size_t &ind) {
        if (ind > current_size) {
            std::cerr << ind << " " << current_size << "\n";
            exit(0);
            throw index_out_of_bound();
        }

        return erase(begin() + ind);
    }
    /**
     * adds an element to the end.
     */
    void push_back(const T &value) {
        if (current_size == current_capacity) {
            reserve(current_capacity ? current_capacity * 2 : 4);
        }
        new (data + current_size) T(value);
        ++current_size;
    }
    /**
     * remove the last element from the end.
     * throw container_is_empty if size() == 0
     */
    void pop_back() {
        if (current_size == 0)
            throw container_is_empty();
        data[--current_size].~T();
    }
};

} // namespace sjtu

#endif