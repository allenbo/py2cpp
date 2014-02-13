#ifndef _GENERATOR_HPP_
#define _GENERATOR_HPP_

#include <iostream>
#include <stdarg.h>
#include <vector>
#include <exception>
#include <memory>
#include <functional>

#include "exception.hpp"
using namespace std;


#define BEGIN_YIELD static int flag = 0; \
                    switch (flag) { \
                      case 0:

#define YIELD(x) flag = 1; \
                 return (x);\
              case 1: \
                 flag = 0;


#define END_YIELD }

template< class IterType, class BaseType, class BaseOfIterType> // IterType should be a smart pointer
class Generator: public Iter<BaseType> , public enable_shared_from_this< Generator<IterType, BaseType, BaseOfIterType> >{
  public:
    Generator(function< BaseType (BaseOfIterType)> func, IterType iter) {
      this->iter = iter;
      this->func = func;
      at = 0;
    }

    BaseType next() {
      if ( at != this->iter->__len__())
        return this->func(this->iter->__getitem__(at++));
      throw StopIteration();
    }

    shared_ptr< Generator<IterType, BaseType, BaseOfIterType> >__iter__() {
      return enable_shared_from_this<Generator<IterType, BaseType, BaseOfIterType> >::shared_from_this();
    }

  private:
    IterType iter;
    function<BaseType (BaseOfIterType)> func;
    int at;
};


template<class FuncType, class BaseType, class ArgType>
class GeneratorFromFunction : public Iter<BaseType> , public enable_shared_from_this< Generator<FuncType, BaseType, ArgType> >{
  public:
    GeneratorFromFunction(FuncType func, ArgType arg) {
      this->func = func;
      this->arg = arg;
      at = 0;
    }

    BaseType next() {
      return func(arg);
    }

    shared_ptr< Generator<FuncType, BaseType, ArgType> >__iter__() {
      return enable_shared_from_this<Generator<FuncType, BaseType, ArgType> >::shared_from_this();
    }

  private:
    FuncType func;
    ArgType arg;
    int at;
};


#endif
