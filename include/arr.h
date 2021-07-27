#ifndef ARR_H
#define ARR_H

#include <cstddef>
#include <cstring>
#include <stdlib.h>

template <class T>
class Array {
public:
    inline void reserve(size_t element_count){
        if (!arr) {
            count = element_count;
            arr = new T[element_count];
            return;
        }
        if (element_count > count) {
            T *temp_array = arr;
            count = element_count;
            memcpy(arr, temp_array, sizeof(T) * element_count);
            delete temp_array;
        }
        count = element_count;
    }

    inline const size_t size() {
        return count;
    }

    inline const size_t top() {
        return reserved;
    }

    inline void push(const T& element) {
        if (reserved + 1 >= count) 
        reserve(count + 1);
        
        arr[reserved++] = element;
    }

    inline const bool is_empty() {
        return (count == 0);
    }

    inline const T& get(size_t index){
        return arr[index];
    }

    inline T* get_arr() {
        return arr;
    }

    inline void clear() {
        reserved = 0;
        memset(arr, 0, count);
    }
private:
    T* arr = nullptr;
    size_t count = 0;
    size_t reserved = 0;
};

using String = Array<char>;

#endif //!ARR_H