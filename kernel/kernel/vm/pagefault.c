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

#include "types.h"
#include "globals.h"
#include "kernel.h"
#include "errno.h"

#include "util/debug.h"

#include "proc/proc.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/pagetable.h"

#include "vm/pagefault.h"
#include "vm/vmmap.h"

/*
 * This gets called by _pt_fault_handler in mm/pagetable.c The
 * calling function has already done a lot of error checking for
 * us. In particular it has checked that we are not page faulting
 * while in kernel mode. Make sure you understand why an
 * unexpected page fault in kernel mode is bad in Weenix. You
 * should probably read the _pt_fault_handler function to get a
 * sense of what it is doing.
 *
 * Before you can do anything you need to find the vmarea that
 * contains the address that was faulted on. Make sure to check
 * the permissions on the area to see if the process has
 * permission to do [cause]. If either of these checks does not
 * pass kill the offending process, setting its exit status to
 * EFAULT (normally we would send the SIGSEGV signal, however
 * Weenix does not support signals).
 *
 * Now it is time to find the correct page (don't forget
 * about shadow objects, especially copy-on-write magic!). Make
 * sure that if the user writes to the page it will be handled
 * correctly.
 *
 * Finally call pt_map to have the new mapping placed into the
 * appropriate page table.
 *
 * @param vaddr the address that was accessed to cause the fault
 *
 * @param cause this is the type of operation on the memory
 *              address which caused the fault, possible values
 *              can be found in pagefault.h
 */
void
handle_pagefault(uintptr_t vaddr, uint32_t cause)
{
        /*NOT_YET_IMPLEMENTED("VM: handle_pagefault");*/
		vmarea_t *vmarea;
		vmmap_t *vmap;
		uint32_t vfn;
		vfn = ADDR_TO_PN(vaddr);
		vmap = curproc->p_vmmap;
		pframe_t *page;
		vmarea = vmmap_lookup(vmap,vfn);
		vmarea->vma_vmmap=vmap;


	     if(cause & FAULT_WRITE)
	     {
		  if( vmarea->vma_prot == FAULT_WRITE)
			  vmarea->vma_obj->mmo_ops->lookuppage(vmarea->vma_obj,vmarea->vma_off + ADDR_TO_PN(vaddr) - vmarea->vma_start ,FAULT_WRITE & cause,&page);
              pt_map(curproc->p_pagedir,PAGE_ALIGN_DOWN(vaddr),pt_virt_to_phys(page->pf_addr),FAULT_WRITE,FAULT_WRITE);
	     }
	     else if  (cause & FAULT_USER)
	     {
	   		  if( vmarea->vma_prot == FAULT_USER)
	   			  vmarea->vma_obj->mmo_ops->lookuppage(vmarea->vma_obj,vmarea->vma_off + ADDR_TO_PN(vaddr) - vmarea->vma_start ,FAULT_USER & cause,&page);
	                 pt_map(curproc->p_pagedir,PAGE_ALIGN_DOWN(vaddr),pt_virt_to_phys(page->pf_addr),FAULT_USER,FAULT_USER);
	     }
	     else if(cause & FAULT_EXEC)
	     {
	   		  if( vmarea->vma_prot == FAULT_EXEC)
	   			  vmarea->vma_obj->mmo_ops->lookuppage(vmarea->vma_obj,vmarea->vma_off + ADDR_TO_PN(vaddr) - vmarea->vma_start ,FAULT_EXEC & cause,&page);
	                 pt_map(curproc->p_pagedir,PAGE_ALIGN_DOWN(vaddr),pt_virt_to_phys(page->pf_addr),FAULT_EXEC,FAULT_EXEC);
	     }
	     else if  (cause & FAULT_RESERVED)
	     {
	   		  if( vmarea->vma_prot == FAULT_RESERVED)
	   			  vmarea->vma_obj->mmo_ops->lookuppage(vmarea->vma_obj,vmarea->vma_off + ADDR_TO_PN(vaddr) - vmarea->vma_start ,FAULT_RESERVED & cause,&page);
	                 pt_map(curproc->p_pagedir,PAGE_ALIGN_DOWN(vaddr),pt_virt_to_phys(page->pf_addr),FAULT_RESERVED,FAULT_RESERVED);
	     }
	     else if  (cause & FAULT_PRESENT)
	     {
	   	   		  if( vmarea->vma_prot == FAULT_PRESENT)
	   	   			  vmarea->vma_obj->mmo_ops->lookuppage(vmarea->vma_obj,vmarea->vma_off + ADDR_TO_PN(vaddr) - vmarea->vma_start ,FAULT_PRESENT & cause,&page);
	   	                 pt_map(curproc->p_pagedir,PAGE_ALIGN_DOWN(vaddr),pt_virt_to_phys(page->pf_addr),FAULT_PRESENT,FAULT_PRESENT);
	     }


	   }

