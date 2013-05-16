#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <functional>
using namespace std;

class pyobj;
class _py_str;
class _py_int;
template<class T> class _py_list;
class _py_lambda;

#define POBJ pyobj*
#define PINT _py_int*
#define PSTR _py_str*
#define PLAMBDA _py_lambda*

#define Int new _py_int
#define Str new _py_str
#define List new _py_list
#define Lambda new _py_lambda

#define DInt(x) dynamic_cast<_py_int*>(x)
#define DStr(x) dynamic_cast<_py_str*>(x)
#define DLambda(x) dynamic_cast<_py_lambda*>(x)

#define LAMBDA(x) Lambda(#x, (void*)x)

#define OBJ 0
#define OINT 1
#define OSTR 2
#define OLIST 3
#define OTUPLE 4
#define OLAMBDA 5


class pyobj {
    public:
        pyobj(int k): kind(k) {}
        int kind;
        virtual _py_str* __repr__() {}
        virtual pyobj* add(pyobj* a) {}
        virtual pyobj* sub(pyobj* a) {}
        virtual pyobj* mul(pyobj* a) {}
};

class pyiter {
    public:
        virtual bool has_next() = 0;
        virtual pyobj* next() = 0;
        virtual pyobj* get(_py_int * ind) = 0;
};

class _py_str: public pyobj {
    public:
        _py_str(char* cs): s(cs), pyobj(OSTR) {
        }
        _py_str(string s): pyobj(OSTR) {
            this->s = s;
        }
        _py_str* __repr__() {
            string s = "'" + this->s + "'";
            return new _py_str(s);
        }

        _py_str* add(pyobj* o ) {
            _py_str* oo = dynamic_cast<_py_str*>(o);
            string ret = this->s + oo->s;
            return new _py_str(ret);
        }

        _py_str* mul(pyobj* o);
        const char* str() { return s.c_str(); }

    private:
        string s;
};


class _py_int: public pyobj {
    public:
        _py_int(long long a): i(a), pyobj(OINT) { }
        _py_int* add(pyobj* a) {
            _py_int * aa = dynamic_cast< _py_int * >(a);
            long long ri = this->i + aa->i;
            return new _py_int(ri);
        }

        _py_int* sub(pyobj* a) {
            _py_int * aa = dynamic_cast< _py_int * >(a);
            long long ri = this->i - aa->i;
            return new _py_int(ri);
        }
        _py_int* mul(pyobj* a) {
            _py_int * aa = dynamic_cast< _py_int * >(a);
            long long ri = this->i * aa->i;
            return new _py_int(ri);
        }

        _py_str* __repr__() {
            char r[1024];
            sprintf(r, "%lld", i);
            return new _py_str(r);
        }
        long long getInt() { return i; }
    private:
        long long i;
};

template<class T>
class _py_list: public pyobj, public  pyiter{
    public:
        _py_list(int count, ...) : elements(0), pointer (0) , pyobj(OLIST){
            if(count == 0) return ;
            va_list ap;
            va_start(ap, count);
            for(int i = 0; i < count; i ++ )
                elements.push_back(va_arg(ap, T));
            va_end(ap);
        };
        _py_list(_py_list<T>* o): elements(0), pyobj(OLIST) {
            for(int i = 0; i < o->elements.size(); i ++ ) {
                this->elements.push_back(o->elements[i]);
            }
        }

        _py_list<T>* add(pyobj* a) {
            _py_list<T>* aa = dynamic_cast<_py_list<T>*>(a);
            _py_list<T>* ret = new _py_list<T>(this);
            for(int i  = 0; i < aa->elements.size(); i ++ ) {
                ret->elements.push_back(aa->elements[i]);
            }
            return ret;
        }

        _py_list<T>* mul(pyobj* a) {
            _py_int * aa = dynamic_cast<_py_int*> (a);
            long long cnt = aa->getInt();
            _py_list<T> * ret = new _py_list<T>(0);

            for(long long i = 0; i < cnt; i ++ ) {
                for(int j = 0; j < this->elements.size(); j ++ )
                    ret->elements.push_back(this->elements[j]);
            }
            return ret;
        }

        void* append(T o) {
            elements.push_back(o);
            return NULL;
        }
        void* extend(pyiter* o) {
            for(; o->has_next();){
                this->elements.push_back(dynamic_cast<T>(o->next()));
            }
            return NULL;
        }
        bool has_next() {
            if(pointer == elements.size()) { pointer = 0; return 0; }
            else return 1;
        }

        T next() {
            return elements[pointer++];
        }

        T get(_py_int* ind) {
            long long i = ind->getInt();
            return elements[i];
        }
        _py_str* __repr__() {
            pyobj* ret = new _py_str("[");
            for(int i = 0; i < elements.size(); i ++ ) {
                ret = ret->add(elements[i]->__repr__());
                if(i != elements.size() - 1)
                    ret = ret->add(new _py_str(", "));
            }
            ret = ret->add(new _py_str("]"));
            return dynamic_cast<_py_str*>(ret);
        }
    private:
        int pointer;
        std::vector<T> elements;
};


class _py_tuple: public pyobj, public pyiter{
    public:
        _py_tuple(int count, ...) : elements(0), pyobj(OTUPLE){
            va_list ap;
            va_start(ap, count);
            for(int i = 0; i < count; i ++ )
                elements.push_back(va_arg(ap, pyobj*));
            va_end(ap);
        };

        _py_str* __repr__() {
            pyobj* ret = new _py_str("(");
            for(int i = 0; i < elements.size(); i ++ ) {
                ret = ret->add(elements[i]->__repr__());
                if(i != elements.size() - 1)
                    ret = ret->add(new _py_str(", "));
            }
            ret = ret->add(new _py_str(")"));
            return dynamic_cast<_py_str*>(ret);
        }
    private:
        std::vector<pyobj*> elements;
};

class _py_lambda : public pyobj {
    public:
        _py_lambda(string s, void* f):name(s), func(f), pyobj(OLAMBDA) { }

        template<typename... types>
        POBJ operator()(types... args) {
            typedef POBJ(*FNT) (types...);
            function<POBJ(types...)> f = (FNT)func;
            return f(args...);
        }

        _py_str* __repr__() {
            string s = "<function " + name + ">";
            return new _py_str(s);
        }
    private:
        //function<Ret(types...)> func;
        void* func;
        string name;

};
_py_str* _py_str::mul(pyobj* o) {
    _py_int* oo = dynamic_cast<_py_int*>(o);
    long long time = oo->getInt();
    string ret;
    for(long long i = 0; i < time; i ++ ) {
        ret += this->s;
    }
    return new _py_str(ret);
}

void print(pyobj* dest, int nl, int cnt,  ...) {
    va_list ap;
    va_start(ap, cnt);
    for(int i = 0; i < cnt; i ++ ) {
        _py_str* p = (va_arg(ap, pyobj*))->__repr__();
        cout<< p->str();
        cout<< " ";
    }
    if(nl) {
        cout<< endl;
    }
}

pyobj* add( pyobj* x, pyobj* y) {
    return x->add(y);
}

_py_list<PINT>* range(PINT start, PINT stop = NULL, PINT step = NULL) {
    long long sa = 0;
    long long sp = 0;
    long long st = 1;
    sp = stop->getInt();
    if(NULL != start) sa = start->getInt();
    if(NULL != step) st = step->getInt();
    _py_list<PINT>* l = List<PINT>(0);
    for(long long i  = sa; i < sp ; i += st) {
        l->append(Int(i));
    }
    return l;
}


int main() {
    /*
    _py_list * x = new _py_list(1, new _py_int(1));
    _py_list * y = new _py_list(2, new _py_int(2), new _py_str("what"));
    y = y->mul(new _py_int(2));
    x->extend(y);
    print (NULL, 1, 2, x, y);
    */
    //_py_lambda<pyobj*, pyobj*, pyobj*> l("add", add);
    //_py_int * z = dynamic_cast<_py_int*>(l(new _py_int(1), new _py_int(2)));
    //_py_list * y = List(2, Int(2), Lambda<pyobj*, pyobj*, pyobj*>("add", add));
    //print(NULL, 1, 1, y);
    /*
    _py_str * x = new _py_str("what");
    x = x->mul(new _py_int(2));
    print(NULL, 1, 1, x);

    */
    /*
    _py_int * x = new _py_int(1);
    _py_int * y = new _py_int(2);
    _py_int * z = add(x, y);
    print(NULL, 1, 1, z);

    _py_list * a = new _py_list(2, new _py_str("what"), new _py_int(2));
    _py_list * b = new _py_list(1, new _py_int(3));
    _py_list * c = add(a, b);
    print (NULL, 1, 3, a, b, c);
    */
    /*
    _py_int * x = new _py_int(1);
    _py_int * y = x;
    x = new _py_int(3);
    print (NULL, 1, x);
    print (NULL, 1, y);
    */
    /*
    _py_list * _l_x = new _py_list(0);
    _py_list * _l_y = new _py_list(1, new _py_str("fuck"));
    _l_y->append(new _py_int(2));
    _l_x->append(_l_y);
    _l_x->append(new _py_int(4));
    _l_x->append(new _py_str("you"));
    print(NULL, 1, _l_x);
    */

    /* x = [x+x for in [1, 2]]
     * print x
     */

/*
    PLIST z = NULL;
    PLIST _listcomp_t0 = List(0);
    {
        PLIST _listcomp_iter0 = List(3, Int(1), Int(2), Int(3));
        for(; _listcomp_iter0->has_next(); ) {
            pyobj * p = _listcomp_iter0->next();
            _listcomp_t0->append(p->mul(p));
        }
    }
    z = _listcomp_t0;
    print(NULL, 1, 1, z);

    PINT y = DInt(z->get(Int(1)));
    y = Int(9);
    print(NULL, 1, 1, z);
    PLIST x = range(Int(5));
    print(NULL, 1, 1, x);
*/
    /*
    PLIST x = List(2, Lambda<PLIST, PINT, PINT, PINT>("range", range), Int(2));
    PLIST y = (*(dynamic_cast<_py_lambda<PLIST, PINT, PINT, PINT>*>(x->get(Int(0)))))(Int(0), Int(5), Int(1));
    print(NULL, 1, 1, y);
    y->append(x);
    print(NULL, 1, 1, y);
    */
    /*
    PLIST x = List(2, LAMBDA(add), LAMBDA(range));
    print(NULL, 1, 1, x);
    PLIST y = DList(DEF(x->get(Int(1)), (NULL, Int(6), NULL)));
    print(NULL, 1, 1, y);
    */
    _py_list<PINT>* x = List<PINT>(2, Int(1), Int(2));
    print(NULL, 1, 1, x);
    return 0;
}
