/**
 * implement a container like std::map
 */
#pragma once
// only for std::less<T>
#include <cstddef>
#include <functional>

namespace sjtu {
template <class T1, class T2>
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

  template <class U1, class U2>
  pair(U1 &&x, U2 &&y) : first(x), second(y) {
  }

  template <class U1, class U2>
  pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {
  }

  template <class U1, class U2>
  pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {
  }
};

template <class Key, class T, class Compare = std::less<Key> >
class map {
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;

 private:
  enum Color { kRed, kBlack };
  struct node {
    value_type val_;
    node *left_son_{nullptr};
    node *right_son_{nullptr};
    node *father_{nullptr};
    Color color_{kRed};

    node() = delete;
    node(const value_type &val, node *father = nullptr)
        : val_(val), father_(father) {
    }
    node(const node &other) : val_(other.val_), color_(other.color_) {
    }
    node *LeftMost() {
      return left_son_ != nullptr ? left_son_->LeftMost() : this;
    }
    node *RightMost() {
      return right_son_ != nullptr ? right_son_->RightMost() : this;
    }
    bool IsLeftSon() const {
      return father_->left_son_ == this;
    }  // assure this != root_
    node *Brother() const {
      return IsLeftSon() ? father_->right_son_ : father_->left_son_;
    }  // assure this != root_
    bool BlackBrother() const {
      node *brother = Brother();
      if (brother == nullptr || brother->color_ == kBlack) {
        return true;
      }
      return false;
    }
  };

  size_t size_{0};
  node *root_{nullptr};
  node *first_{nullptr};
  node *last_{nullptr};

  void DFSCopy(node *cur, node *other) {
    if (other->left_son_ != nullptr) {
      cur->left_son_ = new node(*other->left_son_);
      cur->left_son_->father_ = cur;
      DFSCopy(cur->left_son_, other->left_son_);
    }
    if (other->right_son_ != nullptr) {
      cur->right_son_ = new node(*other->right_son_);
      cur->right_son_->father_ = cur;
      DFSCopy(cur->right_son_, other->right_son_);
    }
  }

  void DFSDeconstruct(node *cur) {
    if (cur->left_son_ != nullptr) {
      DFSDeconstruct(cur->left_son_);
    }
    if (cur->right_son_ != nullptr) {
      DFSDeconstruct(cur->right_son_);
    }
    delete cur;
  }

  void Rotate(node *cur) {
    node *grandfather = cur->father_->father_;
    if (grandfather != nullptr) {
      if (cur->father_->IsLeftSon()) {
        grandfather->left_son_ = cur;
      } else {
        grandfather->right_son_ = cur;
      }
    } else {
      root_ = cur;
    }
    if (cur->IsLeftSon()) {  // Right Rotate
      cur->father_->left_son_ = cur->right_son_;
      if (cur->right_son_ != nullptr) {
        cur->right_son_->father_ = cur->father_;
      }
      cur->right_son_ = cur->father_;
    } else {  // Left Rotate
      cur->father_->right_son_ = cur->left_son_;
      if (cur->left_son_ != nullptr) {
        cur->left_son_->father_ = cur->father_;
      }
      cur->left_son_ = cur->father_;
    }
    cur->father_->father_ = cur;
    cur->father_ = grandfather;
  }

  void InsertAdjust(node *cur) {
    if (cur == root_) {
      cur->color_ = kBlack;
      return;
    }
    if (cur->father_->color_ == kBlack) {
      return;
    }
    // cur->color == cur->father_->color == kRed and cur->father_ can't be root_
    if (!cur->father_->BlackBrother()) {
      cur->father_->color_ = kBlack;
      cur->father_->Brother()->color_ = kBlack;
      cur->father_->father_->color_ = kRed;
      InsertAdjust(cur->father_->father_);
    } else {
      if (cur->IsLeftSon() ^ cur->father_->IsLeftSon()) {  // LR or RL
        cur->color_ = kBlack;
        cur->father_->father_->color_ = kRed;
        Rotate(cur);
        Rotate(cur);
        if (root_->father_ != nullptr) {
          root_ = cur;
        }
      } else {
        cur->father_->color_ = kBlack;
        cur->father_->father_->color_ = kRed;
        Rotate(cur->father_);
        if (root_->father_ != nullptr) {
          root_ = cur->father_;
        }
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
    friend class map;
    friend class const_iterator;

   private:
    /**
     *  add data members
     *   just add whatever you want.
     */
    const map *map_ptr_;
    node *ptr_;

   public:
    iterator() : map_ptr_(nullptr), ptr_(nullptr) {
    }
    iterator(const map *map_ptr, node *ptr) : map_ptr_(map_ptr), ptr_(ptr) {
    }
    iterator(const iterator &other)
        : map_ptr_(other.map_ptr_), ptr_(other.ptr_) {
    }
    /**
     *  iter++
     */
    iterator operator++(int) {
      iterator tmp = *this;
      try {
        ++*this;
      } catch (...) {
        throw std::runtime_error("");
      }
      return tmp;
    }
    /**
     *  ++iter
     */
    iterator &operator++() {
      if (ptr_ == nullptr) {
        throw std::runtime_error("");
      }
      if (ptr_->right_son_ != nullptr) {
        ptr_ = ptr_->right_son_;
        while (ptr_->left_son_ != nullptr) {
          ptr_ = ptr_->left_son_;
        }
        return *this;
      }
      while (ptr_ != map_ptr_->root_) {
        if (ptr_->IsLeftSon()) {
          ptr_ = ptr_->father_;
          return *this;
        }
        ptr_ = ptr_->father_;
      }
      ptr_ = nullptr;
      return *this;
    }
    /**
     *  iter--
     */
    iterator operator--(int) {
      iterator tmp = *this;
      try {
        --*this;
      } catch (...) {
        throw std::runtime_error("");
      }
      return tmp;
    }
    /**
     *  --iter
     */
    iterator &operator--() {
      if (map_ptr_ == nullptr || ptr_ == map_ptr_->first_) {
        throw std::runtime_error("");
      }
      if (ptr_ == nullptr) {
        ptr_ = map_ptr_->last_;
        return *this;
      }
      if (ptr_->left_son_ != nullptr) {
        ptr_ = ptr_->left_son_;
        while (ptr_->right_son_ != nullptr) {
          ptr_ = ptr_->right_son_;
        }
        return *this;
      }
      while (ptr_ != map_ptr_->root_) {
        if (!ptr_->IsLeftSon()) {
          ptr_ = ptr_->father_;
          return *this;
        }
        ptr_ = ptr_->father_;
      }
      throw std::runtime_error("");
    }
    value_type &operator*() const {
      if (ptr_ == nullptr) {
        throw std::runtime_error("");
      }
      return ptr_->val_;
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    bool operator==(const iterator &rhs) const {
      return map_ptr_ == rhs.map_ptr_ && ptr_ == rhs.ptr_;
    }
    bool operator==(const const_iterator &rhs) const {
      return map_ptr_ == rhs.map_ptr_ && ptr_ == rhs.ptr_;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return map_ptr_ != rhs.map_ptr_ || ptr_ != rhs.ptr_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return map_ptr_ != rhs.map_ptr_ || ptr_ != rhs.ptr_;
    }

    /**
     * for the support of it->first.
     * See
     * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
     * for help.
     */
    value_type *operator->() const noexcept {
      return &ptr_->val_;
    }
  };
  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
    friend class map;
    friend class iterator;

   private:
    // data members.
    const map *map_ptr_;
    const node *ptr_;

   public:
    const_iterator() : map_ptr_(nullptr), ptr_(nullptr) {
    }
    const_iterator(const map *map_ptr, const node *ptr)
        : map_ptr_(map_ptr), ptr_(ptr) {
    }
    const_iterator(const const_iterator &other)
        : map_ptr_(other.map_ptr_), ptr_(other.ptr_) {
    }
    const_iterator(const iterator &other)
        : map_ptr_(other.map_ptr_), ptr_(other.ptr_) {
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      try {
        ++*this;
      } catch (...) {
        throw std::runtime_error("");
      }
      return tmp;
    }
    const_iterator &operator++() {
      if (ptr_ == nullptr) {
        throw std::runtime_error("");
      }
      if (ptr_->right_son_ != nullptr) {
        ptr_ = ptr_->right_son_;
        while (ptr_->left_son_ != nullptr) {
          ptr_ = ptr_->left_son_;
        }
        return *this;
      }
      while (ptr_ != map_ptr_->root_) {
        if (ptr_->IsLeftSon()) {
          ptr_ = ptr_->father_;
          return *this;
        }
        ptr_ = ptr_->father_;
      }
      ptr_ = nullptr;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      try {
        --*this;
      } catch (...) {
        throw std::runtime_error("");
      }
      return tmp;
    }
    const_iterator &operator--() {
      if (map_ptr_ == nullptr || map_ptr_->first_ == ptr_) {
        throw std::runtime_error("");
      }
      if (ptr_ == nullptr) {
        ptr_ = map_ptr_->last_;
        return *this;
      }
      if (ptr_->left_son_ != nullptr) {
        ptr_ = ptr_->left_son_;
        while (ptr_->right_son_ != nullptr) {
          ptr_ = ptr_->right_son_;
        }
        return *this;
      }
      while (ptr_ != map_ptr_->root_) {
        if (!ptr_->IsLeftSon()) {
          ptr_ = ptr_->father_;
          return *this;
        }
        ptr_ = ptr_->father_;
      }
      throw std::runtime_error("");
    }
    const value_type &operator*() const {
      if (ptr_ == nullptr) {
        throw std::runtime_error("");
      }
      return ptr_->val_;
    }

    bool operator==(const iterator &rhs) const {
      return map_ptr_ == rhs.map_ptr_ && ptr_ == rhs.ptr_;
    }
    bool operator==(const const_iterator &rhs) const {
      return map_ptr_ == rhs.map_ptr_ && ptr_ == rhs.ptr_;
    }
    bool operator!=(const iterator &rhs) const {
      return map_ptr_ != rhs.map_ptr_ || ptr_ != rhs.ptr_;
    }
    bool operator!=(const const_iterator &rhs) const {
      return map_ptr_ != rhs.map_ptr_ || ptr_ != rhs.ptr_;
    }
    const value_type *operator->() const noexcept {
      return &ptr_->val_;
    }
  };
  /**
   *  two constructors
   */
  map() {
  }
  map(const map &other) {
    size_ = other.size_;
    if (!other.empty()) {
      root_ = new node(*other.root_);
      DFSCopy(root_, other.root_);
      first_ = root_->LeftMost();
      last_ = root_->RightMost();
    }
  }
  /**
   *  assignment operator
   */
  map &operator=(const map &other) {
    if (this == &other) {
      return *this;
    }
    clear();
    size_ = other.size_;
    if (!other.empty()) {
      root_ = new node(*other.root_);
      DFSCopy(root_, other.root_);
      first_ = root_->LeftMost();
      last_ = root_->RightMost();
    }
    return *this;
  }
  /**
   *  Destructors
   */
  ~map() {
    clear();
  }
  /**
   * 
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent
   * to key. If no such element exists, an exception of type
   * `index_out_of_bound'
   */
  T &at(const Key &key) {
    auto ite = find(key);
    if (ite == end()) {
      throw std::runtime_error("");
    }
    return ite->second;
  }
  const T &at(const Key &key) const {
    auto ite = find(key);
    if (ite == cend()) {
      throw std::runtime_error("");
    }
    return ite->second;
  }
  /**
   * 
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    auto ite = find(key);
    if (ite == end()) {
      return insert(value_type(key, T())).first->second;
    }
    return ite->second;
  }
  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    auto ite = find(key);
    if (ite == cend()) {
      throw std::runtime_error("");
    }
    return ite->second;
  }
  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    return iterator(this, first_);
  }
  const_iterator cbegin() const {
    return const_iterator(this, first_);
  }
  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() {
    return iterator(this, nullptr);
  }
  const_iterator cend() const {
    return const_iterator(this, nullptr);
  }
  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const {
    return size_ == 0;
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
    if (root_ != nullptr) {
      DFSDeconstruct(root_);
      root_ = nullptr;
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
    }
  }
  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is true if insert successfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    if (empty()) {
      root_ = new node(value);
      root_->color_ = kBlack;
      first_ = root_, last_ = root_;
      size_ = 1;
      return pair<iterator, bool>(begin(), true);
    }

    node *cur = root_, *las = nullptr;
    bool from_left = true;
    while (cur != nullptr) {
      if (Compare()(value.first, cur->val_.first)) {
        las = cur;
        cur = cur->left_son_;
        from_left = true;
      } else if (Compare()(cur->val_.first, value.first)) {
        las = cur;
        cur = cur->right_son_;
        from_left = false;
      } else {
        return pair<iterator, bool>(iterator(this, cur), false);
      }
    }
    // new node becomes las's son
    cur = new node(value);
    cur->father_ = las;
    if (from_left) {
      las->left_son_ = cur;
      if (las == first_) {
        first_ = cur;
      }
    } else {
      las->right_son_ = cur;
      if (las == last_) {
        last_ = cur;
      }
    }

    InsertAdjust(cur);
    ++size_;
    return pair<iterator, bool>(iterator(this, cur), true);
  }
  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an
   * element out of this)
   */
  pair<iterator,bool>insert(const Key &key,const T& val) {
    return insert(value_type(key,val));
  }
  void erase(Key &key) {
    auto it = find(key);
    if (it != end()) {
      erase(it);
    }
  }
  void erase(iterator pos) {
    if (pos.map_ptr_ != this || pos == end()) {
      throw std::runtime_error("");
    }
    if (empty()) {
      return;
    }
    if (size_ == 1) {
      delete root_;
      root_ = nullptr;
      first_ = nullptr;
      last_ = nullptr;
      size_ = 0;
      return;
    }
    node *cur = root_;
    while (cur != nullptr) {
      if (Compare()(pos->first, cur->val_.first)) {
        cur = cur->left_son_;
      } else if (Compare()(cur->val_.first, pos->first)) {
        cur = cur->right_son_;
      } else {
        bool first_changed = !Compare()(first_->val_.first, cur->val_.first);
        bool last_changed = !Compare()(cur->val_.first, last_->val_.first);
        if (cur->left_son_ != nullptr && cur->right_son_ != nullptr) {
          // transform the case that remove a node with 2 sons to the case that
          // remove a node with 0/1 son
          node *tmp =
              cur->left_son_->RightMost();  // tmp is the substitute of cur
          if (tmp->right_son_ != nullptr) {
            tmp->right_son_->father_ = cur;
          }
          if (cur->right_son_ != nullptr) {
            cur->right_son_->father_ = tmp;
          }
          node *tmp_data = tmp->right_son_;
          tmp->right_son_ = cur->right_son_;
          cur->right_son_ = tmp_data;

          if (cur->father_ != nullptr) {
            if (cur->IsLeftSon()) {
              cur->father_->left_son_ = tmp;
            } else {
              cur->father_->right_son_ = tmp;
            }
          }
          if (tmp == cur->left_son_) {
            cur->left_son_ = tmp->left_son_;
            tmp->left_son_ = cur;
            tmp->father_ = cur->father_;
            cur->father_ = tmp;
          } else {
            tmp_data = tmp->left_son_;
            tmp->left_son_ = cur->left_son_;
            cur->left_son_->father_ = tmp;
            cur->left_son_ = tmp_data;

            tmp_data = tmp->father_;
            tmp->father_->right_son_ = cur;  // tmp must be right son
            tmp->father_ = cur->father_;
            cur->father_ = tmp_data;
          }

          if (root_ == cur) {
            root_ = tmp;
          }
          if (tmp->color_ != cur->color_) {
            cur->color_ = tmp->color_;
            tmp->color_ = (cur->color_ == kBlack ? kRed : kBlack);
          }
        }
        if (cur->left_son_ != nullptr && cur->right_son_ == nullptr) {
          // Remove a node with one son
          // cur->color_ must be kBlack
          cur->left_son_->father_ = cur->father_;
          if (cur->father_ == nullptr) {
            root_ = cur->left_son_;
          } else {
            if (cur->IsLeftSon()) {
              cur->father_->left_son_ = cur->left_son_;
            } else {
              cur->father_->right_son_ = cur->left_son_;
            }
          }
          cur->left_son_->color_ = kBlack;
        } else if (cur->left_son_ == nullptr && cur->right_son_ != nullptr) {
          cur->right_son_->father_ = cur->father_;
          if (cur->father_ == nullptr) {
            root_ = cur->right_son_;
          } else {
            if (cur->IsLeftSon()) {
              cur->father_->left_son_ = cur->right_son_;
            } else {
              cur->father_->right_son_ = cur->right_son_;
            }
          }
          cur->right_son_->color_ = kBlack;
        } else if (cur->color_ == kRed) {
          // Remove a red leaf node
          if (cur->IsLeftSon()) {
            cur->father_->left_son_ = nullptr;
          } else {
            cur->father_->right_son_ = nullptr;
          }
        } else {
          // Remove a black leaf node
          node *bad_node = cur;
          while (true) {
            node *brother = bad_node->Brother();
            if (!bad_node->BlackBrother()) {
              bad_node->father_->color_ = kRed;
              brother->color_ = kBlack;
              Rotate(brother);
              brother = bad_node->Brother();
            }
            // now bad_node's brother must be black
            bool black_left = (brother->left_son_ == nullptr ||
                               brother->left_son_->color_ == kBlack);
            bool black_right = (brother->right_son_ == nullptr ||
                                brother->right_son_->color_ == kBlack);
            if (black_left && black_right) {
              if (bad_node->father_->color_ == kRed) {
                bad_node->father_->color_ = kBlack;
                brother->color_ = kRed;
                break;
              } else {
                brother->color_ = kRed;
                bad_node = bad_node->father_;
                if (bad_node == root_) {
                  break;
                }
              }
            } else if (bad_node->IsLeftSon() && !black_left) {
              Rotate(brother->left_son_);
              Rotate(bad_node->Brother());
              bad_node->father_->father_->color_ = bad_node->father_->color_;
              bad_node->father_->color_ = kBlack;
              break;
            } else if (!bad_node->IsLeftSon() && !black_right) {
              Rotate(brother->right_son_);
              Rotate(bad_node->Brother());
              bad_node->father_->father_->color_ = bad_node->father_->color_;
              bad_node->father_->color_ = kBlack;
              break;
            } else {
              Rotate(brother);
              bad_node->father_->Brother()->color_ = bad_node->father_->color_;
              break;
            }
          }
          if (cur->IsLeftSon()) {
            cur->father_->left_son_ = nullptr;
          } else {
            cur->father_->right_son_ = nullptr;
          }
        }
        delete cur;
        if (first_changed) {
          first_ = root_->LeftMost();
        }
        if (last_changed) {
          last_ = root_->RightMost();
        }
        break;
      }
    }
    --size_;
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
    node *cur = root_;
    while (cur != nullptr) {
      if (Compare()(key, cur->val_.first)) {
        cur = cur->left_son_;
      } else if (Compare()(cur->val_.first, key)) {
        cur = cur->right_son_;
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
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    if (empty()) {
      return end();
    }
    node *cur = root_;
    while (cur != nullptr) {
      if (Compare()(key, cur->val_.first)) {
        cur = cur->left_son_;
      } else if (Compare()(cur->val_.first, key)) {
        cur = cur->right_son_;
      } else {
        return iterator(this, cur);
      }
    }
    return end();
  }
  const_iterator find(const Key &key) const {
    if (empty()) {
      return cend();
    }
    node *cur = root_;
    while (cur != nullptr) {
      if (Compare()(key, cur->val_.first)) {
        cur = cur->left_son_;
      } else if (Compare()(cur->val_.first, key)) {
        cur = cur->right_son_;
      } else {
        return const_iterator(this, cur);
      }
    }
    return cend();
  }
};
}  // namespace sjtu
