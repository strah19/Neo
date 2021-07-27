#include "../include/sym.h"

int SymTable::get_index(const char* name) {
    for(int i = 0; i < table.top(); i++) {
        if (name[0] == table.get(i).name[0]) {
            if (strcmp(name, table.get(i).name) == 0) {
                return i;
            }
        }
    }

    return -1;
}

Entry* SymTable::insert(const char* name, int type) {
    Entry* e = look_up_type(name, type);
    if (e == nullptr) {
        table.push({name, type});
        return &table.get_arr()[table.top() - 1];
    }
    return e;
}

Entry* SymTable::look_up(const char* name) {
    for(int i = 0; i < table.top(); i++) {
        if (name[0] == table.get(i).name[0]) {
            if (strcmp(name, table.get(i).name) == 0) {
                return &table.get_arr()[i];
            }
        }
    }
    return nullptr;
}

Entry* SymTable::look_up_type(const char* name, int type) {
    for(int i = 0; i < table.top(); i++) {
        if (name[0] == table.get(i).name[0] && table.get(i).type == type) {
            if (strcmp(name, table.get(i).name) == 0) {
                return &table.get_arr()[i];
            }
        }
    }
    return nullptr;
}

Entry* SymTable::look_up_by_type(int type) {
    for(int i = 0; i < table.top(); i++) {
        if (table.get(i).type == type) {
             return &table.get_arr()[i];
        }
    }
    return nullptr;
}

SymTable::SymTable() {
    table.reserve(256);
}