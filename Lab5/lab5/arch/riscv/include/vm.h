int getvpn(unsigned long va, int idx);
int getppn(unsigned long pa, int idx);
extern void relocate(long OFFSET);
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
uint64 PA2VA(uint64 pa);
uint64 VA2PA(uint64 va);


extern char _stext[];
extern char _etext[];

extern char _srodata[];
extern char _erodata[];

extern char _sdata[];
extern char _edata[];

extern char _sbss[];
extern char _ebss[];

extern char _ekernel[];

extern char uapp_start[];
extern char uapp_end[];