#ifndef __GEHEAPARRAYLIST_H
#define __GEHEAPARRAYLIST_H

namespace GE
{

  /*------------------------------------------------------
  DynamicArrayList - dynamic size list that allocates
  each element dynamically, thus mixing features of
  linked and regular array list:
  - Capacity is preserved even after the list is cleared.
    As long as capacity is not exhausted all operations
    are assured not to yield any new dynamic allocations
  - Pointers to elements are still valid after insertion
    or removal (indexes however, are not)
  -------------------------------------------------------*/

  template <class T>
  class DynamicArrayList
  {
  private:

    T** elements;
    int _arraycap;
    int _elementcap;
    int _size;

    bool extendArray()
    {
      T** newElements = new T*[_arraycap*2];
      if (newElements == NULL) return false;

      for (int e=0; e<_elementcap; ++e)
        newElements[e] = elements[e];

      delete[] elements;
      elements = newElements;
      _arraycap *= 2;
      return true;
    }

    bool extendElements()
    {
      T* newT = new T;
      if (newT == NULL) return false;

      elements[_elementcap] = newT;
      _elementcap++;
      return true;
    }

  public:

    DynamicArrayList()
    {
      _arraycap = 1;
      _elementcap = 0;
      _size = 0;
      elements = new T*[_arraycap];
    }

    ~DynamicArrayList()
    {
      for (int e=0; e<_elementcap; ++e)
        delete elements[e];

      delete[] elements;
    }

    void clear() {
      _size = 0;
    }

    bool pushBack(const T& e)
    {
      if (_elementcap == _arraycap)
        if (!extendArray()) return false;

      if (_size == _elementcap)
        if (!extendElements()) return false;

      (*elements[_size]) = e;
      _size++;
      return true;
    }

    void popBack()
    {
      if (_size > 0)
        _size--;
    }

    T& operator[] (int index) const {
      return *elements[index];
    }

    int size() const {
      return _size;
    }

    bool empty() const {
      return (_size == 0);
    }

    int capacity() const {
      return _elementcap;
    }

    T& first() const {
      return *elements[0];
    }

    T& last() const {
      return *elements[_size-1];
    }
  };

}//namespace GE
#endif//__GEHEAPARRAYLIST_H
