#ifndef VALUE_H
#define VALUE_H

enum ValueType {
    TYPE_NUM,
    /*TYPE_CPX,
    TYPE_FUNC,
    TYPE_SET,*/
    TYPE_NONE
};

typedef struct value_t {
    enum ValueType type;
    union data_t {
        double num;
    } data;
} Value;

#endif /* ! VALUE_H */
