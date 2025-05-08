#pragma once
#include <exception>

namespace sjtu {
  template<class T>
  class list {
  private:
    struct node {
      T val;
      node *prev;
      node *next;

      explicit node(const T &val, node *prev = nullptr, node *next = nullptr)
        : val(val), prev(prev), next(next) {
      }
    };

  public:
    int size_;
    node *head;
    node *tail;

    list() : size_(0), head(nullptr), tail(nullptr) {
    }

    list(int size, node *head, node *tail)
      : size_(size), head(head), tail(tail) {
    }

    // 深拷贝的拷贝构造函数
    list(const list<T> &other)
      : size_(0), head(nullptr), tail(nullptr) {
      for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
        push_back(*it);
      }
    }

    // 赋值运算符重载
    list<T> &operator=(const list<T> &other) {
      if (this != &other) {
        clear();
        for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
          insert_tail(*it);
        }
      }
      return *this;
    }

    ~list() { clear(); }

    class iterator {
    public:
      list *dl = nullptr;
      node *ptr = nullptr;

      explicit iterator(list *dl = nullptr, node *ptr = nullptr)
        : dl(dl), ptr(ptr) {
      }

      explicit iterator(node *node) : ptr(node) {
      }

      iterator(const iterator &t) {
        if (this == &t)
          return;
        dl = t.dl;
        ptr = t.ptr;
      }

      ~iterator() = default;

      /**
       * iter++
       */
      iterator operator++(int) {
        iterator tmp = *this;
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->end())
          throw std::runtime_error("");
        ptr = ptr->next;
        return tmp;
      }

      /**
       * ++iter
       */
      iterator &operator++() {
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->end())
          throw std::runtime_error("");
        ptr = ptr->next;
        return *this;
      }

      /**
       * iter--
       */
      iterator operator--(int) {
        iterator temp = *this;
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->begin())
          throw std::runtime_error("");
        ptr = ptr->prev;
        return temp;
      }

      /**
       * --iter
       */
      iterator &operator--() {
        iterator temp = *this;

        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->begin())
          throw std::runtime_error("");
        ptr = ptr->prev;
        return *this;
      }

      iterator operator+(int n) {
        iterator temp = *this;
        if (n < 0) {
          for (int i = 0; i < -n; i++) {
            if (temp.ptr == nullptr)
              throw std::runtime_error("");
            temp.ptr = temp.ptr->prev;
          }
        } else {
          for (int i = 0; i < n; i++) {
            if (temp.ptr == nullptr)
              throw std::runtime_error("");
            temp.ptr = temp.ptr->next;
          }
        }
        return temp;
      }

      /**
       * if the iter didn't point to a value
       * throw " invalid"
       */
      T &operator*() const {
        if (ptr == nullptr)
          throw std::runtime_error("");
        return ptr->val;
      }

      /**
       * other operation
       */
      T *operator->() const noexcept { return &(ptr->val); }

      bool operator==(const iterator &rhs) const {
        if (this == &rhs)
          return true;
        if (dl != rhs.dl)
          return false;
        return ptr == rhs.ptr;
      }

      bool operator!=(const iterator &rhs) const {
        if (this == &rhs)
          return false;

        if (dl != rhs.dl)
          return true;
        return ptr != rhs.ptr;
      }
    };

    //  list also uses const interator, for the linked_hashmap
    class const_iterator {
    public:
      const list *dl = nullptr; // 关联的双向链表
      node *ptr = nullptr; // 当前指向的节点

      explicit const_iterator(const list *dl = nullptr, node *ptr = nullptr)
        : dl(dl), ptr(ptr) {
      }

      const_iterator(const const_iterator &t) = default;

      ~const_iterator() = default;

      const_iterator operator++(int) {
        const_iterator tmp = *this;
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->cend())
          throw std::runtime_error("");
        ptr = ptr->next;
        return tmp;
      }

      const_iterator &operator++() {
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->cend())
          throw std::runtime_error("");
        ptr = ptr->next;
        return *this;
      }

      const_iterator operator--(int) {
        const_iterator temp = *this;
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->cbegin())
          throw std::runtime_error("");
        ptr = ptr->prev;
        return temp;
      }

      const_iterator &operator--() {
        if (ptr == nullptr)
          throw std::runtime_error("");
        if (*this == dl->cbegin())
          throw std::runtime_error("");
        ptr = ptr->prev;
        return *this;
      }

      const_iterator operator+(int n) const {
        const_iterator temp = *this;
        if (n < 0) {
          for (int i = 0; i < -n; i++) {
            if (temp.ptr == nullptr)
              throw std::runtime_error("");
            temp.ptr = temp.ptr->prev;
          }
        } else {
          for (int i = 0; i < n; i++) {
            if (temp.ptr == nullptr)
              throw std::runtime_error("");
            temp.ptr = temp.ptr->next;
          }
        }
        return temp;
      }

      const T &operator*() const {
        if (ptr == nullptr)
          throw std::runtime_error("");
        return ptr->val;
      }

      const T *operator->() const noexcept {
        if (ptr == nullptr)
          throw std::runtime_error("");
        return &(ptr->val);
      }

      bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }

      bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
    };

    // begin() and end()
    const_iterator cbegin() const { return const_iterator(this, head); }

    const_iterator cend() const { return const_iterator(this, nullptr); }

    iterator begin() {
      if (head == nullptr)
        return iterator(this, nullptr);
      return iterator(this, head);
    }

    iterator end() { return iterator(this, nullptr); }

    iterator get_tail() const { return iterator(tail); }

    iterator erase(iterator pos) {
      if (pos.ptr == nullptr)
        return end();
      node *p = pos.ptr;
      node *prev = p->prev;
      node *next = p->next;

      if (prev)
        prev->next = next;
      else
        head = next;
      if (next)
        next->prev = prev;
      else
        tail = prev;

      delete p; // 释放被删除节点的内存
      size_--; // 更新链表长度

      return (next) ? iterator(this, next) : end();
    }
    T& front() {
      return head->val;
    }

    T& back() {
      return tail->val;
    }
    /**
     * the following are operations of double list
     */
    iterator push_front(const T &val) {
      node *new_node = new node(val, nullptr, head);
      if (head != nullptr)
        head->prev = new_node;
      head = new_node;
      if (tail == nullptr)
        tail = head;
      size_++;
      return iterator(head);
    }

    iterator push_back(const T &val) {
      node *new_node = new node(val, tail, nullptr);
      if (tail != nullptr)
        tail->next = new_node;
      tail = new_node;
      if (head == nullptr)
        head = tail;
      size_++;
      return iterator(tail);
    }

    void pop_front() {
      if (head == nullptr)
        return;
      node *temp = head;
      head = head->next;
      if (head != nullptr)
        head->prev = nullptr;
      delete temp;
      size_--;
    }

    void pop_back() {
      if (tail == nullptr)
        return;
      node *temp = tail;
      tail = tail->prev;
      if (tail != nullptr)
        tail->next = nullptr;
      delete temp;
      size_--;
    }

    [[nodiscard]] bool empty() const {
      if (size_ == 0)
        return true;
      return false;
    }
    int size() {
      return size_;
    }
    void clear() {
      node *cur = head;
      while (cur) {
        node *tmp = cur;
        cur = cur->next;
        delete tmp;
      }
      size_ = 0;
      head = tail = nullptr;
    }
  };
}
