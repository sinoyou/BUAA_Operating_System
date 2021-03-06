// implement fork from user space

#include "lib.h"
#include <mmu.h>
#include <env.h>

/* ----------------- help functions ---------------- */

/* Overview:
 * 	Copy `len` bytes from `src` to `dst`.
 *
 * Pre-Condition:
 * 	`src` and `dst` can't be NULL. Also, the `src` area 
 * 	 shouldn't overlap the `dest`, otherwise the behavior of this 
 * 	 function is undefined.
 */
void user_bcopy(const void *src, void *dst, size_t len)
{
	void *max;

	//	writef("~~~~~~~~~~~~~~~~ src:%x dst:%x len:%x\n",(int)src,(int)dst,len);
	max = dst + len;

	// copy machine words while possible
	if (((int)src % 4 == 0) && ((int)dst % 4 == 0)) {
		while (dst + 3 < max) {
			*(int *)dst = *(int *)src;
			dst += 4;
			src += 4;
		}
	}

	// finish remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst += 1;
		src += 1;
	}

	//for(;;);
}

/* Overview:
 * 	Sets the first n bytes of the block of memory 
 * pointed by `v` to zero.
 * 
 * Pre-Condition:
 * 	`v` must be valid.
 *
 * Post-Condition:
 * 	the content of the space(from `v` to `v`+ n) 
 * will be set to zero.
 */
void user_bzero(void *v, u_int n)
{
	char *p;
	int m;

	p = v;
	m = n;

	while (--m >= 0) {
		*p++ = 0;
	}
}
/*--------------------------------------------------------------*/

/* Overview:
 * 	Custom page fault handler - if faulting page is copy-on-write,
 * map in our own private writable copy.
 * 
 * Pre-Condition:
 * 	`va` is the address which leads to a TLBS exception.
 *
 * Post-Condition:
 *  Launch a user_panic if `va` is not a copy-on-write page.
 * Otherwise, this handler should map a private writable copy of 
 * the faulting page at correct address.
 */
static void
pgfault(u_int va)
{
	u_int *tmp;
	int ret;
	//	writef("fork.c:pgfault():\t va:%x\n",va);
    struct Page* ppage;
	Pte* pte;

	va = ROUNDDOWN(va, BY2PG);
	tmp = UXSTACKTOP - 2 * BY2PG;
	u_int perm = (*vpt)[VPN(va)] & 0xfff;
	if((perm & PTE_COW) == 0) {
		user_panic("[DEBUG] pgfault: pte's perm wrong!\n");
		return ;
	}
	ret = syscall_mem_alloc(0, tmp, (perm & (~PTE_COW)|PTE_R));
	if(ret < 0) {
		user_panic("[DEBUG] pgfault: syscall_mem_alloc!\n");
		return ;
	}

    //map the new page at a temporary place
	//copy the content
	user_bcopy(va, tmp, BY2PG);    // ???why do we need to copy it ?
    //map the page on the appropriate place
    if(syscall_mem_map(0, tmp, 0, va, (perm & (~PTE_COW)|PTE_R))!=0) {
		user_panic("[DEBUG] pgfault: syscall_mem_map error !\n");
		return ;
	}
	//unmap the temporary place
	ret = syscall_mem_unmap(0, tmp);
	if(ret < 0) {
		user_panic("[DEBUG] pgfault: syscall_mem_unmap error !\n");
		return ;

	}
}

/* Overview:
 * 	Map our virtual page `pn` (address pn*BY2PG) into the target `envid`
 * at the same virtual address. 
 *
 * Post-Condition:
 *  if the page is writable or copy-on-write, the new mapping must be 
 * created copy on write and then our mapping must be marked 
 * copy on write as well. In another word, both of the new mapping and
 * our mapping should be copy-on-write if the page is writable or 
 * copy-on-write.
 * 
 * Hint:
 * 	PTE_LIBRARY indicates that the page is shared between processes.
 * A page with PTE_LIBRARY may have PTE_R at the same time. You
 * should process it correctly.
 */
static void
duppage(u_int envid, u_int pn)
{
	u_int addr;
	u_int perm;
	int ret = 0;
	
	// vpd -> pde[]
	// vpt -> pte[]
	Pte * ppte = vpt[pn];
	perm = (*vpt)[pn] & 0xfff;
	addr = pn*BY2PG;

	if(perm & PTE_V) {
		if((perm & PTE_R)&&!(perm&PTE_LIBRARY)&&!(perm&PTE_COW)){
			perm = perm | PTE_COW;
			if(syscall_mem_map(0,addr,envid,addr,perm)!=0) {
				user_panic("[DEBUG] fork.c duppage: syscall_mem_map1!\n");
			} 
			if(syscall_mem_map(0,addr,0,addr,perm)!=0) {
				user_panic("[DEBUG] fork.c duppage: syscall_mem_map2!\n");
			}
		} else {
			if(syscall_mem_map(0,addr,envid,addr,perm)!=0) {
				user_panic("[DEBUG] fork.c suppage: syscall_mem_map3!\n");
			}
		}
	}
}



	/*
	if(perm & PTE_V) {
		if((perm & PTE_R) || (perm & PTE_COW)) {
			if(perm & PTE_LIBRARY) {
				perm = perm | PTE_V | PTE_R;
			} else {
				perm = perm | PTE_V | PTE_R | PTE_COW;
			}
			ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG, perm);
			if(ret < 0) return ret;
			ret = syscall_mem_map(0, pn*BY2PG, 0, pn*BY2PG, perm);
			if(ret < 0) return ret;

		} else {
		ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG,	perm);
		if(ret < 0) return ret;			
		}
	}*/

	/*
	if((perm & PTE_V) == 0) {
		// not valide
	}else if((perm & PTE_R) == 0) {
		// read only 
		ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG, perm);			// srcenvid: 0 -> curenv
		if(ret < 0) {
			user_panic("[DEBUG] child map failed! read only\n");
			return ret;
		}
	} else if ((perm & PTE_LIBRARY) > 0) {
		// library
		ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG, perm);
		if(ret < 0) {
			user_panic("[DEBUG] child map failed! library\n");
			return ret;
		}
	} else if ((perm & PTE_COW) > 0) { 
		// has PTE_COW : Last fork
		// father
		ret = syscall_mem_map(0, pn*BY2PG, 0, pn*BY2PG, perm);
		if(ret < 0) {
			user_panic("[DEBUG] father map failed! PTE_COW\n");
			return ret;
		}
		// child
		ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG, perm);
		if(ret < 0) {
			user_panic("[DEBUG] child map failed! PTE_COW\n");
			return ret;
		}
	} else {
		perm = perm | PTE_COW;

		ret = syscall_mem_map(0, pn*BY2PG, 0, pn*BY2PG, perm);
		if(ret < 0) {
			user_panic("[DEBUG] father map failed! PTE_COW\n");
			return ret;
		}
		ret = syscall_mem_map(0, pn*BY2PG, envid, pn*BY2PG, perm);
		if(ret < 0) {
			user_panic("[DEBUG] child map failed! PTE_COW\n");
			return ret;
		}
	} */
	//	user_panic("duppage not implemented");

/* Overview:
 * 	User-level fork. Create a child and then copy our address space
 * and page fault handler setup to the child.
 *
 * Hint: use vpd, vpt, and duppage.
 * Hint: remember to fix "env" in the child process!
 * Note: `set_pgfault_handler`(user/pgfault.c) is different from 
 *       `syscall_set_pgfault_handler`. 
 */
extern void __asm_pgfault_handler(void);
int
fork(void)
{
	// Your code here.
	u_int newenvid;
	extern struct Env *envs;
	extern struct Env *env;
	u_int i;

	//The parent installs pgfault using set_pgfault_handler
	set_pgfault_handler(pgfault);						// what does va/pgfault use here?
	//alloc a new alloc
	newenvid = syscall_env_alloc();	
	
	if(newenvid == 0) {
		// child 
		env = &(envs[ENVX(syscall_getenvid())]);
	} else {
		// father
		for(i = 0;i < USTACKTOP;i += BY2PG) {
			if(((*vpd)[VPN(i) /1024 ])!=0 && ((*vpt)[VPN(i)])!=0) {
				duppage(newenvid, VPN(i));
			}
		}
		syscall_mem_alloc(newenvid, UXSTACKTOP - BY2PG, PTE_V|PTE_R|PTE_LIBRARY); //分配子进程的异常处理栈
		syscall_set_pgfault_handler(newenvid, __asm_pgfault_handler, UXSTACKTOP); // 设置子进程的处理函数
		syscall_set_env_status(newenvid, ENV_RUNNABLE );  // 设置子进程的运行状态

	}
	return newenvid;
}

// Challenge!
int
sfork(void)
{
	user_panic("sfork not implemented");
	return -E_INVAL;
}
