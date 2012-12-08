/*	\file   map.h
	\author Gregory Diamos <solusstultus@gmail.com>
	\date   November 14, 2012
	\brief  The header file for the map class
*/

#pragma once

// Archaeopteryx Includes
#include <archaeopteryx/util/interface/functional.h>
#include <archaeopteryx/util/interface/allocator_traits.h>
#include <archaeopteryx/util/interface/RedBlackTree.h>

namespace archaeopteryx
{

namespace util
{

template <class _Key, class _Tp, class _Compare, bool = is_empty<_Compare>::value>
class __map_value_compare
    : private _Compare
{
    typedef pair<typename remove_const<_Key>::type, _Tp> _Pp;
    typedef pair<const _Key, _Tp> _CP;
public:
    __device__ __map_value_compare()
        : _Compare() {}
    __device__ __map_value_compare(_Compare c)
        : _Compare(c) {}
    __device__ const _Compare& key_comp() const {return *this;}
    __device__ bool operator()(const _CP& __x, const _CP& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y.first);}
    __device__ bool operator()(const _CP& __x, const _Pp& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y.first);}
    __device__ bool operator()(const _CP& __x, const _Key& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y);}
    __device__ bool operator()(const _Pp& __x, const _CP& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y.first);}
    __device__ bool operator()(const _Pp& __x, const _Pp& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y.first);}
    __device__ bool operator()(const _Pp& __x, const _Key& __y) const
        {return static_cast<const _Compare&>(*this)(__x.first, __y);}
    __device__ bool operator()(const _Key& __x, const _CP& __y) const
        {return static_cast<const _Compare&>(*this)(__x, __y.first);}
    __device__ bool operator()(const _Key& __x, const _Pp& __y) const
        {return static_cast<const _Compare&>(*this)(__x, __y.first);}
    __device__ bool operator()(const _Key& __x, const _Key& __y) const
        {return static_cast<const _Compare&>(*this)(__x, __y);}
};

template <class _Key, class _Tp, class _Compare>
class __map_value_compare<_Key, _Tp, _Compare, false>
{
    _Compare comp;

    typedef pair<typename remove_const<_Key>::type, _Tp> _Pp;
    typedef pair<const _Key, _Tp> _CP;

public:
    __device__ __map_value_compare()
        : comp() {}
    __device__ __map_value_compare(_Compare c)
        : comp(c) {}
    __device__ const _Compare& key_comp() const {return comp;}

    __device__ bool operator()(const _CP& __x, const _CP& __y) const
        {return comp(__x.first, __y.first);}
    __device__ bool operator()(const _CP& __x, const _Pp& __y) const
        {return comp(__x.first, __y.first);}
    __device__ bool operator()(const _CP& __x, const _Key& __y) const
        {return comp(__x.first, __y);}
    __device__ bool operator()(const _Pp& __x, const _CP& __y) const
        {return comp(__x.first, __y.first);}
    __device__ bool operator()(const _Pp& __x, const _Pp& __y) const
        {return comp(__x.first, __y.first);}
    __device__ bool operator()(const _Pp& __x, const _Key& __y) const
        {return comp(__x.first, __y);}
    __device__ bool operator()(const _Key& __x, const _CP& __y) const
        {return comp(__x, __y.first);}
    __device__ bool operator()(const _Key& __x, const _Pp& __y) const
        {return comp(__x, __y.first);}
    __device__ bool operator()(const _Key& __x, const _Key& __y) const
        {return comp(__x, __y);}
};

template <class _Allocator>
class __map_node_destructor
{
    typedef _Allocator                          allocator_type;
    typedef allocator_traits<allocator_type>    __alloc_traits;
    typedef typename __alloc_traits::value_type::value_type value_type;
public:
    typedef typename __alloc_traits::pointer    pointer;
private:
    typedef typename value_type::first_type     first_type;
    typedef typename value_type::second_type    second_type;

    allocator_type& __na_;

    __device__ __map_node_destructor& operator=(const __map_node_destructor&);

public:
    bool __first_constructed;
    bool __second_constructed;

    explicit __device__ __map_node_destructor(allocator_type& __na)
        : __na_(__na),
          __first_constructed(false),
          __second_constructed(false)
        {}

    __device__ void operator()(pointer __p)
    {
        if (__second_constructed)
            __alloc_traits::destroy(__na_, _Vaddressof(__p->__value_.second));
        if (__first_constructed)
            __alloc_traits::destroy(__na_, _Vaddressof(__p->__value_.first));
        if (__p)
            __alloc_traits::deallocate(__na_, __p, 1);
    }
};

template <class _Key, class _Tp, class _Compare, class _Allocator>
    class map;
template <class _Key, class _Tp, class _Compare, class _Allocator>
    class multimap;
template <class _TreeIterator> class __map_const_iterator;

template <class _TreeIterator>
class __map_iterator
{
    _TreeIterator __i_;

    typedef typename _TreeIterator::__pointer_traits             __pointer_traits;
    typedef const typename _TreeIterator::value_type::first_type __key_type;
    typedef typename _TreeIterator::value_type::second_type      __mapped_type;
public:
    typedef bidirectional_iterator_tag                           iterator_category;
    typedef pair<__key_type, __mapped_type>                      value_type;
    typedef typename _TreeIterator::difference_type              difference_type;
    typedef value_type&                                          reference;
    typedef typename __pointer_traits::template
            rebind<value_type>::other                      pointer;

    __device__ __map_iterator() {}

    __device__ __map_iterator(_TreeIterator __i) : __i_(__i) {}

    __device__ reference operator*() const {return *operator->();}
    __device__ pointer operator->() const {return (pointer)__i_.operator->();}

    __device__ __map_iterator& operator++() {++__i_; return *this;}
    __device__ __map_iterator operator++(int)
    {
        __map_iterator __t(*this);
        ++(*this);
        return __t;
    }

    __device__ __map_iterator& operator--() {--__i_; return *this;}
    __device__ __map_iterator operator--(int)
    {
        __map_iterator __t(*this);
        --(*this);
        return __t;
    }

    friend __device__ bool operator==(const __map_iterator& __x, const __map_iterator& __y)
        {return __x.__i_ == __y.__i_;}
    friend 
    __device__ bool operator!=(const __map_iterator& __x, const __map_iterator& __y)
        {return __x.__i_ != __y.__i_;}

    template <class, class, class, class> friend class map;
    template <class, class, class, class> friend class multimap;
    template <class> friend class __map_const_iterator;
};

template <class _TreeIterator>
class __map_const_iterator
{
    _TreeIterator __i_;

    typedef typename _TreeIterator::__pointer_traits             __pointer_traits;
    typedef const typename _TreeIterator::value_type::first_type __key_type;
    typedef typename _TreeIterator::value_type::second_type      __mapped_type;
public:
    typedef bidirectional_iterator_tag                           iterator_category;
    typedef pair<__key_type, __mapped_type>                      value_type;
    typedef typename _TreeIterator::difference_type              difference_type;
    typedef const value_type&                                    reference;
    typedef typename __pointer_traits::template
            rebind<const value_type>::other                      pointer;

    __device__ __map_const_iterator() {}

    __device__ __map_const_iterator(_TreeIterator __i) : __i_(__i) {}
    __device__ __map_const_iterator(
            __map_iterator<typename _TreeIterator::__non_const_iterator> __i)
               
                : __i_(__i.__i_) {}

    __device__ reference operator*() const {return *operator->();}
    __device__ pointer operator->() const {return (pointer)__i_.operator->();}

    __device__ __map_const_iterator& operator++() {++__i_; return *this;}
    __device__ __map_const_iterator operator++(int)
    {
        __map_const_iterator __t(*this);
        ++(*this);
        return __t;
    }

    __device__ __map_const_iterator& operator--() {--__i_; return *this;}
    __device__ __map_const_iterator operator--(int)
    {
        __map_const_iterator __t(*this);
        --(*this);
        return __t;
    }

    friend __device__ bool operator==(const __map_const_iterator& __x, const __map_const_iterator& __y)
        {return __x.__i_ == __y.__i_;}
    friend __device__ bool operator!=(const __map_const_iterator& __x, const __map_const_iterator& __y)
        {return __x.__i_ != __y.__i_;}

    template <class, class, class, class> friend class map;
    template <class, class, class, class> friend class multimap;
    template <class, class, class> friend class __tree_const_iterator;
};


template <class Key, class T, class Compare = less<Key>,
          class Allocator = allocator<pair<const Key, T> > >
class map
{
public:
    // types:
    typedef Key                                      key_type;
    typedef T                                        mapped_type;
    typedef pair<const key_type, mapped_type>        value_type;
    typedef Compare                                  key_compare;
    typedef Allocator                                allocator_type;
    typedef value_type&                              reference;
    typedef const value_type&                        const_reference;

    class value_compare
        : public binary_function<value_type, value_type, bool>
    {
        friend class map;
    protected:
        key_compare comp;

        __device__ value_compare(key_compare c) : comp(c) {}
    public:
        __device__ bool operator()(const value_type& __x, const value_type& __y) const
            {return comp(__x.first, __y.first);}
    };

private:
    typedef pair<key_type, mapped_type>                             __value_type;
    typedef __map_value_compare<key_type, mapped_type, key_compare> __vc;
    typedef typename allocator_traits<allocator_type>::template
            rebind_alloc<__value_type>::other
                                                           __allocator_type;
    typedef RedBlackTree<__value_type, __vc, __allocator_type>   __base;
    typedef typename __base::__node_traits                 __node_traits;
    typedef allocator_traits<allocator_type>               __alloc_traits;


public:
    typedef typename __alloc_traits::pointer            pointer;
    typedef typename __alloc_traits::const_pointer      const_pointer;
    typedef typename __alloc_traits::size_type          size_type;
    typedef typename __alloc_traits::difference_type    difference_type;
    typedef __map_iterator<typename __base::iterator>   iterator;
    typedef __map_const_iterator<typename __base::const_iterator> const_iterator;
    typedef util::reverse_iterator<iterator>                  reverse_iterator;
    typedef util::reverse_iterator<const_iterator>            const_reverse_iterator;

public:
	explicit __device__ map(const key_compare& __comp = key_compare())
        : __tree_(__vc(__comp)) {}

    explicit __device__ map(const key_compare& __comp, const allocator_type& __a)
        : __tree_(__vc(__comp), __a) {}

    template <class _InputIterator>
        __device__ map(_InputIterator __f, _InputIterator __l,
            const key_compare& __comp = key_compare())
        : __tree_(__vc(__comp))
        {
            insert(__f, __l);
        }

    template <class _InputIterator>
        __device__ map(_InputIterator __f, _InputIterator __l,
            const key_compare& __comp, const allocator_type& __a)
        : __tree_(__vc(__comp), __a)
        {
            insert(__f, __l);
        }

    __device__ map(const map& __m)
        : __tree_(__m.__tree_)
        {
            insert(__m.begin(), __m.end());
        }

    __device__ map& operator=(const map& __m)
        {
            __tree_ = __m.__tree_;
            return *this;
        }

    __device__ explicit map(const allocator_type& __a)
        : __tree_(__a)
        {
        }

    __device__ map(const map& __m, const allocator_type& __a)
        : __tree_(__m.__tree_.value_comp(), __a)
        {
            insert(__m.begin(), __m.end());
        }

public:
	// Iteration
    __device__ iterator begin() {return __tree_.begin();}
    __device__  const_iterator begin() const {return __tree_.begin();}
    __device__ iterator end() {return __tree_.end();}
    __device__ const_iterator end() const {return __tree_.end();}

    __device__ reverse_iterator rbegin() {return reverse_iterator(end());}
    __device__ const_reverse_iterator rbegin() const
        {return const_reverse_iterator(end());}
    __device__ reverse_iterator rend()
            {return       reverse_iterator(begin());}
    __device__ const_reverse_iterator rend() const
        {return const_reverse_iterator(begin());}

public:
	// Size
    __device__ bool      empty() const {return __tree_.size() == 0;}
    __device__ size_type size() const {return __tree_.size();}
    __device__ size_type max_size() const {return __tree_.max_size();}

public:
	// Element Access
    __device__ mapped_type& operator[](const key_type& __k);

          mapped_type& at(const key_type& __k);
    __device__ const mapped_type& at(const key_type& __k) const;

    __device__ pair<iterator, bool>
        insert(const value_type& __v) {return __tree_.__insert_unique(__v);}

    __device__ iterator
        insert(const_iterator __p, const value_type& __v)
            {return __tree_.__insert_unique(__p.__i_, __v);}

    template <class _InputIterator>
            __device__ void insert(_InputIterator __f, _InputIterator __l)
        {
            for (const_iterator __e = end(); __f != __l; ++__f)
                insert(__e.__i_, *__f);
        }

    __device__ iterator erase(const_iterator __p) {return __tree_.erase(__p.__i_);}
    __device__ size_type erase(const key_type& __k)
        {return __tree_.__erase_unique(__k);}
    __device__ iterator  erase(const_iterator __f, const_iterator __l)
        {return __tree_.erase(__f.__i_, __l.__i_);}
    __device__ void clear() {__tree_.clear();}

    __device__ void swap(map& __m)
        {__tree_.swap(__m.__tree_);}

    __device__ iterator find(const key_type& __k)             {return __tree_.find(__k);}
    __device__ const_iterator find(const key_type& __k) const {return __tree_.find(__k);}
    __device__ size_type      count(const key_type& __k) const
        {return __tree_.__count_unique(__k);}
    __device__ iterator lower_bound(const key_type& __k)
        {return __tree_.lower_bound(__k);}
    __device__ const_iterator lower_bound(const key_type& __k) const
        {return __tree_.lower_bound(__k);}
    __device__ iterator upper_bound(const key_type& __k)
        {return __tree_.upper_bound(__k);}
    __device__ const_iterator upper_bound(const key_type& __k) const
        {return __tree_.upper_bound(__k);}
    __device__ pair<iterator,iterator> equal_range(const key_type& __k)
        {return __tree_.__equal_range_unique(__k);}
   __device__  pair<const_iterator,const_iterator> equal_range(const key_type& __k) const
        {return __tree_.__equal_range_unique(__k);}

public:
	// Member access
    __device__ allocator_type get_allocator() const {return __tree_.__alloc();}
    __device__ key_compare    key_comp()      const {return __tree_.value_comp().key_comp();}
    __device__ value_compare  value_comp()    const {return value_compare(__tree_.value_comp().key_comp());}


private:
	__base __tree_;

};

}

}



