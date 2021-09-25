#include "types.h"
#include "sbi.h"


struct sbiret sbi_ecall(int ext, int fid, uint64 arg0,
			            uint64 arg1, uint64 arg2,
			            uint64 arg3, uint64 arg4,
			            uint64 arg5) 
{
    // unimplemented   
	long error, value;
	__asm__ volatile (
		"addi a7, x0, %[ext]\n"
		"addi a6, x0, %[fid]\n"
		"addi a5, x0, %[arg5]\n"
		"addi a4, x0, %[arg4]\n"
		"addi a3, x0, %[arg3]\n"
		"addi a2, x0, %[arg2]\n"
		"addi a1, x0, %[arg1]\n"
		"addi a0, x0, %[arg0]\n"
		"ecall\n"

		:[error] "=r" (error), [value] "=r" (value)
		:[ext] "r" (ext), [fid] "r" (fid), 
		[arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2), 
		[arg3] "r" (arg3), [arg4] "r" (arg4), [arg5] "r" (arg5)
		:"memory"
	);

	struct sbiret _ret;
	_ret.error = error;
	_ret.value = value;

	return _ret;

}
