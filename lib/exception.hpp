#ifndef _EXCEPTION_HPP_
#define _EXCEPTION_HPP_

#include <exception>
using namespace std;

class StopIteration: public exception {
  public:
    virtual const char* what() const throw() {
      return "StopIteration";
    }
};


#endif
