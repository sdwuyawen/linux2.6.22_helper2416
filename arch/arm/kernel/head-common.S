/*
 *  linux/arch/arm/kernel/head-common.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (c) 2003 ARM Limited
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

	.type	__switch_data, %object
__switch_data:
	.long	__mmap_switched
	.long	__data_loc			@ r4
	.long	__data_start			@ r5
	.long	__bss_start			@ r6
	.long	_end				@ r7
	.long	processor_id			@ r4
	.long	__machine_arch_type		@ r5
	.long	cr_alignment			@ r6
	.long	init_thread_union + THREAD_START_SP @ sp

/*
 * The following fragment of code is executed with the MMU on in MMU mode,
 * and uses absolute addresses; this is not position independent.
 *
 *  r0  = cp#15 control register
 *  r1  = machine ID
 *  r9  = processor ID
 */
	.type	__mmap_switched, %function
__mmap_switched:					/* 调用这个函数开始，已经使能了MMU，完全使用虚拟地址 */
	adr	r3, __switch_data + 4

	ldmia	r3!, {r4, r5, r6, r7}	/* 把r3开始的地址中的值依次赋给r4-r7 */
	cmp	r4, r5				@ Copy data segment if needed
1:	cmpne	r5, r6
	ldrne	fp, [r4], #4
	strne	fp, [r5], #4
	bne	1b

	mov	fp, #0				@ Clear BSS (and zero fp)
1:	cmp	r6, r7
	strcc	fp, [r6],#4
	bcc	1b
								/* !!注意:这里修改了新的SP!! */
	ldmia	r3, {r4, r5, r6, sp}	/* 把r3开始的地址中的值依次赋给r4,r5,r6,sp */
	str	r9, [r4]			@ Save processor ID	/* 修改processor_id变量，把r9的值赋给r4的值指向的单元，即processor_id=r9，r9保存的是processor id */
	str	r1, [r5]			@ Save machine type	/* 修改__machine_arch_type变量，把r1的值赋给r5的值指向的单元，即__machine_arch_type=r1，r1保存的是machine type */
	bic	r4, r0, #CR_A			@ Clear 'A' bit
	stmia	r6, {r0, r4}			@ Save control register values	/* 把r0,r4依次存入r6地址开始的单元中 */
	b	start_kernel

/*
 * Exception handling.  Something went wrong and we can't proceed.  We
 * ought to tell the user, but since we don't have any guarantee that
 * we're even running on the right architecture, we do virtually nothing.
 *
 * If CONFIG_DEBUG_LL is set we try to print out something about the error
 * and hope for the best (useful if bootloader fails to pass a proper
 * machine ID for example).
 */

	.type	__error_p, %function
__error_p:
#ifdef CONFIG_DEBUG_LL
	adr	r0, str_p1
	bl	printascii
	b	__error
str_p1:	.asciz	"\nError: unrecognized/unsupported processor variant.\n"
	.align
#endif

	.type	__error_a, %function
__error_a:
#ifdef CONFIG_DEBUG_LL
	mov	r4, r1				@ preserve machine ID
	adr	r0, str_a1
	bl	printascii
	mov	r0, r4
	bl	printhex8
	adr	r0, str_a2
	bl	printascii
	adr	r3, 3f
	ldmia	r3, {r4, r5, r6}		@ get machine desc list
	sub	r4, r3, r4			@ get offset between virt&phys
	add	r5, r5, r4			@ convert virt addresses to
	add	r6, r6, r4			@ physical address space
1:	ldr	r0, [r5, #MACHINFO_TYPE]	@ get machine type
	bl	printhex8
	mov	r0, #'\t'
	bl	printch
	ldr     r0, [r5, #MACHINFO_NAME]	@ get machine name
	add	r0, r0, r4
	bl	printascii
	mov	r0, #'\n'
	bl	printch
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	adr	r0, str_a3
	bl	printascii
	b	__error
str_a1:	.asciz	"\nError: unrecognized/unsupported machine ID (r1 = 0x"
str_a2:	.asciz	").\n\nAvailable machine support:\n\nID (hex)\tNAME\n"
str_a3:	.asciz	"\nPlease check your kernel config and/or bootloader.\n"
	.align
#endif

	.type	__error, %function
__error:
#ifdef CONFIG_ARCH_RPC
/*
 * Turn the screen red on a error - RiscPC only.
 */
	mov	r0, #0x02000000
	mov	r3, #0x11
	orr	r3, r3, r3, lsl #8
	orr	r3, r3, r3, lsl #16
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
#endif
1:	mov	r0, r0
	b	1b


/*
 * Read processor ID register (CP#15, CR0), and look up in the linker-built
 * supported processor list.  Note that we can't use the absolute addresses
 * for the __proc_info lists since we aren't running with the MMU on
 * (and therefore, we are not in the correct address space).  We have to
 * calculate the offset.
 *
 *	r9 = cpuid
 * Returns:
 *	r3, r4, r6 corrupted
 *	r5 = proc_info pointer in physical address space
 *	r9 = cpuid (preserved)
 */
	.type	__lookup_processor_type, %function
__lookup_processor_type:
	adr	r3, 3f				/* 获取3.这个代码当前运行地址，因为没有开MMU，所以是phy address */
	ldmda	r3, {r5 - r7}		/* r5=__proc_info_begin,r6=__proc_info_end,r7=(3.这个代码链接时地址),都是链接时的地址 */
	sub	r3, r3, r7			@ get offset between virt&phys		/* 虚拟地址和物理地址差值 */
	add	r5, r5, r3			@ convert virt addresses to	/* __proc_info_begin物理地址 */
	add	r6, r6, r3			@ physical address space		/* __proc_info_end物理地址 */
1:	ldmia	r5, {r3, r4}			@ value, mask	/* 从物理地址取值,r3=value,r4=mask */
	and	r4, r4, r9			@ mask wanted bits	/* r4=mask&CPU ID */
	teq	r3, r4		/* r3==(r4=mask&CPU ID)?*/
	beq	2f			/* 找到对应的CPU了 */
	add	r5, r5, #PROC_INFO_SZ		@ sizeof(proc_info_list)	/* r5指向下一个proc_info_list结构体 */
	cmp	r5, r6	/* r5已经指向__proc_info_end? */
	blo	1b		/* 没有结束，继续比较 */
	mov	r5, #0				@ unknown processor		/* 没有找到对应CPU,则r5返回0 */
2:	mov	pc, lr

/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_processor_type)
	stmfd	sp!, {r4 - r7, r9, lr}
	mov	r9, r0
	bl	__lookup_processor_type
	mov	r0, r5
	ldmfd	sp!, {r4 - r7, r9, pc}

/*
 * Look in include/asm-arm/procinfo.h and arch/arm/kernel/arch.[ch] for
 * more information about the __proc_info and __arch_info structures.
 */
	.long	__proc_info_begin	/* /arch/arm/kernel/vmlinux.lds中*(.proc.info.init)起始地址 */
	.long	__proc_info_end	/* /arch/arm/kernel/vmlinux.lds中*(.proc.info.init)结束地址 */
3:	.long	.
	.long	__arch_info_begin
	.long	__arch_info_end

/*
 * Lookup machine architecture in the linker-build list of architectures.
 * Note that we can't use the absolute addresses for the __arch_info
 * lists since we aren't running with the MMU on (and therefore, we are
 * not in the correct address space).  We have to calculate the offset.
 *
 *  r1 = machine architecture number
 * Returns:
 *  r3, r4, r6 corrupted
 *  r5 = mach_info pointer in physical address space
 */
	.type	__lookup_machine_type, %function
__lookup_machine_type:
	adr	r3, 3b				@ 注释: r3 = address of 3b, real address, phy address
	ldmia	r3, {r4, r5, r6}	@ r4 = "." virtual address of 3b, r5 = __arch_info_begin, r6 = __arch_info_end
	sub	r3, r3, r4			@ get offset between virt&phys, r3 = r3 -r4 = phy - virtual
	add	r5, r5, r3			@ convert virt addresses to
	add	r6, r6, r3			@ physical address space
/*
  __arch_info_begin = .;
   *(.arch.info.init)
  __arch_info_end = .;
*/
1:	ldr	r3, [r5, #MACHINFO_TYPE]	@ get machine type
	teq	r3, r1				@ matches loader number?
	beq	2f				@ found
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	mov	r5, #0				@ unknown machine
2:	mov	pc, lr

/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_machine_type)
	stmfd	sp!, {r4 - r6, lr}
	mov	r1, r0
	bl	__lookup_machine_type
	mov	r0, r5
	ldmfd	sp!, {r4 - r6, pc}
