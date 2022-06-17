#ifndef SINGLETON_H
#define SINGLETON_H
#include "noncopyable.h"

template <class T> class Singleton : noncopyable {
public:
    static T &Instance() {
        static T t;
        return t;
    }
};
#endif // SINGLETON_H
