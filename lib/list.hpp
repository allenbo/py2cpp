#ifndef _LIST_HPP_
#define _LIST_HPP_


#include <iostream>
#include <stdarg.h>
#include <vector>
#include <memory>
#include <functional>

#include "exception.hpp"
using namespace std;


#include "iter.hpp"

template<class ItemType>
class List {
  public:
    List(int num_item, ... ) {
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


    inline int __len__() const { return content.size(); }

    inline ItemType __getitem__(int index) const { return content[index]; }
    inline void __setitem__(int index, ItemType item) { content[index] = item; }
    
    class ListIter: public Iter<ItemType> {
      public:
        bool has_next() {
          return !(at == list->content.size());
        }

        ItemType next() {
          if (at != list->content.size()) 
            return list->content[at++];
          else{
            throw StopIteration();
          }
        }

        ListIter(List<ItemType> *  list)  {
          this->list = list;
          at = 0;
        }
      private:
        List<ItemType> * list;
        int at;
    };

    shared_ptr< ListIter > __iter__() {
      return make_shared< ListIter > (this);
    }

  private:
    vector<ItemType> content;
};



#endif
