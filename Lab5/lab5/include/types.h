#ifndef _TYPE_H
#define _TYPE_H

typedef unsigned long uint64;

struct pt_regs {
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
};

#endif
