#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>

namespace sjtu {
  /**
   * a container like std::priority_queue which is a heap internal.
   */
  template<typename T, class Compare = std::less<T> >
  class priority_queue {
    size_t size_ = 0;
    Compare comp_;

    struct Node {
      T value;

      Node *child = nullptr, *next = nullptr;

      explicit Node(T value): value(value) {
      }
    };

    Node *head_ = nullptr;

    void clear(Node *obj) {
      if (!obj) {
        return;
      }
      if (obj->child) {
        clear(obj->child);
      }
      if (obj->next) {
        clear(obj->next);
      }
      delete obj;
    }

    void copy(Node *obj, Node *target) {
      if (target->child) {
        obj->child = new Node(*target->child);
        copy(obj->child, target->child);
      }
      if (target->next) {
        obj->next = new Node(*target->next);
        copy(obj->next, target->next);
      }
    }

    Node *rearrange(Node *x) {
      if (x == nullptr || x->next == nullptr) {
        return x;
      }
      Node *y = x->next;
      Node *z = y->next;
      x->next = y->next = nullptr;
      return Node_merge(rearrange(z), Node_merge(x, y));
    }

    Node *Node_merge(Node *x, Node *y) {
      if (x == nullptr) {
        return y;
      }
      if (y == nullptr) {
        return x;
      }
      if (comp_(x->value, y->value)) {
        x->next = y->child;
        y->child = x;
        return y;
      } else {
        y->next = x->child;
        x->child = y;
        return x;
      }
    }

  public:
    /**
     * constructors
     */
    priority_queue() = default;

    priority_queue(const priority_queue &other) {
      if (head_) {
        clear(head_);
      }
      size_ = other.size_;
      if (size_ != 0) {
        head_ = new Node(*other.head_);
        copy(head_, other.head_);
      }
    }

    /**
     * deconstructor
     */
    ~priority_queue() {
      clear(head_);
    }

    /**
     * Assignment operator
     */
    priority_queue &operator=(const priority_queue &other) {
      if (&other == this) {
        return *this;
      }
      if (head_) {
        clear(head_);
        head_ = nullptr;
      }
      size_ = other.size_;
      if (size_ != 0) {
        head_ = new Node(*other.head_);
        copy(head_, other.head_);
      }
      return *this;
    }

    /**
     * get the top of the queue.
     * @return a reference of the top element.
     * throw container_is_empty if empty() returns true;
     */
    const T &top() const {
      if (empty()) {
        throw std::runtime_error("");
      }
      return head_->value;
    }

    /**
     *
     * push new element to the priority queue.
     */
    void push(const T &e) {
      if (head_ == nullptr) {
        size_ = 1;
        head_ = new Node(e);
        return;
      }
      Node *tmp = new Node(e);
      head_ = Node_merge(head_, tmp);
      size_++;
    }

    /**
     *
     * delete the top element.
     * throw container_is_empty if empty() returns true;
     */
    void pop() {
      if (empty()) {
        throw std::runtime_error("");
      }
      Node *tmp = rearrange(head_->child);
      delete head_;
      head_ = tmp;
      size_--;
    }

    /**
     * return the number of the elements.
     */
    size_t size() const {
      return size_;
    }

    /**
     * check if the container has at least an element.
     * @return true if it is empty, false if it has at least an element.
     */
    bool empty() const {
      if (size_ == 0) {
        return true;
      }
      return false;
    }

    /**
     * merge two priority_queues with at most O(logn) complexity.
     * clear the other priority_queue.
     */
    void merge(priority_queue &other) {
      if (other.empty()) {
        return;
      }
      if (empty()) {
        head_ = other.head_;
        other.head_ = nullptr;
        size_ = other.size();
        other.size_ = 0;
        return;
      }
      head_ = Node_merge(head_, other.head_);
      other.head_ = nullptr;
      size_ += other.size();
      other.size_ = 0;
    }
  };
};
