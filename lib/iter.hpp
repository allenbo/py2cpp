#ifndef _ITER_HPP_
#define _ITER_HPP_

template<class BaseType>
class Iter{
  public:
    virtual BaseType next() = 0;
};

#endif

