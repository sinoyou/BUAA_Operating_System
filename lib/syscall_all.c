#include "../drivers/gxconsole/dev_cons.h"
#include <mmu.h>
#include <env.h>
#include <printf.h>
#include <pmap.h>
#include <sched.h>

extern char *KERNEL_SP;
extern struct Env *curenv;
extern int debug_mode;

/* Overview:
 * 	This function is used to print a character on screen.
 * 
 * Pre-Condition:
 * 	`c` is the character you want to print.
 */
void sys_putchar(int sysno, int c, int a2, int a3, int a4, int a5)
{
	printcharc((char) c);
	return ;
}

/* Overview:
 * 	This function enables you to copy content of `srcaddr` to `destaddr`.
 *
 * Pre-Condition:
 * 	`destaddr` and `srcaddr` can't be NULL. Also, the `srcaddr` area 
 * 	shouldn't overlap the `destaddr`, otherwise the behavior of this 
 * 	function is undefined.
 *
 * Post-Condition:
 * 	the content of `destaddr` area(from `destaddr` to `destaddr`+`len`) will
 * be same as that of `srcaddr` area.
 */
void *memcpy(void *destaddr, void const *srcaddr, u_int len)
{
	char *dest = destaddr;
	char const *src = srcaddr;

	while (len-- > 0) {
		*dest++ = *src++;
	}

	return destaddr;
}

/* Overview:
 *	This function provides the environment id of current process.
 *
 * Post-Condition:
 * 	return the current environment id
 */
u_int sys_getenvid(void)
{
	return curenv->env_id;
}

/* Overview:
 *	This function enables the current process to give up CPU.
 *
 * Post-Condition:
 * 	Deschedule current environment. This function will never return.
 */
void sys_yield(void)
{
	bcopy((void*)(KERNEL_SP - sizeof(struct Trapframe)),
			(void*)(TIMESTACK - sizeof(struct Trapframe)),
			sizeof(struct Trapframe));
	sched_yield();
}

/* Overview:
 * 	This function is used to destroy the current environment.
 *
 * Pre-Condition:
 * 	The parameter `envid` must be the environment id of a 
 * process, which is either a child of the caller of this function 
 * or the caller itself.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 when error occurs.
 */
int sys_env_destroy(int sysno, u_int envid)
{
	/*
		printf("[%08x] exiting gracefully\n", curenv->env_id);
		env_destroy(curenv);
	*/
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0) {
		return r;
	}

	printf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

/* Overview:
 * 	Set envid's pagefault handler entry point and exception stack.
 * 
 * Pre-Condition:
 * 	xstacktop points one byte past exception stack.
 *
 * Post-Condition:
 * 	The envid's pagefault handler will be set to `func` and its
 * 	exception stack will be set to `xstacktop`.
 * 	Returns 0 on success, < 0 on error.
 */
int sys_set_pgfault_handler(int sysno, u_int envid, u_int func, u_int xstacktop)
{
	// Your code here.
	struct Env *env;
	int ret;

	ret = envid2env(envid, &env, 0);
	if(ret < 0) {
		if(debug_mode) printf("[DEBUG] sys_set_pgfault_handler: Wrong at envid2env\n");
		return ret;
	}

	env->env_pgfault_handler = func;
	env->env_xstacktop = xstacktop;

	return 0;
	//	panic("sys_set_pgfault_handler not implemented");
}

/* Overview:
 * 	Allocate a page of memory and map it at 'va' with permission
 * 'perm' in the address space of 'envid'.
 *
 * 	If a page is already mapped at 'va', that page is unmapped as a
 * side-effect.
 * 
 * Pre-Condition:
 * perm -- PTE_V is required,
 *         PTE_COW is not allowed(return -E_INVAL),
 *         other bits are optional.
 *
 * Post-Condition:
 * Return 0 on success, < 0 on error
 *	- va must be < UTOP
 *	- env may modify its own address space or the address space of its children
 */
int sys_mem_alloc(int sysno, u_int envid, u_int va, u_int perm)
{
	// Your code here.
	struct Env *env;
	struct Page *ppage;
	int ret;
	ret = 0;
	if(va >= UTOP || va < 0){
		if(debug_mode)	panic("[DEBUG] sys_mem_alloc: va should < UTOP\n");
		return -E_INVAL;
	}
	if((perm & PTE_V) == 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_alloc: PTE_E is required\n");
		return -E_INVAL;
	}
	if((perm & PTE_COW) != 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_alloc: PTE_COW cannot permit\n");
		return -E_INVAL;
	}
	ret = envid2env(envid, &env, 1);		// how to use checkperm?
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_alloc: envid2env has wrong here\n");
		return ret;
	}
	Pde * pgdir = env->env_pgdir;
	ret = page_alloc(&ppage);
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_alloc: page_alloc has wrong here\n");
		return ret;
	}
	//ppage -> pp_ref ++;
	ret = page_insert(pgdir, ppage, va, perm);
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_alloc: page_insert has wrong here\n");
		return ret;
	}
	return 0;
}

/* Overview:
 * 	Map the page of memory at 'srcva' in srcid's address space
 * at 'dstva' in dstid's address space with permission 'perm'.
 * Perm has the same restrictions as in sys_mem_alloc.
 * (Probably we should add a restriction that you can't go from
 * non-writable to writable?)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Note:
 * 	Cannot access pages above UTOP.
 */
int sys_mem_map(int sysno, u_int srcid, u_int srcva, u_int dstid, u_int dstva,
				u_int perm)
{
	int ret;
	u_int round_srcva, round_dstva;
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *ppage;
	Pte *src_ppte;
	Pte *dst_ppte;

	ppage = NULL;
	ret = 0;
	round_srcva = ROUNDDOWN(srcva, BY2PG);
	round_dstva = ROUNDDOWN(dstva, BY2PG);
	
	if((perm & PTE_V) == 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_map: perm has wrong here\n");
		return -E_INVAL;
	}

	if(dstva >= UTOP || srcva >= UTOP) {
		if(debug_mode) panic("[DEBUG] sys_mem_map: va >= UTOP\n");
		return -E_INVAL;
	}

    //your code here
	ret = envid2env(srcid, &srcenv, 0);    // if we need the checkperm set here?
	if(ret<0) {
		if(debug_mode) panic("[DEBUG] sys_mem_map: envid2env-srcid wrong here\n");
		return -E_BAD_ENV;
	}
	ret = envid2env(dstid, &dstenv, 0);
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_map: envid2env-dstid wrong here\n");
		return -E_BAD_ENV;
	}
	
	ppage = page_lookup(srcenv->env_pgdir, round_srcva, &src_ppte);
	if(src_ppte!=NULL){
		if((*src_ppte & PTE_R)==0 && (perm & PTE_R)!=0) {
			if(debug_mode) panic("[DEBUG] sys_mem_map: try to from PTE_R==0 TO PTE_R!=0\n");
			return -E_INVAL;
		}
	} 
	ret = page_insert(dstenv->env_pgdir, ppage, round_dstva, perm);
	if(ret < 0) {
		if(debug_mode) panic("[DBEUG] sys_mem_map: page_insert error here\n");
		return ret;
	}

	return ret;
}

/* Overview:
 * 	Unmap the page of memory at 'va' in the address space of 'envid'
 * (if no page is mapped, the function silently succeeds)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Cannot unmap pages above UTOP.
 */
int sys_mem_unmap(int sysno, u_int envid, u_int va)
{
	// Your code here.
	int ret;
	struct Env *env;
	Pte *ppte;
	
	if(va >= UTOP || va < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_unmap: va >= UTOP\n");
		return -E_INVAL;
	}

	ret = envid2env(envid, &env, 1);
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_mem_unmap: envid2env-envid wrong here \n");
		return ret;
	}
	// u_int round_va = ROUNDDOWN(va, BY2PG);
	
	page_remove(env->env_pgdir, va);

	return ret;
	//	panic("sys_mem_unmap not implemented");
}

/* Overview:
 * 	Allocate a new environment.
 *
 * Pre-Condition:
 * The new child is left as env_alloc created it, except that
 * status is set to ENV_NOT_RUNNABLE and the register set is copied
 * from the current environment.
 *
 * Post-Condition:
 * 	In the child, the register set is tweaked so sys_env_alloc returns 0.
 * 	Returns envid of new environment, or < 0 on error.
 */
int sys_env_alloc(void)
{
	// Your code here.
	int r;
	struct Env *e;

	r = env_alloc(&e, curenv->env_id);		// alloc the child env
	if(r < 0) {
		if(debug_mode) panic("[DEBUG] sys_env_alloc: env_alloc has wrong\n");
		return r;
	}
	e->env_status = ENV_NOT_RUNNABLE;
	// copy the Trapframe
	bcopy( (void*)(KERNEL_SP - sizeof(struct Trapframe)), (void*)(&(e->env_tf)), sizeof(struct Trapframe));	// src dst len
	
	e->env_tf.pc = e->env_tf.cp0_epc;
	e->env_tf.regs[2] = 0;

	e->env_pri = curenv->env_pri;
	return e->env_id;
	//	panic("sys_env_alloc not implemented");
}

/* Overview:
 * 	Set envid's env_status to status.
 *
 * Pre-Condition:
 * 	status should be one of `ENV_RUNNABLE`, `ENV_NOT_RUNNABLE` and
 * `ENV_FREE`. Otherwise return -E_INVAL.
 * 
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if status is not a valid status for an environment.
 * 	The status of environment will be set to `status` on success.
 */
int sys_set_env_status(int sysno, u_int envid, u_int status)
{
	// Your code here.
	struct Env *env;
	int ret;
	ret = envid2env(envid, &env, 0);
	if(ret < 0) {
		if(debug_mode) panic("[DEBUG] sys_set_env_status: wrong at envid2env!\n");
		return ret;
	}
	if(status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE) {
		if(debug_mode) panic("[DEBUG] sys_set_env_status: env has the wrong status!\n");
		return -E_INVAL;
	}
	
	env->env_status = status;				//  是否需要将它装入可以调度的队列呢？在这里env本身就在可以调度的队列里面。

	return 0;
	//	panic("sys_env_set_status not implemented");
}

/* Overview:
 * 	Set envid's trap frame to tf.
 *
 * Pre-Condition:
 * 	`tf` should be valid.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if the environment cannot be manipulated.
 *
 * Note: This hasn't be used now?
 */
int sys_set_trapframe(int sysno, u_int envid, struct Trapframe *tf)
{

	return 0;
}

/* Overview:
 * 	Kernel panic with message `msg`. 
 *
 * Pre-Condition:
 * 	msg can't be NULL
 *
 * Post-Condition:
 * 	This function will make the whole system stop.
 */
void sys_panic(int sysno, char *msg)
{
	// no page_fault_mode -- we are trying to panic!
	panic("%s", TRUP(msg));
}

/* Overview:
 * 	This function enables caller to receive message from 
 * other process. To be more specific, it will flag 
 * the current process so that other process could send 
 * message to it.
 *
 * Pre-Condition:
 * 	`dstva` is valid (Note: NULL is also a valid value for `dstva`).
 * 
 * Post-Condition:
 * 	This syscall will set the current process's status to 
 * ENV_NOT_RUNNABLE, giving up cpu. 
 */
void sys_ipc_recv(int sysno, u_int dstva)
{
	if (dstva >= UTOP) {
		if(debug_mode) panic("[DEBUG] sys_ipc_recv: wrong dstva!\n");
	}
	curenv->env_status = ENV_NOT_RUNNABLE;
	curenv->env_ipc_dstva = dstva;
	curenv->env_ipc_recving = 1;
	// sched_yield();
	sys_yield();
}

/* Overview:
 * 	Try to send 'value' to the target env 'envid'.
 *
 * 	The send fails with a return value of -E_IPC_NOT_RECV if the
 * target has not requested IPC with sys_ipc_recv.
 * 	Otherwise, the send succeeds, and the target's ipc fields are
 * updated as follows:
 *    env_ipc_recving is set to 0 to block future sends
 *    env_ipc_from is set to the sending envid
 *    env_ipc_value is set to the 'value' parameter
 * 	The target environment is marked runnable again.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Hint: the only function you need to call is envid2env.
 */
int sys_ipc_can_send(int sysno, u_int envid, u_int value, u_int srcva,
					 u_int perm)
{

	int r;
	struct Env *e;
	struct Page *p;
	
	r = envid2env(envid, &e, 0);
	if(r < 0){
		if(debug_mode) panic("[DEBUG] sys_ipc_can_send: envid2env wrong!\n");
		return r;
	}
	if(e->env_ipc_recving != 1) {
		return -E_IPC_NOT_RECV;
	}
	e->env_ipc_recving = 0;
	e->env_ipc_from = curenv->env_id;
	e->env_ipc_value = value;
	if(srcva != 0) {
		r = sys_mem_map(sysno, curenv->env_id, srcva, e->env_id, e->env_ipc_dstva, perm);
		if(r < 0){
			if(debug_mode) panic("[DEBUG] sys_ipc_mem_send: sys_mem_map wrong!\n");
			return r;
		}
	}
	e->env_ipc_perm = perm;
	e->env_status = ENV_RUNNABLE; 
	return 0;
}

/* Overview:
 * 	This function is used to write data to device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (see Hint below);
 * 
 * Pre-Condition:
 *      'va' is the startting address of source data, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 	
 * Post-Condition:
 *      copy data from 'va' to 'dev' with length 'len'
 *      Return 0 on success.
 *	Return -E_INVAL on address error.
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 *	 Physical device address:
 *	* ---------------------------------*
 *	|   device   | start addr | length |
 *	* -----------+------------+--------*
 *	|  console   | 0x10000000 | 0x20   |
 *	|    IDE     | 0x13000000 | 0x4200 |
 *	|    rtc     | 0x15000000 | 0x200  |
 *	* ---------------------------------*
 */
int sys_write_dev(int sysno, u_int va, u_int dev, u_int len)
{		
	// check the va
	int match_addr = 0;
	if(dev >= 0x10000000 && dev < 0x10000020) match_addr = 1;
	else if (dev >= 0x13000000 && (dev+len) < 0x13004200) match_addr = 1;
	else if (dev >= 0x15000000 &&(dev+len) < 0x15000200) match_addr = 1;
	else {
		if(debug_mode) panic("[DEBUG] sys_write_dev: va is error!\n");
		return -E_INVAL;
	}

	int dev_kva = dev + 0xA0000000;
	bcopy(va, dev_kva, len);
	return 0;
}

/* Overview:
 * 	This function is used to read data from device, which is
 * 	represented by its mapped physical address.
 *	Remember to check the validity of device address (same as sys_read_dev)
 * 
 * Pre-Condition:
 *      'va' is the startting address of data buffer, 'len' is the
 *      length of data (in bytes), 'dev' is the physical address of
 *      the device
 * 
 * Post-Condition:
 *      copy data from 'dev' to 'va' with length 'len'
 *      Return 0 on success, < 0 on error
 *      
 * Hint: Use ummapped segment in kernel address space to perform MMIO.
 */
int sys_read_dev(int sysno, u_int va, u_int dev, u_int len)
{
        // Your code here
	int match_addr = 0;
	if(dev >= 0x10000000 && dev < 0x10000020) match_addr = 1;
	else if (dev >= 0x13000000 && (dev+len) < 0x13004200) match_addr = 1;
	else if (dev >= 0x15000000 && (dev+len) < 0x15000200) match_addr = 1;
	else {
		if(debug_mode) panic("[DEBUG] sys_write_dev: va is error!\n");
		return -E_INVAL;
	}
	u_int dev_va = dev + 0xA0000000;

	bcopy(dev_va, va, len);
	return 0;
}
