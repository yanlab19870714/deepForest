#ifndef DEQUE_P_H
#define DEQUE_P_H

#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>

using namespace std;

template <class T>
class deque_p {
public:
    mutex lock;
    deque<T*> queue;

    deque_p() {}

    deque_p(const deque_p<T> & q) {
        //required due to RespQueue's initialization calls resize(.)
        //the trouble is caused by mutex-elements, which cannot be copied
        //the function is not actually called, we need to define it just because resize(.) may copy
        cout << "conque's copy constructor called, should never happen !!!" << endl;
        exit(-1);
    }

    void push_back(T* val) {
        unique_lock<mutex> lck(lock);
        queue.push_back(val);
    }

    void push_front(T* val) {
        unique_lock<mutex> lck(lock);
        queue.push_front(val);
    }

    /*T* pop_back() {
        unique_lock<mutex> lck(lock);

        if(queue.empty()) {
            return NULL;
        }

        T* value = queue.back();
        queue.pop_back();

        return value;
    }*/

    T* pop_front() {
        unique_lock<mutex> lck(lock);

        if(queue.empty()) {
            return NULL;
        }

        T* value = queue.front();
        queue.pop_front();

        return value;
    }

    bool empty() {
        unique_lock<mutex> lck(lock);
        return queue.empty();
    }
};

#endif
