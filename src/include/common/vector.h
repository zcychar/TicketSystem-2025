#pragma once

#include <climits>
#include <cstddef>

namespace sjtu {
  /**
   * a data container like std::vector
   * store data in a successive memory and support random access.
   */
  template<typename T>
  class vector {
    T *arr_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void double_capacity() {
      T *tmp = (T *) malloc(2 * capacity_ * sizeof(T));
      for (int i = 0; i < size_; ++i) {
        new(tmp + i) T(std::move(arr_[i]));
      }
      capacity_ *= 2;
      if (arr_ != nullptr) {
        for (int i = 0; i < size_; ++i) {
          arr_[i].~T();
        }
        free(arr_);
        arr_ = nullptr;
      }
      arr_ = tmp;
    }

  public:
    /**
     *
     * a type for actions of the elements of a vector, and you should write
     *   a class named const_iterator with same interfaces.
     */
    /**
     * you can see RandomAccessIterator at CppReference for help.
     */
    class const_iterator;

    class iterator {
      // The following code is written for the C++ type_traits library.
      // Type traits is a C++ feature for describing certain properties of a type.
      // For instance, for an iterator, iterator::value_type is the type that the
      // iterator points to.
      // STL algorithms and containers may use these type_traits (e.g. the following
      // typedef) to work properly. In particular, without the following code,
      // @code{std::sort(iter, iter1);} would not compile.
      // See these websites for more information:
      // https://en.cppreference.com/w/cpp/header/type_traits
      // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
      // About iterator_category: https://en.cppreference.com/w/cpp/iterator
    public:
      using difference_type = std::ptrdiff_t;
      using value_type = T;
      using pointer = T *;
      using reference = T &;
      using iterator_category = std::output_iterator_tag;

    private:
      T *location_ = nullptr;
      vector *container_ = nullptr;

    public:
      iterator() = default;

      iterator(T *location, vector *container): location_(location), container_(container) {
      }

      /**
       * return a new iterator which pointer n-next elements
       * as well as operator-
       */
      iterator operator+(const int &n) const {
        return iterator(location_ + n, container_);
      }

      iterator operator-(const int &n) const {
        return operator+(-n);
      }

      // return the distance between two iterators,
      // if these two iterators point to different vectors, throw invaild_iterator.
      int operator-(const iterator &rhs) const {
        if (container_ != rhs.container_) {
          throw std::runtime_error("invalid_iterator");
        }
        return static_cast<int>(location_ - rhs.location_);
      }

      iterator &operator+=(const int &n) {
        location_ += n;
        return *this;
      }

      iterator &operator-=(const int &n) {
        return operator-=(-n);
      }

      /**
       * iter++
       */
      iterator operator++(int) {
        iterator tmp = *this;
        location_++;
        return tmp;
      }

      /**
       * ++iter
       */
      iterator &operator++() {
        location_++;
        return *this;
      }

      /**
       * iter--
       */
      iterator operator--(int) {
        iterator tmp = *this;
        location_--;
        return tmp;
      }

      /**
       * --iter
       */
      iterator &operator--() {
        location_--;
        return *this;
      }

      /**
       * *it
       */
      T &operator*() const {
        return *location_;
      }

      /**
       * a operator to check whether two iterators are same (pointing to the same memory address).
       */
      bool operator==(const iterator &rhs) const {
        if (location_ == rhs.location_) {
          return true;
        }
        return false;
      }

      bool operator==(const const_iterator &rhs) const {
        if (location_ == rhs.location_) {
          return true;
        }
        return false;
      }

      /**
       * some other operator for iterator.
       */
      bool operator!=(const iterator &rhs) const {
        return !(operator==(rhs));
      }

      bool operator!=(const const_iterator &rhs) const {
        return operator==(rhs) ^ 1;
      }
    };

    /**
     *
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
      T *location_ = nullptr;
      const vector *container_ = nullptr;

    public:
      const_iterator() = default;

      const_iterator(T *location, const vector *container): location_(location), container_(container) {
      }

      /**
       * return a new iterator which pointer n-next elements
       * as well as operator-
       */
      const_iterator operator+(const int &n) const {
        return const_iterator(location_ + n, container_);
      }

      const_iterator operator-(const int &n) const {
        return operator+(-n);
      }

      // return the distance between two iterators,
      // if these two iterators point to different vectors, throw invaild_iterator.
      int operator-(const const_iterator &rhs) const {
        if (container_ != rhs.container_) {
          throw std::runtime_error("invalid_iterator");
        }
        return static_cast<int>(location_ - rhs.location_);
      }

      const_iterator &operator+=(const int &n) {
        location_ += n;
        return *this;
      }

      const_iterator &operator-=(const int &n) {
        return operator-=(-n);
      }

      /**
       * iter++
       */
      const_iterator operator++(int) {
        const_iterator tmp = *this;
        location_++;
        return tmp;
      }

      /**
       * ++iter
       */
      const_iterator &operator++() {
        location_++;
        return *this;
      }

      /**
       * iter--
       */
      const_iterator operator--(int) {
        const_iterator tmp = *this;
        location_--;
        return tmp;
      }

      /**
       * --iter
       */
      const_iterator &operator--() {
        location_--;
        return *this;
      }

      /**
       * *it
       */
      T &operator*() const {
        return *location_;
      }

      /**
       * a operator to check whether two iterators are same (pointing to the same memory address).
       */
      bool operator==(const iterator &rhs) const {
        if (location_ == rhs.location_) {
          return true;
        }
        return false;
      }

      bool operator==(const const_iterator &rhs) const {
        if (location_ == rhs.location_) {
          return true;
        }
        return false;
      }

      /**
       * some other operator for iterator.
       */
      bool operator!=(const iterator &rhs) const {
        return operator==(rhs) ^ 1;
      }

      bool operator!=(const const_iterator &rhs) const {
        return operator==(rhs) ^ 1;
      }
    };

    /**
     * Constructs
     * At least two: default constructor, copy constructor
     */
    vector() = default;

    vector(const vector &other) {
      if (arr_ != nullptr) {
        for (int i = 0; i < size_; ++i) {
          arr_[i].~T();
        }
        free(arr_);
        arr_ = nullptr;
      }
      size_ = other.size_;
      capacity_ = other.capacity_;
      arr_ = (T *) malloc(capacity_ * sizeof(T));
      for (int i = 0; i < size_; ++i) {
        new(arr_ + i)T(other.arr_[i]);
      }
    }

    vector(vector &&other) noexcept
      : arr_(other.arr_), size_(other.size_), capacity_(other.capacity_) {
      other.arr_ = nullptr;
      other.size_ = other.capacity_ = 0;
    }

    explicit vector(size_t size, const T &value) : size_(size), capacity_(size) {
      if (size > 0) {
        arr_ = static_cast<T *>(malloc(size * sizeof(T)));
        if (!arr_) throw std::bad_alloc();
        size_t i = 0;
        try {
          for (; i < size; ++i) {
            new(arr_ + i) T(value); // 拷贝构造元素
          }
        } catch (...) {
          for (size_t j = 0; j < i; ++j) arr_[j].~T();
          free(arr_);
          throw;
        }
      }
    }

    // 移动赋值运算符
    vector &operator=(vector &&other) noexcept {
      if (this != &other) {
        if (arr_) {
          for (size_t i = 0; i < size_; ++i) arr_[i].~T();
          free(arr_);
        }
        arr_ = other.arr_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.arr_ = nullptr;
        other.size_ = other.capacity_ = 0;
      }
      return *this;
    }

    /**
     * Destructor
     */
    ~vector() {
      if (arr_ != nullptr) {
        for (int i = 0; i < size_; ++i) {
          arr_[i].~T();
        }
        free(arr_);
        arr_ = nullptr;
      }
    }

    /**
     * Assignment operator
     */
    vector &operator=(const vector &other) {
      if (this == &other) {
        return *this;
      }
      if (arr_ != nullptr) {
        for (int i = 0; i < size_; ++i) {
          arr_[i].~T();
        }
        free(arr_);
        arr_ = nullptr;
      }
      size_ = other.size_;
      capacity_ = other.capacity_;
      arr_ = (T *) malloc(capacity_ * sizeof(T));
      for (int i = 0; i < size_; ++i) {
        new(arr_ + i)T(other.arr_[i]);
      }
      return *this;
    }

    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     */
    T &at(const size_t &pos) {
      if (pos < 0 || pos >= size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      return arr_[pos];
    }

    const T &at(const size_t &pos) const {
      if (pos < 0 || pos >= size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      return arr_[pos];
    }

    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     * !!! Pay attentions
     *   In STL this operator does not check the boundary but I want you to do.
     */
    T &operator[](const size_t &pos) {
      if (pos < 0 || pos >= size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      return arr_[pos];
    }

    const T &operator[](const size_t &pos) const {
      if (pos < 0 || pos >= size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      return arr_[pos];
    }

    /**
     * access the first element.
     * throw container_is_empty if size == 0
     */
    const T &front() const {
      if (size_ == 0) {
        throw std::runtime_error("container_is_empty");
      }
      return arr_[0];
    }

    /**
     * access the last element.
     * throw container_is_empty if size == 0
     */
    const T &back() const {
      if (size_ == 0) {
        throw std::runtime_error("container_is_empty");
      }
      return arr_[size_ - 1];
    }

    /**
     * returns an iterator to the beginning.
     */
    iterator begin() {
      return iterator(&arr_[0], this);
    }

    const_iterator begin() const {
      return const_iterator(&arr_[0], this);
    }

    const_iterator cbegin() const {
      return const_iterator(&arr_[0], this);
    }

    /**
     * returns an iterator to the end.
     */
    iterator end() {
      return iterator(&arr_[size_], this);
    }

    const_iterator end() const {
      return const_iterator(&arr_[size_], this);
    }

    const_iterator cend() const {
      return const_iterator(&arr_[size_], this);
    }

    /**
     * checks whether the container is empty
     */
    bool empty() const {
      if (size_ == 0) {
        return true;
      }
      return false;
    }

    /**
     * returns the number of elements
     */
    size_t size() const {
      return size_;
    }

    /**
     * clears the contents
     */
    void clear() {
      if (arr_ != nullptr) {
        for (int i = 0; i < size_; ++i) {
          arr_[i].~T();
        }
        free(arr_);
        arr_ = nullptr;
      }
      size_ = 0;
      capacity_ = 0;
    }

    /**
     * inserts value before pos
     * returns an iterator pointing to the inserted value.
     */
    iterator insert(iterator pos, const T &value) {
      int length = pos - begin();
      if (size_ == capacity_) {
        double_capacity();
      }
      for (int i = size_ - 1; i >= length; --i) {
        new(arr_ + i + 1)T(std::move(arr_[i]));
      }
      arr_[length].~T();
      new(arr_ + length)T(value);
      size_++;
      return iterator(&arr_[length], this);
    }

    /**
     * inserts value at index ind.
     * after inserting, this->at(ind) == value
     * returns an iterator pointing to the inserted value.
     * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
     */
    iterator insert(const size_t &ind, const T &value) {
      if (ind > size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      if (size_ == capacity_) {
        double_capacity();
      }
      for (int i = size_ - 1; i >= ind; --i) {
        new(arr_ + i + 1)T(std::move(arr_[i]));
      }
      arr_[ind].~T();
      new(arr_ + ind)T(value);
      size_++;
      return iterator(&arr_[ind], this);
    }

    /**
     * removes the element at pos.
     * return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     */
    iterator erase(iterator pos) {
      int length = pos - begin();
      arr_[length].~T();
      for (int i = length; i < size_ - 1; ++i) {
        new(arr_ + i)T(std::move(arr_[i + 1]));
      }
      size_--;
      return iterator(&arr_[length], this);
    }

    /**
     * removes the element with index ind.
     * return an iterator pointing to the following element.
     * throw index_out_of_bound if ind >= size
     */
    iterator erase(const size_t &ind) {
      if (ind >= size_) {
        throw std::runtime_error("index_out_of_bound");
      }
      arr_[ind].~T();
      for (int i = ind; i < size_ - 1; ++i) {
        new(arr_ + i)T(std::move(arr_[i + 1]));
      }
      size_--;
      return iterator(&arr_[ind], this);
    }

    /**
     * adds an element to the end.
     */
    void push_back(const T &value) {
      if (capacity_ == 0) {
        capacity_ = 1;
        size_ = 1;
        arr_ = (T *) malloc(capacity_ * sizeof(T));
        new(arr_)T(value);
        return;
      }
      if (size_ == capacity_) {
        double_capacity();
      }
      new(arr_ + size_)T(value);
      size_++;
    }

    void push_back(T &&value) {
      if (capacity_ == 0) {
        capacity_ = 1;
        size_ = 1;
        arr_ = (T *) malloc(capacity_ * sizeof(T));
        new(arr_)T(std::move(value));
        return;
      }
      if (size_ == capacity_) double_capacity();
      new(arr_ + size_) T(std::move(value)); // 移动构造
      size_++;
    }

    /**
     * remove the last element from the end.
     * throw container_is_empty if size() == 0
     */
    void pop_back() {
      if (size_ == 0) {
        throw std::runtime_error("container_is_empty");
      }
      arr_[size_ - 1].~T();
      size_--;
    }

    T *data() {
      return arr_;
    }

    const T *data() const {
      return arr_;
    }

    T &back() {
      return arr_[size_ - 1];
    }

    void reserve(size_t n) {
      if (n > capacity_) {
        T *new_arr = static_cast<T *>(malloc(n * sizeof(T)));
        if (!new_arr) throw std::bad_alloc();
        size_t i = 0;
        try {
          for (; i < size_; ++i) {
            new(new_arr + i) T(std::move(arr_[i])); // 移动构造元素
          }
        } catch (...) {
          for (size_t j = 0; j < i; ++j) new_arr[j].~T();
          free(new_arr);
          throw;
        }
        // 销毁并释放旧内存
        if (arr_) {
          for (size_t j = 0; j < size_; ++j) arr_[j].~T();
          free(arr_);
        }
        arr_ = new_arr;
        capacity_ = n;
      }
    }
  };
}
