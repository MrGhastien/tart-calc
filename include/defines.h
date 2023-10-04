#pragma once

typedef unsigned int u32;
typedef unsigned long int u64;
typedef unsigned short int u16;
typedef unsigned char u8;

typedef signed int i32;
typedef signed long int i64;
typedef signed short i16;
typedef signed char i8;

typedef unsigned char bool;

#define false 0
#define true 1
#define null 0

enum errcodes {
    ERRCODE_GENERAL = 1,
    ERRCODE_UNKNOWN_OPTION = 2,
    ERRCODE_IDX_OOB = 10,
};

void error(int code);
