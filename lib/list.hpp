#ifndef _LIST_HPP_
#define _LIST_HPP_


#include <iostream>
#include <stdarg.h>
#include <vector>
#include <exception>
#include <memory>
#include <functional>
using namespace std;

template<class ItemType>
class List {
  public:
    List(int num_item, ... ) {
      at = 0;
      if ( num_item == 0 ) {
        return ;
      }
      va_list ap;
      va_start(ap, num_item);
    
      int i = 0;
      for(; i < num_item; i ++ ) {
        ItemType item = va_arg(ap, ItemType);
        content.push_back(item);
      }
    }


    bool has_next() {
      return !(at == content.size());
    }

    ItemType next() {
      return content[at++];
    }

  private:
    vector<ItemType> content;
    int at;
};

#endif
