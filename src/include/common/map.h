/**
 * implement a container like std::map
 */
#pragma once
// only for std::less<T>
#include <functional>
#include <cstddef>

namespace sjtu {
  template<class T1, class T2>
  class pair {
  public:
    T1 first;
    T2 second;

    constexpr pair() : first(), second() {
    }

    pair(const pair &other) = default;

    pair(pair &&other) = default;

    pair(const T1 &x, const T2 &y) : first(x), second(y) {
    }

    template<class U1, class U2>
    pair(U1 &&x, U2 &&y) : first(x), second(y) {
    }

    template<class U1, class U2>
    pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {
    }

    template<class U1, class U2>
    pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {
    }
  };


  template<
    class Key,
    class T,
    class Compare = std::less<Key> >
  class map {
  public:
    /**
 * the internal type of data.
 * it should have a default constructor, a copy constructor.
 * You can use sjtu::map as value_type by typedef.
 */
    typedef pair<const Key, T> value_type;

  private:
    struct Node {
      Node *father = nullptr;
      Node *left = nullptr;
      Node *right = nullptr;
      int factor = 0;
      value_type value;
    };

    Node *root_ = nullptr;
    size_t size_ = 0;
    Compare compare = Compare();
    Node *begin_ = nullptr;
    Node *end_ = nullptr;

    void Clear(Node *&obj) {
      if (obj == nullptr) {
        return;
      }
      if (obj->left != nullptr) {
        Clear(obj->left);
      }
      if (obj->right != nullptr) {
        Clear(obj->right);
      }
      Delete(obj);
    }

    Node *New(const value_type &value) {
      Node *tmp = static_cast<Node *>(malloc(sizeof(Node)));
      tmp->father = tmp->left = tmp->right = nullptr;
      tmp->factor = 0;
      new(&tmp->value)value_type(std::move(value));
      tmp->factor = 0;
      return tmp;
    }

    Node *New(Node *other) {
      Node *tmp = static_cast<Node *>(malloc(sizeof(Node)));
      tmp->father = tmp->left = tmp->right = nullptr;
      tmp->factor = 0;
      new(&tmp->value)value_type(std::move(other->value));
      return tmp;
    }

    void Delete(Node *&obj) {
      if (obj == nullptr) {
        return;
      }
      obj->value.first.~Key();
      obj->value.second.~T();
      free(obj);
      obj = nullptr;
    }

    void Copy(Node *obj, Node *target) {
      if (target->left != nullptr) {
        obj->left = New(target->left);
        obj->left->factor = target->left->factor;
        obj->left->father = obj;
        Copy(obj->left, target->left);
      }
      if (target->right != nullptr) {
        obj->right = New(target->right);
        obj->right->factor = target->right->factor;
        obj->right->father = obj;
        Copy(obj->right, target->right);
      }
    }

    //Left Rotation
    //links the rl node to the right of current node
    //the right node becomes the parent of current node
    //factor will be changed in this function
    void RotateL(Node *obj) {
      Node *parent = obj->father;
      Node *right = obj->right;
      Node *right_left = obj->right->left;
      //obj <-->right left
      obj->right = right_left;
      if (right_left != nullptr) {
        right_left->father = obj;
      }
      //obj <-->right
      obj->father = right;
      right->left = obj;
      //right <--> parent
      if (parent == nullptr) {
        root_ = right;
        right->father = parent;
      } else {
        if (obj == parent->left) {
          parent->left = right;
        } else {
          parent->right = right;
        }
        right->father = parent;
      }
      obj->factor = right->factor = 0;
    }

    //Right Rotation
    //links the lr node to the left of current node
    //the left node becomes the parent of current node
    //factor will be changed in this function
    void RotateR(Node *obj) {
      Node *parent = obj->father;
      Node *left = obj->left;
      Node *left_right = left->right;
      //obj <-->left right
      obj->left = left_right;
      if (left_right != nullptr) {
        left_right->father = obj;
      }
      //obj <-->left
      obj->father = left;
      left->right = obj;
      //left <--> parent
      if (parent == nullptr) {
        root_ = left;
        left->father = parent;
      } else {
        if (obj == parent->left) {
          parent->left = left;
        } else {
          parent->right = left;
        }
        left->father = parent;
      }
      obj->factor = left->factor = 0;
    }

    //Rotate the left child left then rotate obj right
    //This is used under minus situation where left child is positive
    //After rotation left_right node will be root
    //factor will be changed in this function
    void RotateLR(Node *obj) {
      Node *left = obj->left;
      Node *left_right = left->right;
      auto record = left_right->factor;
      RotateL(left);
      RotateR(obj);
      switch (record) {
        case 0: {
          obj->factor = left->factor = left_right->factor = 0;
          break;
        }
        case -1: {
          obj->factor = 1;
          left_right->factor = left->factor = 0;
          break;
        }
        case 1: {
          left->factor = -1;
          left_right->factor = obj->factor = 0;
          break;
        }
        default: {
          break;
        }
      }
    }

    void RotateRL(Node *obj) {
      Node *right = obj->right;
      Node *right_left = right->left;
      auto record = right_left->factor;
      RotateR(right);
      RotateL(obj);
      switch (record) {
        case 0: {
          obj->factor = right->factor = right_left->factor = 0;
          break;
        }
        case -1: {
          right_left->factor = obj->factor = 0;
          right->factor = 1;
          break;
        }
        case 1: {
          obj->factor = -1;
          right->factor = right_left->factor = 0;
          break;
        }
        default: {
          break;
        }
      }
    }

    //Rotation when erase
    //factor will be changed in this function
    //Stops when cur->factor==1 or -1
    //Equally current height would not be changed
    //Exists two new situations
    void RotateE(Node *obj) {
      Node *last = nullptr;
      while (obj) {
        switch (obj->factor) {
          case 1: {
            return;
          }
          case -1: {
            return;
          }
          case 0: {
            last = obj;
            obj = obj->father;
            break;
          }
          case 2: {
            switch (obj->right->factor) {
              case 0: {
                RotateL(obj);
                obj->father->factor = -1; //Satisfies end condition
                obj->factor = 1;
                return;
              }
              case 1: {
                RotateL(obj);
                last = obj->father;
                obj = last->father;
                break;
              }
              case -1: {
                RotateRL(obj);
                last = obj->father;
                obj = last->father;
                break;
              }
              default: {
                exit(1);
              }
            }
            break;
          }
          case -2: {
            switch (obj->left->factor) {
              case 0: {
                RotateR(obj);
                obj->factor = -1;
                obj->father->factor = 1;
                return;
              }
              case -1: {
                RotateR(obj);
                last = obj->father;
                obj = last->father;
                break;
              }
              case 1: {
                RotateLR(obj);
                last = obj->father;
                obj = last->father;
                break;
              }
              default: {
                exit(1);
              }
            }
            break;
          }
          default: {
            exit(1);
          }
        }
        //determine the influence of erase
        if (obj && obj->left == last) {
          ++obj->factor;
        } else if (obj && obj->right == last) {
          --obj->factor;
        }
      }
    }

  public:
    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = map.begin(); --it;
     *       or it = map.end(); ++end();
     */
    class const_iterator;

    class iterator {
      friend map;

    private:
      /**
       * add data members
       *   just add whatever you want.
       */
      Node *value = nullptr;
      map *container = nullptr;

    public:
      iterator() = default;

      iterator(const iterator &other) {
        value = other.value;
        container = other.container;
      }

      iterator(Node *value, map *container): value(value), container(container) {
      }

      /**
       * iter++
       */
      iterator operator++(int) {
        if (value == nullptr) {
          throw std::runtime_error("");
        }
        iterator tmp = *this;
        operator++();
        return tmp;
      }

      /**
       * ++iter
       */
      iterator &operator++() {
        if (value == nullptr) {
          throw std::runtime_error("");
        }
        if (value->right) {
          auto tmp = value->right;
          while (tmp->left) {
            tmp = tmp->left;
          }
          return *this = iterator(tmp, container);
        }
        auto tmp = value->father;
        while (tmp && container->compare(tmp->value.first, value->value.first)) {
          tmp = tmp->father;
        }
        if (tmp) {
          return *this = iterator(tmp, container);
        }
        return *this = iterator(nullptr, container);
      }

      /**
       * iter--
       */
      iterator operator--(int) {
        auto tmp = *this;
        operator--();
        return tmp;
      }

      /**
       * --iter
       */
      iterator &operator--() {
        if (value == nullptr) {
          if (container->end_ && !container->end_->right) {
            return *this = iterator(container->end_, container);
          }
          auto tmp = container->root_;
          if (!container->root_) {
            throw std::runtime_error("");
          }
          while (tmp->right) {
            tmp = tmp->right;
          }
          return *this = iterator(tmp, container);
        }
        if (value->left) {
          auto tmp = value->left;
          while (tmp->right) {
            tmp = tmp->right;
          }
          return *this = iterator(tmp, container);
        }
        auto tmp = value->father;
        while (tmp && !container->compare(tmp->value.first, value->value.first)) {
          tmp = tmp->father;
        }
        if (tmp) {
          return *this = iterator(tmp, container);
        }
        throw std::runtime_error("");
      }

      /**
       * a operator to check whether two iterators are same (pointing to the same memory).
       */
      value_type &operator*() const {
        return value->value;
      }

      bool operator==(const iterator &rhs) const {
        if (value == rhs.value && container == rhs.container) {
          return true;
        }
        return false;
      }

      bool operator==(const const_iterator &rhs) const {
        if (value == rhs.value && container == rhs.container) {
          return true;
        }
        return false;
      }

      /**
       * some other operator for iterator.
       */
      bool operator!=(const iterator &rhs) const {
        return !operator==(rhs);
      }

      bool operator!=(const const_iterator &rhs) const {
        return !operator==(rhs);
      }

      /**
       * for the support of it->first.
       * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
       */
      value_type *operator->() const noexcept {
        return &(value->value);
      }
    };

    class const_iterator {
      // it should has similar member method as iterator.
      //  and it should be able to construct from an iterator.
      friend map;

    private:
      // data members.
      Node *value = nullptr;
      const map *container = nullptr;

    public:
      const_iterator() = default;

      const_iterator(const iterator &other) {
        value = other.value;
        container = other.container;
      }

      const_iterator(const const_iterator &other) {
        value = other.value;
        container = other.container;
      }

      const_iterator(Node *value, const map *container): value(value), container(container) {
      }

      /**
       * iter++
       */
      const_iterator operator++(int) {
        const_iterator tmp = *this;
        operator++();
        return tmp;
      }

      /**
       * ++iter
       */
      const_iterator &operator++() {
        if (value == nullptr) {
          throw std::runtime_error("");
        }
        if (value->right) {
          auto tmp = value->right;
          while (tmp->left) {
            tmp = tmp->left;
          }
          return *this = const_iterator(tmp, container);
        }
        auto tmp = value->father;
        while (tmp && container->compare(tmp->value.first, value->value.first)) {
          tmp = tmp->father;
        }
        if (tmp) {
          return *this = const_iterator(tmp, container);
        }
        return *this = const_iterator(nullptr, container);
      }

      /**
       * iter--
       */
      const_iterator operator--(int) {
        auto tmp = *this;
        operator--();
        return tmp;
      }

      /**
       * --iter
       */
      const_iterator &operator--() {
        if (value == nullptr) {
          if (container->end_ && !container->end_->right) {
            return *this = const_iterator(container->end_, container);
          }
          auto tmp = container->root_;
          if (!container->root_) {
            throw std::runtime_error("");
          }
          while (tmp->right) {
            tmp = tmp->right;
          }
          return *this = const_iterator(tmp, container);
        }
        if (value->left) {
          auto tmp = value->left;
          while (tmp->right) {
            tmp = tmp->right;
          }
          return *this = const_iterator(tmp, container);
        }
        auto tmp = value->father;
        while (tmp && !container->compare(tmp->value.first, value->value.first)) {
          tmp = tmp->father;
        }
        if (tmp) {
          return *this = const_iterator(tmp, container);
        }
        throw std::runtime_error("");
      }

      /**
       * a operator to check whether two iterators are same (pointing to the same memory).
       */
      value_type &operator*() const {
        return value->value;
      }

      bool operator==(const iterator &rhs) const {
        if (value == rhs.value && container == rhs.container) {
          return true;
        }
        return false;
      }

      bool operator==(const const_iterator &rhs) const {
        if (value == rhs.value && container == rhs.container) {
          return true;
        }
        return false;
      }

      /**
       * some other operator for iterator.
       */
      bool operator!=(const iterator &rhs) const {
        return !operator==(rhs);
      }

      bool operator!=(const const_iterator &rhs) const {
        return !operator==(rhs);
      }

      /**
       * for the support of it->first.
       * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
       */
      value_type *operator->() const noexcept {
        return &(value->value);
      }
    };

    /**
     * two constructors
     */
    map() = default;

    map(const map &other) {
      size_ = other.size();
      if (other.empty()) {
        return;
      }
      root_ = New(other.root_);
      root_->factor = other.root_->factor;
      Copy(root_, other.root_);
    }

    /**
     * assignment operator
     */
    map &operator=(const map &other) {
      if (&other == this) {
        return *this;
      }
      if (root_) {
        Clear(root_);
      }
      size_ = other.size();
      if (other.empty()) {
        return *this;
      }
      root_ = New(other.root_);
      root_->factor = other.root_->factor;
      Copy(root_, other.root_);
      return *this;
    }

    /**
     */
    ~map() {
      if (root_) {
        Clear(root_);
      }
    }

    /**
     *
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key equivalent to key.
     * If no such element exists, an exception of type `index_out_of_bound'
     */
    T &at(const Key &key) {
      auto tmp = find(key);
      if (!tmp.value) {
        throw std::runtime_error("");
      }
      return tmp.value->value.second;
    }

    const T &at(const Key &key) const {
      auto tmp = find(key);
      if (!tmp.value) {
        throw std::runtime_error("");
      }
      return tmp.value->value.second;
    }

    /**
     *
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     *   performing an insertion if such key does not already exist.
     */
    T &operator[](const Key &key) {
      auto tmp = insert(value_type(key, T()));
      return tmp.first->second;
    }

    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T &operator[](const Key &key) const {
      return at(key);
    }

    /**
     * return a iterator to the beginning
     */
    iterator begin() {
      if (!root_) {
        return iterator(nullptr, this);
      }
      if (begin_ && !begin_->left) {
        return iterator(begin_, this);
      }
      auto tmp = root_;
      while (tmp->left) {
        tmp = tmp->left;
      }
      begin_ = tmp;
      return iterator(tmp, this);
    }

    const_iterator cbegin() const {
      if (!root_) {
        throw std::runtime_error("");
      }
      auto tmp = root_;
      while (tmp->left) {
        tmp = tmp->left;
      }
      return const_iterator(tmp, this);
    }

    /**
     * return a iterator to the end
     * in fact, it returns past-the-end.
     */
    iterator end() {
      return iterator(nullptr, this);
    }

    const_iterator cend() const {
      return const_iterator(nullptr, this);
    }

    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {
      if (size_ == 0) {
        return true;
      }
      return false;
    }

    /**
     * returns the number of elements.
     */
    size_t size() const {
      return size_;
    }

    /**
     * clears the contents
     */
    void clear() {
      size_ = 0;
      if (root_) {
        Clear(root_);
      }
    }

    pair<iterator, bool> insert(const Key &key, const T &value) {
      return insert(pair(key, value));
    }

    /* insert an element.
    * return a pair, the first of the pair is
    *   the iterator to the new element (or the element that prevented the insertion),
    *   the second one is true if insert successfully, or false.
    */
    pair<iterator, bool> insert(const value_type &value) {
      begin_ = nullptr;
      end_ = nullptr;
      if (!root_) {
        root_ = New(value);
        size_++;
        return pair(iterator(root_, this), true);
      }
      Node *tmp = root_, *prev = root_;
      while (tmp) {
        if (compare(value.first, tmp->value.first)) {
          prev = tmp;
          tmp = tmp->left;
        } else if (compare(tmp->value.first, value.first)) {
          prev = tmp;
          tmp = tmp->right;
        } else {
          return pair(iterator(tmp, this), false);
        }
      }
      Node *cur = New(value);
      Node *pos = cur;
      size_++;
      if (compare(prev->value.first, value.first)) {
        prev->right = cur;
        cur->father = prev;
      } else {
        prev->left = cur;
        cur->father = prev;
      }
      while (prev) {
        if (prev->left == cur) {
          --prev->factor;
        } else {
          ++prev->factor;
        }
        if (prev->factor == 0) {
          break;
        }
        if (prev->factor == 1 || prev->factor == -1) {
          cur = prev;
          prev = prev->father;
        } else if (prev->factor == -2) {
          if (cur->factor == -1) {
            RotateR(prev);
          } else if (cur->factor == 1) {
            RotateLR(prev);
          }
          break;
        } else if (prev->factor == 2) {
          if (cur->factor == 1) {
            RotateL(prev);
          } else if (cur->factor == -1) {
            RotateRL(prev);
          }
          break;
        } else {
          exit(1);
        }
      }
      return pair(iterator(pos, this), true);
    }

    void erase(Key &key) {
      auto it = find(key);
      if (it != end()) {
        erase(find(key));
      }
    }

    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     */
    void erase(iterator pos) {
      begin_ = nullptr;
      end_ = nullptr;
      if (pos.value == nullptr || pos.container != this) {
        throw std::runtime_error("");
      }
      if (!root_) {
        throw std::runtime_error("");
      }
      size_--;
      auto cur = pos.value;
      if (!cur->left || !cur->right) {
        if (cur == root_) {
          if (!cur->left) {
            root_ = root_->right;
          } else {
            root_ = root_->left;
          }
          Delete(cur);
          if (root_) {
            root_->factor = 0;
          }
        } else if (!cur->left && !cur->right) {
          auto prev = cur->father;
          if (cur == prev->left) {
            ++prev->factor;
            prev->left = nullptr;
          } else {
            --prev->factor;
            prev->right = nullptr;
          }
          Delete(cur);
          RotateE(prev);
        } else if (!cur->left) {
          auto tmp = cur->right;
          if (cur->father->left == cur) {
            cur->father->left = tmp;
          } else {
            cur->father->right = tmp;
          }
          tmp->father = cur->father;
          tmp->factor = cur->factor;
          --tmp->factor;
          Delete(cur);
          RotateE(tmp);
        } else if (!cur->right) {
          auto tmp = cur->left;
          if (cur->father->left == cur) {
            cur->father->left = tmp;
          } else {
            cur->father->right = tmp;
          }
          tmp->father = cur->father;
          tmp->factor = cur->factor;
          ++tmp->factor;
          Delete(cur);
          RotateE(tmp);
        }
      } else {
        Node *successor = cur->right;
        while (successor->left) {
          successor = successor->left;
        }
        if (successor == cur->right) {
          if (!cur->father) {
            root_ = successor;
          } else if (cur == cur->father->left) {
            cur->father->left = successor;
          } else {
            cur->father->right = successor;
          }
          successor->father = cur->father;
          successor->left = cur->left;
          if (cur->left) {
            cur->left->father = successor;
          }
          successor->factor = cur->factor;
          --successor->factor;
          Delete(cur);
          RotateE(successor);
        } else {
          auto succPar = successor->father;
          if (successor->right) {
            auto tmp = successor->right;
            succPar->left = tmp;
            tmp->father = succPar;
            tmp->factor = successor->factor;
            if (!cur->father) {
              root_ = successor;
            } else if (cur == cur->father->left) {
              cur->father->left = successor;
            } else {
              cur->father->right = successor;
            }
            successor->father = cur->father;
            successor->factor = cur->factor;
            if (cur->left) {
              cur->left->father = successor;
            }
            successor->left = cur->left;
            if (cur->right) {
              cur->right->father = successor;
            }
            successor->right = cur->right;
            Delete(cur);
            --tmp->factor;
            RotateE(tmp);
          } else {
            succPar->left = nullptr;
            if (!cur->father) {
              root_ = successor;
            } else if (cur == cur->father->left) {
              cur->father->left = successor;
            } else {
              cur->father->right = successor;
            }
            successor->father = cur->father;
            successor->factor = cur->factor;
            if (cur->left) {
              cur->left->father = successor;
            }
            successor->left = cur->left;
            if (cur->right) {
              cur->right->father = successor;
            }
            successor->right = cur->right;
            Delete(cur);
            ++succPar->factor;
            RotateE(succPar);
          }
        }
      }
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     * The default method of check the equivalence is !(a < b || b > a)
     */
    size_t count(const Key &key) const {
      if (empty()) {
        return 0;
      }
      Node *cur = root_;
      while (cur) {
        if (compare(cur->value.first, key)) {
          cur = cur->right;
        } else if (compare(key, cur->value.first)) {
          cur = cur->left;
        } else {
          return 1;
        }
      }
      return 0;
    }

    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is returned.
     */
    iterator find(const Key &key) {
      if (empty()) {
        return end();
      }
      Node *cur = root_;
      while (cur) {
        if (compare(cur->value.first, key)) {
          cur = cur->right;
        } else if (compare(key, cur->value.first)) {
          cur = cur->left;
        } else {
          return iterator(cur, this);
        }
      }
      return end();
    }

    const_iterator find(const Key &key) const {
      if (empty()) {
        throw std::runtime_error("");
      }
      Node *cur = root_;
      while (cur) {
        if (compare(cur->value.first, key)) {
          cur = cur->right;
        } else if (compare(key, cur->value.first)) {
          cur = cur->left;
        } else {
          return const_iterator(cur, this);
        }
      }
      return cend();
    }
  };
}
