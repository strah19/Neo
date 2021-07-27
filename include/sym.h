#ifndef SYM_H
#define SYM_H

#include "arr.h"

struct Entry {
    const char* name;
    int type;
};

struct SymTable {
    Array<Entry> table;

    int get_index(const char* name);
    Entry* insert(const char* name, int type);
    Entry* look_up(const char* name);
    Entry* look_up_type(const char* name, int type);
    Entry* look_up_by_type(int type);

    SymTable();
};

#endif //!SYM_H