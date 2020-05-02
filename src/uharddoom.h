#ifndef UHARDDOOM_H
#define UHARDDOOM_H

/* Section 1: PCI ids. */

#define UHARDDOOM_VENDOR_ID			0x0666
#define UHARDDOOM_DEVICE_ID			0x1995

/* Section 2: MMIO registers.  */

/* Section 2.1: main control area.  */

/* Enables active units of the device.  TLB and CACHE is passive and doesn't
 * have an enable (disable the client instead).  FIFOs also don't have enables
 * — disable the source and/or destination unit instead.  */
#define UHARDDOOM_ENABLE				0x0000
#define UHARDDOOM_ENABLE_BATCH				0x00000001
#define UHARDDOOM_ENABLE_CMD				0x00000004
#define UHARDDOOM_ENABLE_FE				0x00000008
#define UHARDDOOM_ENABLE_SRD				0x00000010
#define UHARDDOOM_ENABLE_SPAN				0x00000020
#define UHARDDOOM_ENABLE_COL				0x00000040
#define UHARDDOOM_ENABLE_FX				0x00000080
#define UHARDDOOM_ENABLE_SWR				0x00000100
#define UHARDDOOM_ENABLE_ALL				0x000001fd
/* Status of device units — 1 means they have work to do.  */
#define UHARDDOOM_STATUS				0x0004
#define UHARDDOOM_STATUS_BATCH				0x00000001
#define UHARDDOOM_STATUS_JOB				0x00000002
#define UHARDDOOM_STATUS_CMD				0x00000004
#define UHARDDOOM_STATUS_FE				0x00000008
#define UHARDDOOM_STATUS_SRD				0x00000010
#define UHARDDOOM_STATUS_SPAN				0x00000020
#define UHARDDOOM_STATUS_COL				0x00000040
#define UHARDDOOM_STATUS_FX				0x00000080
#define UHARDDOOM_STATUS_SWR				0x00000100
#define UHARDDOOM_STATUS_FIFO_SRDCMD			0x00010000
#define UHARDDOOM_STATUS_FIFO_SPANCMD			0x00020000
#define UHARDDOOM_STATUS_FIFO_COLCMD			0x00040000
#define UHARDDOOM_STATUS_FIFO_FXCMD			0x00080000
#define UHARDDOOM_STATUS_FIFO_SWRCMD			0x00100000
#define UHARDDOOM_STATUS_FIFO_COLIN			0x00200000
#define UHARDDOOM_STATUS_FIFO_FXIN			0x00400000
#define UHARDDOOM_STATUS_FIFO_FESEM			0x01000000
#define UHARDDOOM_STATUS_FIFO_SRDSEM			0x02000000
#define UHARDDOOM_STATUS_FIFO_COLSEM			0x04000000
#define UHARDDOOM_STATUS_FIFO_SPANSEM			0x08000000
#define UHARDDOOM_STATUS_FIFO_SPANOUT			0x10000000
#define UHARDDOOM_STATUS_FIFO_COLOUT			0x20000000
#define UHARDDOOM_STATUS_FIFO_FXOUT			0x40000000
/* The reset register.  Punching 1 will clear all pending work and/or
 * cached data.  */
#define UHARDDOOM_RESET					0x0004
#define UHARDDOOM_RESET_JOB				0x00000002
#define UHARDDOOM_RESET_CMD				0x00000004
#define UHARDDOOM_RESET_FE				0x00000008
#define UHARDDOOM_RESET_SRD				0x00000010
#define UHARDDOOM_RESET_SPAN				0x00000020
#define UHARDDOOM_RESET_COL				0x00000040
#define UHARDDOOM_RESET_FX				0x00000080
#define UHARDDOOM_RESET_SWR				0x00000100
#define UHARDDOOM_RESET_STATS				0x00000200
#define UHARDDOOM_RESET_TLB_KERNEL			0x00000400
#define UHARDDOOM_RESET_TLB_USER			0x00000800
#define UHARDDOOM_RESET_CACHE_COL_CMAP_B		0x00001000
#define UHARDDOOM_RESET_CACHE_COL_SRC			0x00002000
#define UHARDDOOM_RESET_CACHE_SPAN_SRC			0x00004000
#define UHARDDOOM_RESET_CACHE_SWR_TRANSMAP		0x00008000
#define UHARDDOOM_RESET_FIFO_SRDCMD			0x00010000
#define UHARDDOOM_RESET_FIFO_SPANCMD			0x00020000
#define UHARDDOOM_RESET_FIFO_COLCMD			0x00040000
#define UHARDDOOM_RESET_FIFO_FXCMD			0x00080000
#define UHARDDOOM_RESET_FIFO_SWRCMD			0x00100000
#define UHARDDOOM_RESET_FIFO_COLIN			0x00200000
#define UHARDDOOM_RESET_FIFO_FXIN			0x00400000
#define UHARDDOOM_RESET_FIFO_FESEM			0x01000000
#define UHARDDOOM_RESET_FIFO_SRDSEM			0x02000000
#define UHARDDOOM_RESET_FIFO_COLSEM			0x04000000
#define UHARDDOOM_RESET_FIFO_SPANSEM			0x08000000
#define UHARDDOOM_RESET_FIFO_SPANOUT			0x10000000
#define UHARDDOOM_RESET_FIFO_COLOUT			0x20000000
#define UHARDDOOM_RESET_FIFO_FXOUT			0x40000000
#define UHARDDOOM_RESET_ALL				0x7ffffffe
/* Interrupt status.  */
#define UHARDDOOM_INTR					0x0008
#define UHARDDOOM_INTR_BATCH_WAIT			0x00000001
#define UHARDDOOM_INTR_JOB_DONE				0x00000002
#define UHARDDOOM_INTR_FE_ERROR				0x00000010
#define UHARDDOOM_INTR_CMD_ERROR			0x00000020
#define UHARDDOOM_INTR_PAGE_FAULT(i)			(0x00000100 << (i))
#define UHARDDOOM_INTR_PAGE_FAULT_BATCH			0x00000100
#define UHARDDOOM_INTR_PAGE_FAULT_CMD			0x00000200
#define UHARDDOOM_INTR_PAGE_FAULT_SRD			0x00000400
#define UHARDDOOM_INTR_PAGE_FAULT_SWR_DST		0x00000800
#define UHARDDOOM_INTR_PAGE_FAULT_COL_CMAP_B		0x00001000
#define UHARDDOOM_INTR_PAGE_FAULT_COL_SRC		0x00002000
#define UHARDDOOM_INTR_PAGE_FAULT_SPAN_SRC		0x00004000
#define UHARDDOOM_INTR_PAGE_FAULT_SWR_TRANSMAP		0x00008000
#define UHARDDOOM_INTR_MASK				0x0000ff33
/* And enable (same bitfields).  */
#define UHARDDOOM_INTR_ENABLE				0x000c

/* Section 2.2: BATCH — batch processor.  Fetches (user pd, cmd ptr,
 * cmd size) tuples from kernel pd and pokes JOB.
 *
 * The pseudocode for this unit is:
 *
 * while True:
 *     # Whenever we have an available batch to read and JOB is not busy...
 *     if not STATUS.JOB and BATCH_GET != BATCH_PUT:
 *         # Read a 16-byte batch structure from the kernel PD
 *         cur_batch = mem_read(BATCH_PD, BATCH_GET, 0x10)
 *         # And send it to run
 *         JOB_PD = cur_batch[0:4]
 *         JOB_CMD_PTR = cur_batch[4:8]
 *         JOB_CMD_SIZE = cur_batch[8:12]
 *         # bytes 12:16 ignored
 *         JOB.trigger_from_batch()
 *         # Wait until JOB is finished
 *         JOB.wait()
 *         # Increment BATCH_GET, possibly wrapping
 *         BATCH_GET += 0x10
 *         if BATCH_GET == BATCH_WRAP:
 *             BATCH_GET = 0
 *         # If we reached the interrupt value, trigger interrupt.
 *         if BATCH_GET == BATCH_WAIT:
 *             INTR |= INTR_BATCH_WAIT
 * */

/* The kernel PD, used for fetching batch jobs.  Goes straight to
 * TLB_KERNEL_PD.  */
#define UHARDDOOM_BATCH_PDP				0x0020
/* If BATCH_GET would be equal to this value after incrementing, set it to 0
 * instead.  */
#define UHARDDOOM_BATCH_WRAP				0x0024
/* The current kernel vaddr that BATCH is reading batches from.  Must be
 * 0x10-byte aligned.  */
#define UHARDDOOM_BATCH_GET				0x0028
/* Current end pointer — when BATCH_GET is equal to BATCH_PUT, BATCH will
 * halt until more data is available.  */
#define UHARDDOOM_BATCH_PUT				0x002c
/* Interrupt pointer — when BATCH_GET is incremented and reaches this value,
 * the BATCH_WAIT interrupt will be triggered.  */
#define UHARDDOOM_BATCH_WAIT				0x0030
#define UHARDDOOM_BATCH_PTR_MASK			0xfffffff0
#define UHARDDOOM_BATCH_JOB_SIZE			0x10

/* Section 2.3: JOB — the main job controller.  Collects a single job, sends
 * the PD to TLB, pokes CMD and FE, waits until the whole job is finished,
 * notifies BATCH of completion, raises JOB_DONE interrupt.
 *
 * Functions:
 *
 * 1. Can be triggered by BATCH or manually.  When this happens, the PD is
 *    sent to TLB_USER_PD, the CMD pointer and size is sent to PD, and
 *    the ACTIVE bit is set in state.
 * 2. When JOB_WAIT is read by FE, the read will block until the ACTIVE bit
 *    is set.
 * 3. When JOB_DONE is written by FE, the job considered to be done — the
 *    JOB_DONE interrupt is triggered, ACTIVE bit is cleared, and BATCH
 *    is notified of the JOB completion (if FROM_BATCH flag was set).
 */

/* The user PD for this job.  Goes straight to TLB_USER_PD.  */
#define UHARDDOOM_JOB_PDP				0x0040
/* Pointer (in user PD) to commands for this job.  Goes straight to
 * CMD_CMD_PTR.  */
#define UHARDDOOM_JOB_CMD_PTR				0x0044
/* Size (in bytes) of command buffer for this job.  Goes straight to
 * CMD_CMD_SIZE.  */
#define UHARDDOOM_JOB_CMD_SIZE				0x0048
/* Write any value to this register to manually trigger a new job.  */
#define UHARDDOOM_JOB_TRIGGER				0x004c
/* A state register.  */
#define UHARDDOOM_JOB_STATE				0x0050
/* Set if a job is active.  Cleared by FE when it decides we're done.  */
#define UHARDDOOM_JOB_STATE_ACTIVE			0x00000001
/* Set if the current job comes from BATCH (and not manual trigger).  If set,
 * will notify BATCH that it should bump the BATCH_GET pointer when we're
 * done with the current job.  */
#define UHARDDOOM_JOB_STATE_FROM_BATCH			0x00000002
#define UHARDDOOM_JOB_STATE_MASK			0x00000003

/* Section 2.4: CMD — the command fetcher.  Reads a stream of commands from
 * the user PD, buffers them, and delivers them to FE.  Set up by RUN.
 *
 * Functions:
 *
 * 1. When a new job is triggered in JOB, both CMD_FE_PTR and CMD_READ_PTR
 *    are set to JOB_CMD_PTR, and CMD_READ_SIZE is set to JOB_CMD_SIZE.
 * 2. Whenever there is space in the buffer, CMD reads from CMD_READ_PTR,
 *    increments it, decrements CMD_READ_SIZE accordingly, and stores
 *    the commands in the buffer.
 * 3. At any time, FE can read the CMD_END register to know if there are
 *    any commands left in the job — if CMD_READ_SIZE is zero and the buffer
 *    is empty, CMD will return 1 (otherwise, it will return 0).
 * 4. FE can read the CMD_FETCH register to get the next command from the buffer.
 *    If there is something in the buffer, the command is removed from
 *    the buffer and returned to FE.  If the buffer is empty, but
 *    CMD_READ_SIZE is non-0, the read is blocked until the next command is
 *    read.  If the buffer is empty and CMD_READ_SIZE is 0, the CMD_ERROR
 *    interrupt is raised.
 */

/* The current FE command pointer (ie. the address of the next command to be
 * delivered to FE).  */
#define UHARDDOOM_CMD_FE_PTR				0x0060
/* The current command read pointer (ie. the address of the next command to
 * be buffered).  */
#define UHARDDOOM_CMD_READ_PTR				0x0064
/* How many bytes are still left in the command buffer after CMD_READ_PTR.  */
#define UHARDDOOM_CMD_READ_SIZE				0x0068
#define UHARDDOOM_CMD_FIFO_GET				0x0070
#define UHARDDOOM_CMD_FIFO_PUT				0x0074
#define UHARDDOOM_CMD_FIFO_WINDOW			0x0078
#define UHARDDOOM_CMD_FIFO_SIZE				0x00000400
#define UHARDDOOM_CMD_PTR_MASK				0xfffffffc
#define UHARDDOOM_CMD_SIZE_MASK				0xfffffffc

/* Section 2.5: FE — the front end.  Runs firmware that reads the user
 * commands, and converts them to the proper low-level commands understood by
 * COL/SPAN/FX/SWR.  */

/* The FE RAM windows.  Reading/writing the WINDOW register reads/writes
 * a word of FE code/data RAM at address ADDR, then auto-increments ADDR by 4.  */
#define UHARDDOOM_FE_CODE_ADDR				0x0100
#define UHARDDOOM_FE_CODE_ADDR_MASK			0x0000fffc
#define UHARDDOOM_FE_CODE_WINDOW			0x0104
#define UHARDDOOM_FE_DATA_ADDR				0x0108
#define UHARDDOOM_FE_DATA_ADDR_MASK			0x0000fffc
#define UHARDDOOM_FE_DATA_WINDOW			0x010c
/* The current program counter.  */
#define UHARDDOOM_FE_PC					0x0110
#define UHARDDOOM_FE_PC_MASK				0xfffffffc
/* The current execution state.  */
#define UHARDDOOM_FE_STATE				0x0114
/* The core is actively executing instructions.  */
#define UHARDDOOM_FE_STATE_STATE_RUNNING		0x00000000
/* The core is halted due to an error.  */
#define UHARDDOOM_FE_STATE_STATE_ERROR			0x00000001
/* The core is blocked on JOB_WAIT read.  */
#define UHARDDOOM_FE_STATE_STATE_JOB_WAIT		0x00000002
/* The core is blocked on CMD_FETCH read.  */
#define UHARDDOOM_FE_STATE_STATE_CMD_FETCH		0x00000003
/* The core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_STATE_STATE_SRDCMD			0x00000004
#define UHARDDOOM_FE_STATE_STATE_SPANCMD		0x00000005
#define UHARDDOOM_FE_STATE_STATE_COLCMD			0x00000006
#define UHARDDOOM_FE_STATE_STATE_FXCMD			0x00000007
#define UHARDDOOM_FE_STATE_STATE_SWRCMD			0x00000008
/* The core is blocked on FESEM read.  */
#define UHARDDOOM_FE_STATE_STATE_FESEM			0x00000009
#define UHARDDOOM_FE_STATE_STATE_MASK			0x0000000f
/* The pending command code, when core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_STATE_CMD_MASK			0x000000f0
#define UHARDDOOM_FE_STATE_CMD_SHIFT			4
/* The destination register, when core is blocked on a register read.  */
#define UHARDDOOM_FE_STATE_DST_MASK			0x00001f00
#define UHARDDOOM_FE_STATE_DST_SHIFT			8
#define UHARDDOOM_FE_STATE_MASK				0x00001fff
/* The pending write data, when core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_WRITE_DATA				0x0118
/* The FE error data (set when FE_ERROR interrupt is triggered).  */
#define UHARDDOOM_FE_ERROR_DATA_A			0x0120
#define UHARDDOOM_FE_ERROR_DATA_B			0x0124
/* The FE error code (set when FE_ERROR interrupt is triggered).  */
#define UHARDDOOM_FE_ERROR_CODE				0x0128
/* Unknown user command.  Data A is cmd pointer, data B is command header.  */
#define UHARDDOOM_FE_ERROR_CODE_UNK_USER_COMMAND	0x00000000
/* Unaligned dst pointer.  Data A is cmd pointer, data B is dst pointer.  */
#define UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED	0x00000001
/* Unaligned dst pitch.  Data A is cmd pointer, data B is dst pitch.  */
#define UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED	0x00000002
/* Unaligned colormap pointer.  Data A is cmd pointer, data B is colormap pointer.  */
#define UHARDDOOM_FE_ERROR_COLORMAP_UNALIGNED		0x00000003
/* Y coordinates for DRAW_COLUMNS in wrong order.  Data A is cmd pointer, data B is command word.  */
#define UHARDDOOM_FE_ERROR_DRAW_COLUMNS_Y_REV		0x00000004
/* X coordinates for DRAW_SPANS in wrong order.  Data A is cmd pointer, data B is command word.  */
#define UHARDDOOM_FE_ERROR_DRAW_SPANS_X_REV		0x00000005
/* The FE core encountered an illegal instruction.  A is address, B is
 * the instruction opcode.  */
#define UHARDDOOM_FE_ERROR_CODE_ILLEGAL_INSTRUCTION	0x00000080
/* The FE core tried to jump to an unaligned address.  A is current PC, B is would-be new PC.  */
#define UHARDDOOM_FE_ERROR_CODE_UNALIGNED_INSTRUCTION	0x00000081
/* The FE core encountered a load bus error.  A is the faulting
 * address.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_LOAD		0x00000082
/* The FE core encountered a store bus error.  A is the faulting
 * address, B is the written data.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_STORE		0x00000083
/* The FE core encountered an instruction fetch bus error.  A is the
 * faulting address.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_EXEC		0x00000084
#define UHARDDOOM_FE_ERROR_CODE_MASK			0x000000ff
/* The registers.  */
#define UHARDDOOM_FE_REG(i)				(0x0180 + (i) * 4)
#define UHARDDOOM_FE_REG_NUM				0x20

/* Section 2.6: FIFOs.  */

#define UHARDDOOM_FIFO_SRDCMD_GET			0x0200
#define UHARDDOOM_FIFO_SRDCMD_PUT			0x0204
#define UHARDDOOM_FIFO_SRDCMD_CMD_WINDOW		0x0208
#define UHARDDOOM_FIFO_SRDCMD_DATA_WINDOW		0x020c
#define UHARDDOOM_FIFO_SRDCMD_SIZE			0x40
#define UHARDDOOM_FIFO_SPANCMD_GET			0x0210
#define UHARDDOOM_FIFO_SPANCMD_PUT			0x0214
#define UHARDDOOM_FIFO_SPANCMD_CMD_WINDOW		0x0218
#define UHARDDOOM_FIFO_SPANCMD_DATA_WINDOW		0x021c
#define UHARDDOOM_FIFO_SPANCMD_SIZE			0x40
#define UHARDDOOM_FIFO_COLCMD_GET			0x0220
#define UHARDDOOM_FIFO_COLCMD_PUT			0x0224
#define UHARDDOOM_FIFO_COLCMD_CMD_WINDOW		0x0228
#define UHARDDOOM_FIFO_COLCMD_DATA_WINDOW		0x022c
#define UHARDDOOM_FIFO_COLCMD_SIZE			0x40
#define UHARDDOOM_FIFO_FXCMD_GET			0x0230
#define UHARDDOOM_FIFO_FXCMD_PUT			0x0234
#define UHARDDOOM_FIFO_FXCMD_CMD_WINDOW			0x0238
#define UHARDDOOM_FIFO_FXCMD_DATA_WINDOW		0x023c
#define UHARDDOOM_FIFO_FXCMD_SIZE			0x40
#define UHARDDOOM_FIFO_SWRCMD_GET			0x0240
#define UHARDDOOM_FIFO_SWRCMD_PUT			0x0244
#define UHARDDOOM_FIFO_SWRCMD_CMD_WINDOW		0x0248
#define UHARDDOOM_FIFO_SWRCMD_DATA_WINDOW		0x024c
#define UHARDDOOM_FIFO_SWRCMD_SIZE			0x40
#define UHARDDOOM_FIFO_CMD_MASK				0x0000000f
/* The 4 semaphore registers.  Bumped by one by SWR commands, decreased by FE/SRD/SPAN/COL.  */
#define UHARDDOOM_FIFO_FESEM				0x0250
#define UHARDDOOM_FIFO_SRDSEM				0x0254
#define UHARDDOOM_FIFO_COLSEM				0x0258
#define UHARDDOOM_FIFO_SPANSEM				0x025c
#define UHARDDOOM_FIFO_SEM_MASK				0x00000001
#define UHARDDOOM_FIFO_COLIN_GET			0x0260
#define UHARDDOOM_FIFO_COLIN_PUT			0x0264
#define UHARDDOOM_FIFO_COLIN_SIZE			0x40
#define UHARDDOOM_FIFO_FXIN_GET				0x0268
#define UHARDDOOM_FIFO_FXIN_PUT				0x026c
#define UHARDDOOM_FIFO_FXIN_SIZE			0x40
#define UHARDDOOM_FIFO_COLIN_DATA_WINDOW		0x0280
#define UHARDDOOM_FIFO_FXIN_DATA_WINDOW			0x02c0
#define UHARDDOOM_FIFO_SPANOUT_GET			0x0300
#define UHARDDOOM_FIFO_SPANOUT_PUT			0x0304
#define UHARDDOOM_FIFO_SPANOUT_SIZE			0x40
#define UHARDDOOM_FIFO_COLOUT_GET			0x0310
#define UHARDDOOM_FIFO_COLOUT_PUT			0x0314
#define UHARDDOOM_FIFO_COLOUT_SIZE			0x40
#define UHARDDOOM_FIFO_COLOUT_MASK_WINDOW		0x0318
#define UHARDDOOM_FIFO_FXOUT_GET			0x0320
#define UHARDDOOM_FIFO_FXOUT_PUT			0x0324
#define UHARDDOOM_FIFO_FXOUT_SIZE			0x40
#define UHARDDOOM_FIFO_FXOUT_MASK_WINDOW		0x0328
#define UHARDDOOM_FIFO_SPANOUT_DATA_WINDOW		0x0340
#define UHARDDOOM_FIFO_COLOUT_DATA_WINDOW		0x0380
#define UHARDDOOM_FIFO_FXOUT_DATA_WINDOW		0x03c0

/* Section 2.7: TLB.  */

/* The TLB client indices.  */
#define UHARDDOOM_TLB_CLIENT_BATCH			0
#define UHARDDOOM_TLB_CLIENT_CMD			1
#define UHARDDOOM_TLB_CLIENT_SRD			2
#define UHARDDOOM_TLB_CLIENT_SWR_DST			3
#define UHARDDOOM_TLB_CLIENT_COL_CMAP_B			4
#define UHARDDOOM_TLB_CLIENT_COL_SRC			5
#define UHARDDOOM_TLB_CLIENT_SPAN_SRC			6
#define UHARDDOOM_TLB_CLIENT_SWR_TRANSMAP		7
#define UHARDDOOM_TLB_CLIENT_NUM			8
/* The master PD pointers.  */
#define UHARDDOOM_TLB_KERNEL_PDP			0x0400
#define UHARDDOOM_TLB_USER_PDP				0x0404
/* The PDE cache pool (two entries).  */
#define UHARDDOOM_TLB_POOL_PDE_TAG(i)			(0x0410 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PDE_VALUE(i)			(0x0414 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PDE_SIZE			2
/* The per-client singular PTE TLBs.  */
#define UHARDDOOM_TLB_CLIENT_PTE_TAG(i)			(0x0440 + (i) * 8)
#define UHARDDOOM_TLB_CLIENT_PTE_VALUE(i)		(0x0444 + (i) * 8)
/* The last translated virt address for each client (useful for page fault handling).  */
#define UHARDDOOM_TLB_CLIENT_VA(i)			(0x0480 + (i) * 4)
/* The PTE cache pool.  */
#define UHARDDOOM_TLB_POOL_PTE_TAG(i)			(0x0600 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PTE_VALUE(i)			(0x0604 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PTE_SIZE			64
/* The tag fields.  */
#define UHARDDOOM_TLB_TAG_VALID				0x00000001
/* Set if the TLB entry relates to kernel PD.  */
#define UHARDDOOM_TLB_TAG_KERNEL			0x00000002
#define UHARDDOOM_TLB_TAG_MASK_PD			0xffc00003
#define UHARDDOOM_TLB_TAG_MASK_PT			0xfffff003

/* Section 2.8: STATS.  */

/* XXX: destined for 0x800:0xa00 */

/* Section 2.9: SRD — surface read unit.  Reads raw surface or colorbuf
 * blocks, sends them to COL or FX.  */

#define UHARDDOOM_SRD_STATE				0x0a00
#define UHARDDOOM_SRD_STATE_READ_LENGTH_MASK		0x0000ffff
/* If 1, reads to COL, otherwise reads to FX.  */
#define UHARDDOOM_SRD_STATE_COL				0x00010000
/* If 1, waiting on SRDSEM.  */
#define UHARDDOOM_SRD_STATE_SRDSEM			0x10000000
#define UHARDDOOM_SRD_STATE_FESEM			0x20000000
#define UHARDDOOM_SRD_STATE_MASK			0x3001ffff
/* The virtual base address of the source.  */
#define UHARDDOOM_SRD_SRC_PTR				0x0a04
/* The pitch of the source.  */
#define UHARDDOOM_SRD_SRC_PITCH				0x0a08

/* Section 2.10: SPAN — the span reader.  Used to read texture data for
 * the DRAW_SPAN function, and to read source data for the BLIT
 * function.  Sends the texels to the FX unit.
 *
 * The pseudocode for the main function (DRAW) is as follows:
 *
 * for _ in range(DRAW.LENGTH):
 *     u = (USTART >> 16) & ((1 << UVMASK.ULOG) - 1)
 *     v = (VSTART >> 16) & ((1 << UVMASK.VLOG) - 1)
 *     texel_ptr = src_ptr + v * src_pitch + u
 *     block[DRAW.XOFF] = cached_mem_read(USER_PD, texel_ptr)
 *     USTART += USTEP
 *     VSTART += VSTEP
 *     if DRAW.XOFF == 63:
 *         DRAW.XOFF = 0
 *         SPANOUT.send(block)
 *     else:
 *         DRAW.XOFF += 1
 * if DRAW.XOFF != 0:
 *     SPANOUT.send(block)
 */

#define UHARDDOOM_SPAN_STATE				0x0a80
#define UHARDDOOM_SPAN_STATE_DRAW_LENGTH_MASK		0x0000ffff
#define UHARDDOOM_SPAN_STATE_DRAW_XOFF_MASK		0x003f0000
#define UHARDDOOM_SPAN_STATE_DRAW_XOFF_SHIFT		16
/* If 1, waiting on SPANSEM.  */
#define UHARDDOOM_SPAN_STATE_SPANSEM			0x10000000
#define UHARDDOOM_SPAN_STATE_MASK			0x103fffff
/* The virtual base address of the source.  */
#define UHARDDOOM_SPAN_SRC_PTR				0x0a84
/* The pitch of the source.  */
#define UHARDDOOM_SPAN_SRC_PITCH			0x0a88
/* The mask of the source uv coords.  */
#define UHARDDOOM_SPAN_UVMASK				0x0a8c
#define UHARDDOOM_SPAN_UVMASK_MASK			0x00001f1f
#define UHARDDOOM_SPAN_USTART				0x0a90
#define UHARDDOOM_SPAN_VSTART				0x0a94
#define UHARDDOOM_SPAN_USTEP				0x0a98
#define UHARDDOOM_SPAN_VSTEP				0x0a9c

/* Section 2.11: SWR — surface write unit.  Gathers blocks from COL or FX,
 * possibly runs them through the TRANSMAP, writes them to memory.  */

#define UHARDDOOM_SWR_STATE				0x0b00
#define UHARDDOOM_SWR_STATE_DRAW_LENGTH_MASK		0x0000ffff
#define UHARDDOOM_SWR_STATE_COL_EN			0x00010000
#define UHARDDOOM_SWR_STATE_TRANS_EN			0x00020000
#define UHARDDOOM_SWR_STATE_SRC_BUF_FULL		0x00040000
#define UHARDDOOM_SWR_STATE_DST_BUF_FULL		0x00080000
#define UHARDDOOM_SWR_STATE_TRANS_POS_MASK		0x07f00000
#define UHARDDOOM_SWR_STATE_TRANS_POS_SHIFT		20
/* If 1, pending SRDSEM, COLSEM, SPANSEM.  */
#define UHARDDOOM_SWR_STATE_SRDSEM			0x10000000
#define UHARDDOOM_SWR_STATE_COLSEM			0x20000000
#define UHARDDOOM_SWR_STATE_SPANSEM			0x40000000
#define UHARDDOOM_SWR_STATE_MASK			0x77fff7ff
#define UHARDDOOM_SWR_TRANSMAP_PTR			0x0b04
#define UHARDDOOM_SWR_DST_PTR				0x0b08
#define UHARDDOOM_SWR_DST_PTR_MASK			0xffffffc0
#define UHARDDOOM_SWR_DST_PITCH				0x0b0c
#define UHARDDOOM_SWR_DST_PITCH_MASK			0xffffffc0
/* 64-bit */
#define UHARDDOOM_SWR_BLOCK_MASK			0x0b10
/* The three buffers: source data, orignal destination data, post-TRANSMAP
 * data.  If TRANSMAP is not enabled, only SRC_BUF is used.  */
#define UHARDDOOM_SWR_SRC_BUF(i)			(0x0b40 + (i))
#define UHARDDOOM_SWR_DST_BUF(i)			(0x0b80 + (i))
#define UHARDDOOM_SWR_TRANS_BUF(i)			(0x0bc0 + (i))

/* Section 2.11: FX — the effects unit.  Gets raw texture data from the SPAN
 * unit, or draws with a constant color.  Applies colormap for spans, handles
 * solid drawing, does the fuzz effect.  */

#define UHARDDOOM_FX_STATE				0x0c00
#define UHARDDOOM_FX_STATE_DRAW_LENGTH_MASK		0x0000ffff
#define UHARDDOOM_FX_STATE_DRAW_CMAP_EN			0x00010000
#define UHARDDOOM_FX_STATE_DRAW_FUZZ_EN			0x00020000
#define UHARDDOOM_FX_STATE_DRAW_FUZZ_POS_MASK		0x000c0000
#define UHARDDOOM_FX_STATE_DRAW_FUZZ_POS_SHIFT		18
#define UHARDDOOM_FX_STATE_DRAW_FETCH_SRD		0x00100000
#define UHARDDOOM_FX_STATE_DRAW_FETCH_SPAN		0x00200000
#define UHARDDOOM_FX_STATE_DRAW_FETCH_DONE		0x00400000
#define UHARDDOOM_FX_STATE_DRAW_NON_FIRST		0x00800000
#define UHARDDOOM_FX_STATE_LOAD_MODE_CMAP		0x10000000
#define UHARDDOOM_FX_STATE_LOAD_MODE_BLOCK		0x20000000
#define UHARDDOOM_FX_STATE_LOAD_MODE_FUZZ		0x30000000
#define UHARDDOOM_FX_STATE_LOAD_MODE_MASK		0x30000000
#define UHARDDOOM_FX_STATE_LOAD_CNT_MASK		0xc0000000
#define UHARDDOOM_FX_STATE_LOAD_CNT_SHIFT		30
#define UHARDDOOM_FX_STATE_MASK				0xf0ffffff
#define UHARDDOOM_FX_SKIP				0x0c04
#define UHARDDOOM_FX_SKIP_BEGIN_MASK			0x0000003f
#define UHARDDOOM_FX_SKIP_END_MASK			0x00003f00
#define UHARDDOOM_FX_SKIP_END_SHIFT			8
#define UHARDDOOM_FX_SKIP_ALWAYS			0x00010000
#define UHARDDOOM_FX_SKIP_MASK				0x00013f3f
#define UHARDDOOM_FX_COL(i)				(0xd00 + (i) * 4)
#define UHARDDOOM_FX_COL_FUZZPOS_MASK			0x0000003f
#define UHARDDOOM_FX_COL_ENABLE				0x00000080
#define UHARDDOOM_FX_COL_MASK				0x000000bf
#define UHARDDOOM_FX_BUF(i)				(0xe00 + (i))
#define UHARDDOOM_FX_BUF_SIZE				0x100
#define UHARDDOOM_FX_CMAP(i)				(0xf00 + (i))

/* Section 2.12: COL — the column reader.  Used to read texture data for
 * the DRAW_COLUMN function, and to read source data for the WIPE function.
 * Capable of handling 64 columns at once.  Can apply up to two colormaps:
 * colormap A applies to all columns, while colormap B is per-column.
 * The textured and colormapped pixels are sent to SWR, along with the valid
 * pixel mask.  */

#define UHARDDOOM_COL_STATE				0x2000
#define UHARDDOOM_COL_STATE_DRAW_LENGTH_MASK		0x0000ffff
#define UHARDDOOM_COL_STATE_CMAP_A_EN			0x00010000
/* The next column to be textured.  */
#define UHARDDOOM_COL_STATE_XOFF_MASK			0x03f00000
#define UHARDDOOM_COL_STATE_XOFF_SHIFT			20
/* If 1, waiting on COLSEM.  */
#define UHARDDOOM_COL_STATE_COLSEM			0x10000000
/* Set if LOAD_CMAP_A in progresss.  */
#define UHARDDOOM_COL_STATE_LOAD_CMAP_A			0x20000000
/* The current position (in blocks) for LOAD_CMAP_A.  */
#define UHARDDOOM_COL_STATE_CMAP_A_POS_MASK		0xc0000000
#define UHARDDOOM_COL_STATE_CMAP_A_POS_SHIFT		30
#define UHARDDOOM_COL_STATE_MASK			0xf3f1ffff
/* Per-column command state, to be moved to proper column RAM by COL_SETUP.  */
#define UHARDDOOM_COL_STAGE_CMAP_B_PTR			0x2004
#define UHARDDOOM_COL_STAGE_SRC_PTR			0x2008
#define UHARDDOOM_COL_STAGE_SRC_PITCH			0x200c
#define UHARDDOOM_COL_STAGE_USTART			0x2010
#define UHARDDOOM_COL_STAGE_USTEP			0x2014
/* Actual per-column state.  */
#define UHARDDOOM_COL_COLS_CMAP_B_PTR(i)		(0x2100 + (i) * 4)
#define UHARDDOOM_COL_COLS_SRC_PTR(i)			(0x2200 + (i) * 4)
#define UHARDDOOM_COL_COLS_SRC_PITCH(i)			(0x2300 + (i) * 4)
#define UHARDDOOM_COL_COLS_USTART(i)			(0x2400 + (i) * 4)
#define UHARDDOOM_COL_COLS_USTEP(i)			(0x2500 + (i) * 4)
#define UHARDDOOM_COL_COLS_STATE(i)			(0x2600 + (i) * 4)
#define UHARDDOOM_COL_COLS_STATE_DATA_CMAP_MASK		0x0000007f
#define UHARDDOOM_COL_COLS_STATE_DATA_CMAP_SHIFT	0
#define UHARDDOOM_COL_COLS_STATE_ULOG_MASK		0x00001f00
#define UHARDDOOM_COL_COLS_STATE_COL_EN			0x00002000
#define UHARDDOOM_COL_COLS_STATE_CMAP_B_EN		0x00004000
#define UHARDDOOM_COL_COLS_STATE_DATA_GET_MASK		0x003f0000
#define UHARDDOOM_COL_COLS_STATE_DATA_GET_SHIFT		16
#define UHARDDOOM_COL_COLS_STATE_DATA_PUT_MASK		0x3f000000
#define UHARDDOOM_COL_COLS_STATE_DATA_PUT_SHIFT		24

#define UHARDDOOM_COL_COLS_STATE_MASK			0x3f3f7f3f
/* The CMAP_A data (256 bytes).  */
#define UHARDDOOM_COL_CMAP_A(i)				(0x2700 + (i))
/* The pre-textured data.  64 bytes for each lane.  */
#define UHARDDOOM_COL_DATA(i)				(0x3000 + (i))
#define UHARDDOOM_COL_DATA_SIZE				64

/* Section 2.13: CACHEs.  */

#define UHARDDOOM_CACHE_CLIENT_COL_CMAP_B		0
#define UHARDDOOM_CACHE_CLIENT_COL_SRC			1
#define UHARDDOOM_CACHE_CLIENT_SPAN_SRC			2
#define UHARDDOOM_CACHE_CLIENT_SWR_TRANSMAP		3
#define UHARDDOOM_CACHE_CLIENT_NUM			4
#define UHARDDOOM_CACHE_TAG(i)				(0x8000 + (i) * 4)
#define UHARDDOOM_CACHE_DATA(i)				(0xc000 + (i))
#define UHARDDOOM_CACHE_SIZE				64
#define UHARDDOOM_CACHE_TAG_VA_MASK			0xffffffc0
#define UHARDDOOM_CACHE_TAG_VALID			0x00000001
#define UHARDDOOM_CACHE_TAG_MASK			0xffffffc1

/* Section 2.14: end.  */

#define UHARDDOOM_BAR_SIZE				0x10000


/* Section 3: Page tables.  */

#define UHARDDOOM_PAGE_SIZE				0x1000
#define UHARDDOOM_PAGE_SHIFT				12
/* Page directory pointer.  */
#define UHARDDOOM_PDP_MASK				0x0fffffff
#define UHARDDOOM_PDP_SHIFT				12
/* Page directory entry.  */
#define UHARDDOOM_PDE_PRESENT				0x00000001
#define UHARDDOOM_PDE_PA_MASK				0xfffffff0
#define UHARDDOOM_PDE_PA_SHIFT				8
#define UHARDDOOM_PDE_MASK				0xfffffff1
/* Page table entry.  */
#define UHARDDOOM_PTE_PRESENT				0x00000001
#define UHARDDOOM_PTE_WRITABLE				0x00000002
#define UHARDDOOM_PTE_PA_MASK				0xfffffff0
#define UHARDDOOM_PTE_PA_SHIFT				8
#define UHARDDOOM_PTE_MASK				0xfffffff3
/* Splits the VA into PDI + PTI + OFF */
#define UHARDDOOM_VA_PDI(va)				((va) >> 22 & 0x3ff)
#define UHARDDOOM_VA_PTI(va)				((va) >> 12 & 0x3ff)
#define UHARDDOOM_VA_OFF(va)				((va) & 0xfff)
/* The parts of VA that are used for tagging PDE and PTE caches.  */
#define UHARDDOOM_VA_PDE_TAG_MASK			0xffc00000
#define UHARDDOOM_VA_PTE_TAG_MASK			0xfffff000


/* Section 4: The driver commands.  */

/* The command type.  Always in word 0.  */
#define UHARDDOOM_USER_CMD_HEADER_EXTR_TYPE(w)		((w) & 0xff)
#define UHARDDOOM_USER_CMD_TYPE_FILL_RECT		0x0
#define UHARDDOOM_USER_CMD_TYPE_DRAW_LINE		0x1
#define UHARDDOOM_USER_CMD_TYPE_BLIT			0x2
#define UHARDDOOM_USER_CMD_TYPE_WIPE			0x3
#define UHARDDOOM_USER_CMD_TYPE_DRAW_COLUMNS		0x4
#define UHARDDOOM_USER_CMD_TYPE_DRAW_FUZZ		0x5
#define UHARDDOOM_USER_CMD_TYPE_DRAW_SPANS		0x6

/* Fill rectangle.  Fills the specified rectangle with a solid color.  */
/* Word 0: command type and fill color.  */
#define UHARDDOOM_USER_FILL_RECT_HEADER(color)		(UHARDDOOM_USER_CMD_TYPE_FILL_RECT | (color) << 8)
#define UHARDDOOM_USER_FILL_RECT_HEADER_EXTR_COLOR(w)	((w) >> 8 & 0xff)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: X and Y coords of the left upper corner.  */
#define UHARDDOOM_USER_FILL_RECT_W3(x, y)		((x) | (y) << 16)
#define UHARDDOOM_USER_FILL_RECT_W3_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_FILL_RECT_W3_EXTR_Y(w)		((w) >> 16 & 0xffff)
/* Word 4: width and height of the rectangle.  */
#define UHARDDOOM_USER_FILL_RECT_W4(w, h)		((w) | (h) << 16)
#define UHARDDOOM_USER_FILL_RECT_W4_EXTR_W(w)		((w) & 0xffff)
#define UHARDDOOM_USER_FILL_RECT_W4_EXTR_H(w)		((w) >> 16 & 0xffff)

/* Draw line.  */
#define UHARDDOOM_USER_DRAW_LINE_HEADER(color)		(UHARDDOOM_USER_CMD_TYPE_DRAW_LINE | (color) << 8)
#define UHARDDOOM_USER_DRAW_LINE_HEADER_EXTR_COLOR(w)	((w) >> 8 & 0xff)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: X and Y coords of one line endpoint.  */
#define UHARDDOOM_USER_DRAW_LINE_W3(x, y)		((x) | (y) << 16)
#define UHARDDOOM_USER_DRAW_LINE_W3_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_LINE_W3_EXTR_Y(w)		((w) >> 16 & 0xffff)
/* Word 4: X and Y coords of the other line endpoint.  */
#define UHARDDOOM_USER_DRAW_LINE_W4(x, y)		((x) | (y) << 16)
#define UHARDDOOM_USER_DRAW_LINE_W4_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_LINE_W4_EXTR_Y(w)		((w) >> 16 & 0xffff)

/* Blit.  */
/* Word 0: command type and source UV mask.  */
#define UHARDDOOM_USER_BLIT_HEADER(ulog, vlog)		(UHARDDOOM_USER_CMD_TYPE_BLIT | (ulog) << 16 | (vlog) << 24)
#define UHARDDOOM_USER_BLIT_HEADER_EXTR_ULOG(w)		((w) >> 16 & 0x1f)
#define UHARDDOOM_USER_BLIT_HEADER_EXTR_VLOG(w)		((w) >> 16 & 0x1f)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: X and Y coords of the destination left upper corner.  */
#define UHARDDOOM_USER_BLIT_W3(x, y)			((x) | (y) << 16)
#define UHARDDOOM_USER_BLIT_W3_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_BLIT_W3_EXTR_Y(w)		((w) >> 16 & 0xffff)
/* Word 4: width and height of the destination rectangle.  */
#define UHARDDOOM_USER_BLIT_W4(w, h)			((w) | (h) << 16)
#define UHARDDOOM_USER_BLIT_W4_EXTR_W(w)		((w) & 0xffff)
#define UHARDDOOM_USER_BLIT_W4_EXTR_H(w)		((w) >> 16 & 0xffff)
/* Word 5: source pointer.  */
/* Word 6: source pitch.  */
/* Word 7: X and Y coords of the source left upper corner.  */
#define UHARDDOOM_USER_BLIT_W7(x, y)			((x) | (y) << 16)
#define UHARDDOOM_USER_BLIT_W7_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_BLIT_W7_EXTR_Y(w)		((w) >> 16 & 0xffff)
/* Word 8: width and height of the source rectangle.  */
#define UHARDDOOM_USER_BLIT_W8(w, h)			((w) | (h) << 16)
#define UHARDDOOM_USER_BLIT_W8_EXTR_W(w)		((w) & 0xffff)
#define UHARDDOOM_USER_BLIT_W8_EXTR_H(w)		((w) >> 16 & 0xffff)

/* Wipe.  */
/* Word 0: just the command type.  */
#define UHARDDOOM_USER_WIPE_HEADER			UHARDDOOM_USER_CMD_TYPE_WIPE
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: X and Y coords of the destination left upper corner.  */
#define UHARDDOOM_USER_WIPE_W3(x, y)			((x) | (y) << 16)
#define UHARDDOOM_USER_WIPE_W3_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_WIPE_W3_EXTR_Y(w)		((w) >> 16 & 0xffff)
/* Word 4: width and height of the destination rectangle.  */
#define UHARDDOOM_USER_WIPE_W4(w, h)			((w) | (h) << 16)
#define UHARDDOOM_USER_WIPE_W4_EXTR_W(w)		((w) & 0xffff)
#define UHARDDOOM_USER_WIPE_W4_EXTR_H(w)		((w) >> 16 & 0xffff)
/* Word 5: source A pointer.  */
/* Word 6: source A pitch.  */
/* Word 7: source B pointer.  */
/* Word 8: source B pitch.  */
/* The following word is repeated once for every X coordinate.  */
/* Repeat word 0: Y offset of a given column */

/* Draw columns.  */
/* Word 0: command type, enables, number of columns.  */
#define UHARDDOOM_USER_DRAW_COLUMNS_HEADER(cae, cbe, te, nc)	(UHARDDOOM_USER_CMD_TYPE_DRAW_COLUMNS | (cae) << 8 | (cbe) << 9 | (te) << 12 | (nc) << 16)
#define UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_A_EN(w)	((w) >> 8 & 1)
#define UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_B_EN(w)	((w) >> 9 & 1)
#define UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_TRANS_EN(w)	((w) >> 12 & 1)
#define UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_NUM_COLS(w)	((w) >> 16 & 0xffff)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3 (if enabled in header): colormap A address.  */
/* Word 4 (if enabled in header): transmap address.  */
/* The following 5 or 6 words are repeated once for every column.  */
/* Repeat word 0: x, U mask.  */
#define UHARDDOOM_USER_DRAW_COLUMNS_WR0(x, ulog)	((x) | (ulog) << 16)
#define UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_X(w)	((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_ULOG(w)	((w) >> 16 & 0x1f)
/* Repeat word 1: y0 and y1.  */
#define UHARDDOOM_USER_DRAW_COLUMNS_WR1(y0, y1)		((y0) | (y1) << 16)
#define UHARDDOOM_USER_DRAW_COLUMNS_WR1_EXTR_Y0(w)	((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_COLUMNS_WR1_EXTR_Y1(w)	((w) >> 16 & 0xffff)
/* Repeat word 2: texture pointer.  */
/* Repeat word 3: ustart.  */
/* Repeat word 4: ustep.  */
/* Repeat word 5 (if enabled in header): colormap B address.  */

/* Draw fuzz.  */
/* Word 0: command type, number of columns.  */
#define UHARDDOOM_USER_DRAW_FUZZ_HEADER(nc)		(UHARDDOOM_USER_CMD_TYPE_DRAW_FUZZ | (nc) << 16)
#define UHARDDOOM_USER_DRAW_FUZZ_HEADER_EXTR_NUM_COLS(w)	((w) >> 16 & 0xffff)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: fuzzstart and fuzzend.  */
#define UHARDDOOM_USER_DRAW_FUZZ_W3(fs, fe)		((fs) | (fe) << 16)
#define UHARDDOOM_USER_DRAW_FUZZ_W3_EXTR_FUZZSTART(w)	((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_FUZZ_W3_EXTR_FUZZEND(w)	((w) >> 16 & 0xffff)
/* Word 4: colormap address.  */
/* The following 2 words are repeated once for every column.  */
/* Repeat word 0: x, U mask.  */
#define UHARDDOOM_USER_DRAW_FUZZ_WR0(x, fuzzpos)	((x) | (fuzzpos) << 16)
#define UHARDDOOM_USER_DRAW_FUZZ_WR0_EXTR_X(w)		((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_FUZZ_WR0_EXTR_FUZZPOS(w)	((w) >> 16 & 0x3f)
/* Repeat word 1: y0 and y1.  */
#define UHARDDOOM_USER_DRAW_FUZZ_WR1(y0, y1)		((y0) | (y1) << 16)
#define UHARDDOOM_USER_DRAW_FUZZ_WR1_EXTR_Y0(w)		((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_FUZZ_WR1_EXTR_Y1(w)		((w) >> 16 & 0xffff)

/* Draw spans.  */
/* Word 0: command type, enables, UV mask.  */
#define UHARDDOOM_USER_DRAW_SPANS_HEADER(ce, te, ulog, vlog)	(UHARDDOOM_USER_CMD_TYPE_DRAW_SPANS | (ce) << 8 | (te) << 12 | (ulog) << 16 | (vlog) << 24)
#define UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_CMAP_EN(w)	((w) >> 8 & 1)
#define UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_TRANS_EN(w)	((w) >> 12 & 1)
#define UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_ULOG(w)	((w) >> 16 & 0x1f)
#define UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_VLOG(w)	((w) >> 24 & 0x1f)
/* Word 1: destination pointer.  */
/* Word 2: destination pitch.  */
/* Word 3: y0 and y1.  */
#define UHARDDOOM_USER_DRAW_SPANS_W3(y0, y1)		((y0) | (y1) << 16)
#define UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y0(w)		((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y1(w)		((w) >> 16 & 0xffff)
/* Word 4: source pointer.  */
/* Word 5: source pitch.  */
/* Word 6 (if enabled in header): transmap address.  */
/* The following 4 or 5 words are repeated once for every x coordinate.  */
/* Repeat word 0: x0 and x1.  */
#define UHARDDOOM_USER_DRAW_SPANS_WR0(x0, x1)		((x0) | (x1) << 16)
#define UHARDDOOM_USER_DRAW_SPANS_WR0_EXTR_X0(w)	((w) & 0xffff)
#define UHARDDOOM_USER_DRAW_SPANS_WR0_EXTR_X1(w)	((w) >> 16 & 0xffff)
/* Repeat word 1: ustart.  */
/* Repeat word 2: vstart.  */
/* Repeat word 3: ustep.  */
/* Repeat word 4: vstep.  */
/* Repeat word 5 (if enabled in header): colormap address.  */


/* Section 5: Misc definitions.  */

/* The block size used for drawing etc (64 pixels).  */
#define UHARDDOOM_BLOCK_SIZE				0x40
#define UHARDDOOM_BLOCK_MASK				0x3f
#define UHARDDOOM_BLOCK_SHIFT				6
#define UHARDDOOM_BLOCK_PTR_MASK			0xffffffc0
#define UHARDDOOM_BLOCK_PITCH_MASK			0xffffffc0
#define UHARDDOOM_COORD_MASK				0xffff
#define UHARDDOOM_COLORMAP_SIZE				0x100


/* Section 6: FE core internal memory map.  */

/* Write-only aliases of the FE_ERROR_DATA_* registers.  */
#define UHARDDOOM_FEMEM_FE_ERROR_DATA_A			0x00000040
#define UHARDDOOM_FEMEM_FE_ERROR_DATA_B			0x00000044
/* When written, triggers FE_ERROR and halts the core.  */
#define UHARDDOOM_FEMEM_FE_ERROR_CODE			0x00000048
/* Read to wait until a job is active.  */
#define UHARDDOOM_FEMEM_JOB_WAIT			0x00000080
/* Write to signal job done.  */
#define UHARDDOOM_FEMEM_JOB_DONE			0x00000084
/* Read only alias of CMD_FE_PTR.  */
#define UHARDDOOM_FEMEM_CMD_PTR				0x000000c0
/* If 1, means no more commands in the job.  */
#define UHARDDOOM_FEMEM_CMD_END				0x000000c4
/* Read to get the next command word.  */
#define UHARDDOOM_FEMEM_CMD_FETCH			0x000000c8
/* Write to submit a command to a FIFO.  */
#define UHARDDOOM_FEMEM_SRDCMD(t)			(0x00000100 + (t) * 4)
#define UHARDDOOM_FEMEM_SPANCMD(t)			(0x00000140 + (t) * 4)
#define UHARDDOOM_FEMEM_COLCMD(t)			(0x00000180 + (t) * 4)
#define UHARDDOOM_FEMEM_FXCMD(t)			(0x000001c0 + (t) * 4)
#define UHARDDOOM_FEMEM_SWRCMD(t)			(0x00000200 + (t) * 4)
/* Read to wait for a signal from SWR on the FESEM interface.  */
#define UHARDDOOM_FEMEM_FESEM				0x00000240
/* Write to bump a STATS counter.  */
#define UHARDDOOM_FEMEM_STATS_BUMP(t)			(0x00000300 + (t) * 4)
/* The code RAM — read only from the core.  */
#define UHARDDOOM_FEMEM_CODE_BASE			0x80000000
#define UHARDDOOM_FEMEM_CODE_SIZE			0x10000
/* The data RAM.  */
#define UHARDDOOM_FEMEM_DATA_BASE			0xc0000000
#define UHARDDOOM_FEMEM_DATA_SIZE			0x10000


/* Section 7: Internal commands.  */

/* Section 7.1: SRDCMD — SRD unit internal commands.  */

#define UHARDDOOM_SRDCMD_TYPE_NOP			0x0
/* The virtual base address of the source.  */
#define UHARDDOOM_SRDCMD_TYPE_SRC_PTR			0x1
/* The pitch of the source.  */
#define UHARDDOOM_SRDCMD_TYPE_SRC_PITCH			0x2
/* Reads blocks.  */
#define UHARDDOOM_SRDCMD_TYPE_READ			0x3
/* Wait for a signal on SRDSEM.  */
#define UHARDDOOM_SRDCMD_TYPE_SRDSEM			0x4
/* Send a signal on FESEM.  */
#define UHARDDOOM_SRDCMD_TYPE_FESEM			0x5
#define UHARDDOOM_SRDCMD_DATA_READ(len, col)		((len) | (col) << 16)
#define UHARDDOOM_SRDCMD_DATA_EXTR_READ_LENGTH(cmd)	((cmd) & 0xffff)
/* If set, send blocks to COL, otherwise to FX.  */
#define UHARDDOOM_SRDCMD_DATA_EXTR_READ_COL(cmd)	((cmd) >> 16 & 1)

/* Section 7.2: SPANCMD — SPAN unit internal commands.  */

#define UHARDDOOM_SPANCMD_TYPE_NOP			0x0
/* The virtual base address of the source.  */
#define UHARDDOOM_SPANCMD_TYPE_SRC_PTR			0x1
/* The pitch of the source.  */
#define UHARDDOOM_SPANCMD_TYPE_SRC_PITCH		0x2
/* The mask of the source uv coords.  */
#define UHARDDOOM_SPANCMD_TYPE_UVMASK			0x3
/* Straight from the command words.  */
#define UHARDDOOM_SPANCMD_TYPE_USTART			0x4
#define UHARDDOOM_SPANCMD_TYPE_VSTART			0x5
#define UHARDDOOM_SPANCMD_TYPE_USTEP			0x6
#define UHARDDOOM_SPANCMD_TYPE_VSTEP			0x7
/* Emits a given number of texels to the FX.  */
#define UHARDDOOM_SPANCMD_TYPE_DRAW			0x8
/* Waits for a signal from SWR on the SPANSEM interface, then flushes cache.  */
#define UHARDDOOM_SPANCMD_TYPE_SPANSEM			0x9

#define UHARDDOOM_SPANCMD_DATA_UVMASK(ulog, vlog)	((ulog) | (vlog) << 8)
#define UHARDDOOM_SPANCMD_DATA_EXTR_UVMASK_ULOG(cmd)	((cmd) & 0x3f)
#define UHARDDOOM_SPANCMD_DATA_EXTR_UVMASK_VLOG(cmd)	((cmd) >> 8 & 0x3f)
#define UHARDDOOM_SPANCMD_DATA_DRAW(len, xoff)		((len) | (xoff) << 16)
/* Number of actual pixels to be drawn.  */
#define UHARDDOOM_SPANCMD_DATA_EXTR_DRAW_LENGTH(cmd)	((cmd) & 0xffff)
/* Number of dummy pixels to be sent at the beginning of the first block.  */
#define UHARDDOOM_SPANCMD_DATA_EXTR_DRAW_XOFF(cmd)	((cmd) >> 16 & 0x3f)

/* Section 7.3: COLCMD — COL unit internal commands.  */

#define UHARDDOOM_COLCMD_TYPE_NOP			0x0
/* Per-column colormap B virtual address.  */
#define UHARDDOOM_COLCMD_TYPE_COL_CMAP_B_PTR		0x1
/* Per-column source virtual address.  */
#define UHARDDOOM_COLCMD_TYPE_COL_SRC_PTR		0x2
/* Per-column source pitch.  */
#define UHARDDOOM_COLCMD_TYPE_COL_SRC_PITCH		0x3
/* Per-column U coord.  */
#define UHARDDOOM_COLCMD_TYPE_COL_USTART		0x4
#define UHARDDOOM_COLCMD_TYPE_COL_USTEP			0x5
/* Sets up a column (use after the above per-column commands).  */
#define UHARDDOOM_COLCMD_TYPE_COL_SETUP			0x6
/* Load colormap A from SRD.  */
#define UHARDDOOM_COLCMD_TYPE_LOAD_CMAP_A		0x7
/* Emits a given number of blocks to the SWR.  */
#define UHARDDOOM_COLCMD_TYPE_DRAW			0x8
/* Waits for a signal from SWR on the COLSEM interface, then flushes cache.  */
#define UHARDDOOM_COLCMD_TYPE_COLSEM			0x9
#define UHARDDOOM_COLCMD_DATA_COL_SETUP(x, ulog, col_en, cmap_b_en)	((x) | (ulog) << 8 | (col_en) << 13 | (cmap_b_en) << 14)
/* The column X coord in the block.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_COL_SETUP_X(cmd)	((cmd) & 0x3f)
/* U coord mask.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_COL_SETUP_ULOG(cmd)	((cmd) >> 8 & 0x1f)
/* Column enable — if unset, this column will be disabled and skipped in
 * blocks sent to SWR.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_COL_SETUP_COL_EN(cmd)	((cmd) >> 13 & 1)
/* Colormap B enable.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_COL_SETUP_CMAP_B_EN(cmd)	((cmd) >> 14 & 1)
/* If enabled, U is mapped to y coord within the source (ie. multiplied by
 * source pitch), otherwise x.  */
#define UHARDDOOM_COLCMD_DATA_DRAW(len, cmap_a_en)	((len) | (cmap_a_en) << 16)
/* Number of blocks to be drawn.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_DRAW_LENGTH(cmd)	((cmd) & 0xffff)
/* Colormap A enable.  */
#define UHARDDOOM_COLCMD_DATA_EXTR_DRAW_CMAP_A_EN(cmd)	((cmd) >> 16 & 1)

/* Section 7.4: FXCMD — FX unit internal commands.  */

#define UHARDDOOM_FXCMD_TYPE_NOP			0x0
/* Loads 4 blocks from FXIN as the colormap.  */
#define UHARDDOOM_FXCMD_TYPE_LOAD_CMAP			0x1
/* Loads 1 block from FXIN to the buffer.  */
#define UHARDDOOM_FXCMD_TYPE_LOAD_BLOCK			0x2
/* Loads 2 blocks from FXIN to the buffer.  */
#define UHARDDOOM_FXCMD_TYPE_LOAD_FUZZ			0x3
/* Fills the whole buffer with a single color.  */
#define UHARDDOOM_FXCMD_TYPE_FILL_COLOR			0x4
/* Sets FUZZ state for a column.  */
#define UHARDDOOM_FXCMD_TYPE_COL_SETUP			0x5
/* Sets how many columns should be masked off at start/end.  */
#define UHARDDOOM_FXCMD_TYPE_SKIP			0x6
/* Draws stuff.  */
#define UHARDDOOM_FXCMD_TYPE_DRAW			0x7
#define UHARDDOOM_FXCMD_DATA_COL_SETUP(x, pos, en)		((x) | (pos) << 8 | (en) << 15)
#define UHARDDOOM_FXCMD_DATA_EXTR_COL_SETUP_X(cmd)	((cmd) & 0x3f)
#define UHARDDOOM_FXCMD_DATA_EXTR_COL_SETUP_FUZZPOS(cmd)	((cmd) >> 8 & 0x3f)
#define UHARDDOOM_FXCMD_DATA_EXTR_COL_SETUP_ENABLE(cmd)	((cmd) >> 15 & 1)
#define UHARDDOOM_FXCMD_DATA_SKIP(b, e, a)		((b) | (e) << 8 | (a) << 16)
#define UHARDDOOM_FXCMD_DATA_EXTR_SKIP_BEGIN(cmd)	((cmd) & 0x3f)
#define UHARDDOOM_FXCMD_DATA_EXTR_SKIP_END(cmd)		((cmd) >> 8 & 0x3f)
/* If true, SKIP will be applied to all blocks; otherwise, BEGIN will
 * only be applied to the first block, and END only to the last block.  */
#define UHARDDOOM_FXCMD_DATA_EXTR_SKIP_ALWAYS(cmd)	((cmd) >> 16 & 1)
#define UHARDDOOM_FXCMD_DATA_DRAW(len, ce, fe, srd, span)	((len) | (ce) << 16 | (fe) << 17 | (srd) << 20 | (span) << 21)
/* Number of blocks to be drawn.  */
#define UHARDDOOM_FXCMD_DATA_EXTR_DRAW_LENGTH(cmd)	((cmd) & 0xffff)
/* If true, the pixels sent to SWR will be translated through the colormap.  */
#define UHARDDOOM_FXCMD_DATA_EXTR_DRAW_CMAP_EN		((cmd) >> 16 & 1)
/* Extra-special DRAW_FUZZ mode.  */
#define UHARDDOOM_FXCMD_DATA_EXTR_DRAW_FUZZ_EN		((cmd) >> 17 & 1)
/* Selects the data source.  If neither is set, just draw whatever is in
 * the buffer.  */
#define UHARDDOOM_FXCMD_DATA_EXTR_DRAW_FETCH_SRD	((cmd) >> 20 & 1)
#define UHARDDOOM_FXCMD_DATA_EXTR_DRAW_FETCH_SPAN	((cmd) >> 21 & 1)

/* Section 7.5: SWRCMD — SWR unit internal commands.  */

#define UHARDDOOM_SWRCMD_TYPE_NOP			0x0
/* The virtual base address of the TRANSMAP.  */
#define UHARDDOOM_SWRCMD_TYPE_TRANSMAP_PTR		0x1
/* The virtual base address of the destination.  */
#define UHARDDOOM_SWRCMD_TYPE_DST_PTR			0x2
/* The pitch of the destination.  */
#define UHARDDOOM_SWRCMD_TYPE_DST_PITCH			0x3
/* Draw blocks from COL or FX.  */
#define UHARDDOOM_SWRCMD_TYPE_DRAW			0x4
/* Send a signal on SRDSEM.  */
#define UHARDDOOM_SWRCMD_TYPE_SRDSEM			0x5
/* Send a signal on COLSEM.  */
#define UHARDDOOM_SWRCMD_TYPE_COLSEM			0x6
/* Send a signal on SPANSEM.  */
#define UHARDDOOM_SWRCMD_TYPE_SPANSEM			0x7
#define UHARDDOOM_SWRCMD_DATA_DRAW(len, c_en, t_en)	((len) | (c_en) << 16 | (t_en) << 17)
#define UHARDDOOM_SWRCMD_DATA_EXTR_DRAW_LENGTH(cmd)	((cmd) & 0xffff)
/* If set, draw from COL, otherwise from FX.  */
#define UHARDDOOM_SWRCMD_DATA_EXTR_DRAW_COL_EN(cmd)	((cmd) >> 16 & 1)
/* If set, enable the TRANSMAP.  */
#define UHARDDOOM_SWRCMD_DATA_EXTR_DRAW_TRANS_EN(cmd)	((cmd) >> 17 & 1)

#endif
