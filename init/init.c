#include <asm/asm.h>
#include <pmap.h>
#include <env.h>
#include <printf.h>
#include <trap.h>

void mips_init()
{
	printf("init.c:\tmips_init() is called\n");

	// Lab 2 memory management initialization functions
	mips_detect_memory();
	mips_vm_init();
	page_init();

	// physical_memory_manage_check();
    page_check();

	panic("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

	while (1);

	panic("init.c:\tend of mips_init() reached!");
}

void bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	max = dst + len;

	// copy machine words while possiblea 前面按int 4个字节4个字节拷贝
	while (dst + 3 < max) {
		*(int *)dst = *(int *)src;
		dst += 4;
		src += 4;
	}

	// finish remaining 0-3 bytes 最后几个byte单独处理
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}
}
/*将从指针开始的指定长度的空间清零*/
void bzero(void *b, size_t len)
{
	void *max;

	max = b + len;

	//printf("init.c:\tzero from %x to %x\n",(int)b,(int)max);

	// zero machine words while possible

	while (b + 3 < max) {
		*(int *)b = 0;
		b += 4;
	}

	// finish remaining 0-3 bytes
	while (b < max) {
		*(char *)b++ = 0;
	}

}
