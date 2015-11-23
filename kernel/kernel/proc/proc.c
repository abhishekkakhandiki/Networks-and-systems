/******************************************************************************/
/* Important Fall 2015 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "kernel.h"
#include "config.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"
#include "proc/proc.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/mm.h"
#include "mm/mman.h"

#include "vm/vmmap.h"

#include "fs/vfs.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"
#include "fs/file.h"

proc_t *curproc = NULL; /* global */
static slab_allocator_t *proc_allocator = NULL;

static list_t _proc_list;
static proc_t *proc_initproc = NULL; /* Pointer to the init process (PID 1) */

void proc_init() {
	list_init(&_proc_list);
	proc_allocator = slab_allocator_create("proc", sizeof(proc_t));
	KASSERT(proc_allocator != NULL);
}

proc_t *
proc_lookup(int pid) {
	proc_t *p;
	list_iterate_begin(&_proc_list, p, proc_t, p_list_link)
				{
					if (p->p_pid == pid) {
						return p;
					}
				}list_iterate_end();
	return NULL;
}

list_t *
proc_list() {
	return &_proc_list;
}

static pid_t next_pid = 0;

/**
 * Returns the next available PID.
 *
 * Note: Where n is the number of running processes, this algorithm is
 * worst case O(n^2). As long as PIDs never wrap around it is O(n).
 *
 * @return the next available PID
 */
static int _proc_getid() {
	proc_t *p;
	pid_t pid = next_pid;
	while (1) {
		failed:
		list_iterate_begin(&_proc_list, p, proc_t, p_list_link)
					{
						if (p->p_pid == pid) {
							if ((pid = (pid + 1) % PROC_MAX_COUNT)
									== next_pid) {
								return -1;
							} else {
								goto failed;
							}
						}
					}list_iterate_end();
					next_pid = (pid + 1) % PROC_MAX_COUNT;
					return pid;
				}
			}

			/*
			 * The new process, although it isn't really running since it has no
			 * threads, should be in the PROC_RUNNING state.
			 *
			 * Don't forget to set proc_initproc when you create the init
			 * process. You will need to be able to reference the init process
			 * when reparenting processes to the init process.
			 */
proc_t *
proc_create(char *name) {
	proc_t *p = NULL;
	proc_allocator = slab_allocator_create(name, sizeof(proc_t));
	p = slab_obj_alloc(proc_allocator);

	if (strcmp(name, "idle") == 0) {
		p->p_pid = PID_IDLE;
		p->p_cwd=vfs_root_vn;
	} else if (strcmp(name, "init") == 0) {
		p->p_pid = PID_INIT;
		KASSERT(PID_IDLE != p->p_pid || list_empty(&_proc_list));
		dbg(DBG_PRINT, "(GRADING1A 2.a)\n");
		proc_initproc = p;
		p->p_cwd=vfs_root_vn;
	} else {
		p->p_pid = (pid_t) _proc_getid();
		KASSERT(PID_INIT != p->p_pid || PID_IDLE == curproc->p_pid);
		dbg(DBG_PRINT, "(GRADING1A 2.a)\n");
		p->p_cwd=curproc->p_cwd;
		if(curproc->p_cwd!=NULL && curproc->p_pid !=2)
		vref(curproc->p_cwd);
	}
	strncpy(p->p_comm, name, PROC_NAME_LEN);
	list_init(&p->p_threads);
	list_init(&p->p_children);
	if (strcmp(name, "idle") == 0) {
		p->p_pproc = NULL;
	} else {
		p->p_pproc = curproc;
	}
	sched_queue_init(&p->p_wait);
	p->p_state = PROC_RUNNING;
	p->p_status = 0;
	p->p_pagedir = pt_create_pagedir();
	int i=0;
	for(i=0;i<NFILES;i++){
		p->p_files[i]=NULL;
	}


	list_link_init(&p->p_list_link);
	list_link_init(&p->p_child_link);
	list_insert_head(&_proc_list, &p->p_list_link);
	if (curproc != NULL) {
		list_insert_head(&curproc->p_children, &p->p_child_link);
	}

	/* curproc = p; */
	return p;
}

/**
 * Cleans up as much as the process as can be done from within the
 * process. This involves:
 *    - Closing all open files (VFS)
 *    - Cleaning up VM mappings (VM)
 *    - Waking up its parent if it is waiting
 *    - Reparenting any children to the init process
 *    - Setting its status and state appropriately
 *
 * The parent will finish destroying the process within do_waitpid (make
 * sure you understand why it cannot be done here). Until the parent
 * finishes destroying it, the process is informally called a 'zombie'
 * process.
 *
 * This is also where any children of the current process should be
 * reparented to the init process (unless, of course, the current
 * process is the init process. However, the init process should not
 * have any children at the time it exits).
 *
 * Note: You do _NOT_ have to special case the idle process. It should
 * never exit this way.
 *
 * @param status the status to exit the process with
 */
void proc_cleanup(int status) {

	KASSERT(NULL != proc_initproc); /* should have an "init" process */

	KASSERT(1 <= curproc->p_pid); /* this process should not be idle process */
	KASSERT(NULL != curproc->p_pproc); /* this process should have parent process */

	proc_t *p = NULL;

	if (curproc != proc_initproc) {
		list_iterate_begin(&curproc -> p_children, p, proc_t, p_child_link)
					{
						p->p_pproc = proc_initproc;
						list_remove(&p->p_child_link);
						list_insert_head(&proc_initproc->p_children,
								&p->p_child_link);
					}list_iterate_end();
				}

	KASSERT(NULL != curproc->p_pproc); /* this process should have parent process */
	if(curproc->p_cwd!=NULL && curproc->p_pid !=2)
	vput(curproc->p_cwd);
	int fd1;
	for( fd1 = 0;fd1<NFILES;fd1++){
		if(curproc->p_files[fd1]!=NULL&&curproc->p_files[fd1]->f_refcount > 0){
			do_close(fd1);
		}
	}
	curproc->p_cwd = NULL;
	curproc->p_status = status;
	curproc->p_state = PROC_DEAD;

	if (!sched_queue_empty(&curproc->p_pproc->p_wait))
		sched_broadcast_on(&curproc->p_pproc->p_wait);
}

/*
 * This has nothing to do with signals and kill(1).
 *
 * Calling this on the current process is equivalent to calling
 * do_exit().
 *
 * In Weenix, this is only called from proc_kill_all.
 */
void proc_kill(proc_t *p, int status) {

	if (p == curproc) {
		do_exit(status);
	} else {
		p->p_status = status;
		kthread_t *k = NULL;
		list_iterate_begin(&p -> p_threads, k, kthread_t, kt_plink)
					{
						kthread_cancel(k, k->kt_retval);
					}list_iterate_end();
				}
				/*NOT_YET_IMPLEMENTED("PROCS: proc_kill");*/
			}

			/*
			 * Remember, proc_kill on the current process will _NOT_ return.
			 * Don't kill direct children of the idle process.
			 *
			 * In Weenix, this is only called by sys_halt.
			 */
void proc_kill_all() {
	proc_t *p = NULL;
	list_iterate_begin(&_proc_list, p, proc_t, p_list_link)
				{
					if (p != curproc && p->p_pid != 0
							&& p->p_pproc->p_pid != 0) {
						proc_kill(p, p->p_status);
					}

				}list_iterate_end();

	if (curproc->p_pid != 0 && curproc->p_pproc->p_pid != 0) {
		proc_kill(curproc, curproc->p_status);
	}

	/*NOT_YET_IMPLEMENTED("PROCS: proc_kill_all");*/
}

/*
 * This function is only called from kthread_exit.
 *
 * Unless you are implementing MTP, this just means that the process
 * needs to be cleaned up and a new thread needs to be scheduled to
 * run. If you are implementing MTP, a single thread exiting does not
 * necessarily mean that the process should be exited.
 */
void proc_thread_exited(void *retval) {
	proc_cleanup(curproc->p_status);
	sched_switch();
	/*NOT_YET_IMPLEMENTED("PROCS: proc_thread_exited");*/
}

/* If pid is -1 dispose of one of the exited children of the current
 * process and return its exit status in the status argument, or if
 * all children of this process are still running, then this function
 * blocks on its own p_wait queue until one exits.
 *
 * If pid is greater than 0 and the given pid is a child of the
 * current process then wait for the given pid to exit and dispose
 * of it.
 *
 * If the current process has no children, or the given pid is not
 * a child of the current process return -ECHILD.
 *
 * Pids other than -1 and positive numbers are not supported.
 * Options other than 0 are not supported.
 */
pid_t do_waitpid(pid_t pid, int options, int *status) {
	proc_t *proc;
	pid_t pid_temp;
	kthread_t *thr;

	if (pid < -1 || options != 0) {
		dbg(DBG_PRINT,"Invalid pid/options\n");
		return -EINVAL;
	}
	if(list_empty(&curproc->p_children)) {
		dbg(DBG_PRINT,"Children list is empty\n");
		return -ECHILD;
	}
	if (pid == -1) {
		while (1) {
			list_iterate_begin(&curproc->p_children, proc, proc_t, p_child_link)
						{
							if (proc->p_state == PROC_DEAD) {

								KASSERT(-1 == pid || proc->p_pid == pid);
								dbg(DBG_PRINT, "(GRADING1A 2.c)\n");

								proc->p_status = 0;
								status = &proc->p_status;
								pid_temp = proc->p_pid;
								KASSERT(NULL != proc->p_pagedir);
								dbg(DBG_PRINT, "(GRADING1A 2.c)\n");
								dbg_print("The process has a valid pagedir\n");
								KASSERT(NULL != proc);

								dbg(DBG_PRINT, "(GRADING1A 2.c)\n");
								dbg_print(
										"The dead child process should not be null\n");

								list_iterate_begin(&proc->p_threads, thr, kthread_t,kt_plink)
											{
												KASSERT(
														KT_EXITED == thr->kt_state);
												dbg(DBG_PRINT,
														"(GRADING1A 2.c)\n");
												dbg_print(
														"Thread points to a thread to be destroyed\n");
												kthread_destroy(thr);
											}list_iterate_end();
								if (status != NULL)
									*status = proc->p_status;
								list_remove(&proc->p_list_link);
								list_remove(&proc->p_child_link);
								pt_destroy_pagedir(proc->p_pagedir);
								slab_obj_free(proc_allocator, proc);
								return pid_temp;
							}

						}list_iterate_end();
						sched_sleep_on(&curproc->p_wait);
					}
				}
	if (pid > 0) {
		list_iterate_begin(&curproc->p_children, proc, proc_t, p_child_link)
					{
						if (proc->p_pid == pid) {
							while (1) {
								if (proc->p_state == PROC_DEAD) {
									list_iterate_begin(&proc->p_threads, thr, kthread_t,kt_plink)
												{
													KASSERT(
															KT_EXITED == thr->kt_state);
													dbg(DBG_PRINT,
															"(GRADING1A 2.c)\n");
													list_remove(&thr->kt_plink);
													kthread_destroy(thr);
												}list_iterate_end();
												if(status!=NULL)
												*status=proc->p_status;
												list_remove(&proc->p_list_link);
												list_remove(&proc->p_child_link);
												pt_destroy_pagedir(proc->p_pagedir);
												slab_obj_free(proc_allocator, proc);
												return pid;

											}
											sched_sleep_on(&curproc->p_wait);

										}

									}

								}list_iterate_end();
								return -ECHILD;
							}
	return 0;
}
/*
 * Cancel all threads, join with them, and exit from the current
 * thread.
 *
 * @param status the exit status of the process
 */
void do_exit(int status) {
	kthread_t *thr = NULL;
	curproc->p_status = status;

	list_iterate_begin(&curproc->p_threads, thr, kthread_t,kt_plink)
				{
					kthread_cancel(thr, thr->kt_retval);
				}list_iterate_end();

			}

size_t proc_info(const void *arg, char *buf, size_t osize) {
	const proc_t *p = (proc_t *) arg;
	size_t size = osize;
	proc_t *child;

	KASSERT(NULL != p);
	KASSERT(NULL != buf);

	iprintf(&buf, &size, "pid:          %i\n", p->p_pid);
	iprintf(&buf, &size, "name:         %s\n", p->p_comm);
	if (NULL != p->p_pproc) {
		iprintf(&buf, &size, "parent:       %i (%s)\n", p->p_pproc->p_pid,
				p->p_pproc->p_comm);
	} else {
		iprintf(&buf, &size, "parent:       -\n");
	}

#ifdef __MTP__
	int count = 0;
	kthread_t *kthr;
	list_iterate_begin(&p->p_threads, kthr, kthread_t, kt_plink) {
		++count;
	}list_iterate_end();
	iprintf(&buf, &size, "thread count: %i\n", count);
#endif

	if (list_empty(&p->p_children)) {
		iprintf(&buf, &size, "children:     -\n");
	} else {
		iprintf(&buf, &size, "children:\n");
	}
	list_iterate_begin(&p->p_children, child, proc_t, p_child_link)
				{
					iprintf(&buf, &size, "     %i (%s)\n", child->p_pid,
							child->p_comm);
				}list_iterate_end();

	iprintf(&buf, &size, "status:       %i\n", p->p_status);
	iprintf(&buf, &size, "state:        %i\n", p->p_state);

#ifdef __VFS__
#ifdef __GETCWD__
	if (NULL != p->p_cwd) {
		char cwd[256];
		lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
		iprintf(&buf, &size, "cwd:          %-s\n", cwd);
	} else {
		iprintf(&buf, &size, "cwd:          -\n");
	}
#endif /* __GETCWD__ */
#endif

#ifdef __VM__
	iprintf(&buf, &size, "start brk:    0x%p\n", p->p_start_brk);
	iprintf(&buf, &size, "brk:          0x%p\n", p->p_brk);
#endif

	return size;
}

size_t proc_list_info(const void *arg, char *buf, size_t osize) {
	size_t size = osize;
	proc_t *p;

	KASSERT(NULL == arg);
	KASSERT(NULL != buf);

#if defined(__VFS__) && defined(__GETCWD__)
	iprintf(&buf, &size, "%5s %-13s %-18s %-s\n", "PID", "NAME", "PARENT", "CWD");
#else
	iprintf(&buf, &size, "%5s %-13s %-s\n", "PID", "NAME", "PARENT");
#endif

	list_iterate_begin(&_proc_list, p, proc_t, p_list_link)
				{
					char parent[64];
					if (NULL != p->p_pproc) {
						snprintf(parent, sizeof(parent), "%3i (%s)",
								p->p_pproc->p_pid, p->p_pproc->p_comm);
					} else {
						snprintf(parent, sizeof(parent), "  -");
					}

#if defined(__VFS__) && defined(__GETCWD__)
					if (NULL != p->p_cwd) {
						char cwd[256];
						lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
						iprintf(&buf, &size, " %3i  %-13s %-18s %-s\n",
								p->p_pid, p->p_comm, parent, cwd);
					} else {
						iprintf(&buf, &size, " %3i  %-13s %-18s -\n",
								p->p_pid, p->p_comm, parent);
					}
#else
					iprintf(&buf, &size, " %3i  %-13s %-s\n", p->p_pid,
							p->p_comm, parent);
#endif
				}list_iterate_end();
	return size;
}
