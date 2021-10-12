# @(#)42        1.2  src/bos/kernel/ml/POWER/memcheck.m4, sysml, bos411, 9428A410j 12/20/93 12:59:57
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

# -----------------------------------------------------------------------------
# Program:  memcheck.m4
#
# Purpose:  To make sure that there is enough contiguous good memory at the
#           beginning of main memory to load the AIX kernel.
#
# Method:   At IPL, the ROS code checks all real memory and builds a bit map
#           showing the results in the IPL control block.  Each bit in the bit
#           map represents a block of memory (16K is the default size of the
#           block).  The bit values are 0 for good memory and 1 for bad.
#           First, memcheck.m4 calculates the size of the kernel.  Then,
#           memcheck.m4 calculates the number of bits in the bit map needed to
#           cover the kernel memory size.  memcheck.m4 scans the bit map
#           counting good bits (0) while watching for a bad bit (1).  If a bad
#           bit is found before enough good bits have gone by, code 559 is
#           displayed in the LED and the code goes into an unending loop.  If
#           enough good bits have been found, kernel initialization continues
#           (back in start.m4).
#
# On entry: r3  - a(iplcb)
#           r13 - base register for relative references within start.m4 and
#                 memcheck.m4
#
# Changes:  20 Dec 93
#           Removed the SNOOPY, PEGASUS, and PowerPC LED code.  The memcheck.m4
#           routine allows newer, larger (>1MB) AIX kernels to run on the older
#           platforms (RIOS 1 and Salmon) whose ROSes only check for 1MB of
#           contiguous memory.  The newer machines (e.g., RIOS 2 and PowerPC)
#           have ROSes that check for 2MB so memcheck.m4 will find
#           non-contiguous memory only on the original Power machines.  So
#           keeping up with the changing PEGASUS, etc. code is unnecessary.
#           Stated another way, the memcheck.m4 routine will find a memory
#           error only on the RIOS 1 and Salmon machines so its LED code is
#           now just for those mahcines.
# -----------------------------------------------------------------------------

.machine "com"

        .file   "memcheck.m4"

# -----------------------------------------------------------------------------
# Load the iplcb dir and iplcb info addresses.
#
# r6  - a(iplcb dir)
# r7  - a(iplcb info)
# -----------------------------------------------------------------------------

        ai      r6, r3, ipldir          # r6 <- a(iplcb dir)
        l       r7, iplinfoo(r6)        # r7 <- o(iplcb info) from iplcb
        a       r7, r3, r7              # r7 <- a(iplcb info)

# -----------------------------------------------------------------------------
# Find the number of RAM bytes covered by each bit map bit.
#
# r9  - iplcb info bytes-per-bit
# -----------------------------------------------------------------------------

        l       r9, bytesperbit(r7)     # r9 <- c(iplcb info bytes-per-bit)

# -----------------------------------------------------------------------------
# Compute the number of bit map bits needed for the kernel.
#
# r10 - number of bit map bits for the kernel
# -----------------------------------------------------------------------------

        l       r12, kernel_size(r13)   # r12 <- size of kernel from the
                                        #   address of the last toc entry
        lil     r10, 0                  # r10 <- 0
                                        #   = initial count of bit map bits
                                        #     to cover kernel size

mc_loop02:

        sf      r12, r9, r12            # r12 <- r12 - r9
                                        #   Decrement the kernel size by one
                                        #   bit map bit's worth of coverage
        ai      r10, r10, 1             # r10 <- r10 + 1
                                        #   = count of bit map bits
                                        #     to cover kernel size
        cmpi    cr0, r12, 0             # Compare r12 to 0
        bgt     mc_loop02               # Continue loop if r12 > 0

# -----------------------------------------------------------------------------
# Get the bit map address.
#
# r8  - bit map address
# -----------------------------------------------------------------------------

        l       r8, rammapo(r6)         # r8 <- o(bit map) from iplcb
        a       r8, r3, r8              # r8 <- a(bit map)

# -----------------------------------------------------------------------------
# Check a kernel's worth of the bit map bits to see that the memory is good.
# The bit map bits are zero for good memory.
#
# r11 - bit map word
# -----------------------------------------------------------------------------

mc_loop01:

        l       r11, 0(r8)              # r11 <- c(bit map word)
        cntlz   r14, r11                # r14 <- count of leading zeros in r11
        sf.     r10, r14, r10           # r10 <- r10 - r14
                                        #   Reduce kernel-coverage bit count
                                        #   by number of good memory bits in
                                        #   this bit map word
        ble     mc_good                 # Branch if enough good memory bits
                                        #   have been found
        cmpi    cr0, r11, 0             # Is bit map word all zero bits?
        bne     mc_bad                  # Branch if bit map word != 0
        ai      r8, r8, 4               # r8 <- r8 + 4
                                        #   increment to next bit map word
        b       mc_loop01               # Continue checking bit map words

# -----------------------------------------------------------------------------
# There is bad memory where the kernel needs to go.
# -----------------------------------------------------------------------------

mc_bad:

# -----------------------------------------------------------------------------
# LED code beginning - lifted from start.m4 then changed to support just
#                      the older platforms (RIOS 1 and Salmon)
# -----------------------------------------------------------------------------

#       Determine machine model.
#       r17: machine model

        l       r17,MODEL(r7)           # model field in ipl_info structure

#       Put bad memory code (559) in LED display

        andiu.  r5,r17,PPC_MODEL        # test if PPC model
        cau     r7, 0, 0x5590           # 559 in high end of reg
        bne     cr0,mc_stop             # not an original Power model

#       Begin POWER specific LED code

        cau     r5, 0, u.nv_ram_seg_pwr # Get value for seg reg
        oril    r5, r5, l.nv_ram_seg_pwr
        mtsr    r_segr15, r5            # Load segment register 15
        cau     r8, 0, u.leds_addr_pwr  # Construct address of LEDs

        mfmsr   r5                      # Get current (non-reloc) MSR
        oril    r6, r5, MSR_DR          # Set Data Relocate bit = 1
        mtmsr   r6                      # Turn on Data Relocate
        isync                           # make sure DR is noticed
        st      r7, l.leds_addr(r8)     # Write 559 to LEDs
        mtmsr   r5                      # Turn off Data Relocate again
        xor     r5,r5,r5                # zero r5
        mtsr    r_segr15, r5            # zero SR15 for safety sake
        isync                           # make sure DR is noticed
        sync                            # sync so LEDs are seen

# -----------------------------------------------------------------------------
# LED code end - lifted from start.m4
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Not enough good memory for the kernel to be loaded so stop here.
# -----------------------------------------------------------------------------

mc_stop:

        b       mc_stop                 # Tight loop

# -----------------------------------------------------------------------------
# Size of the kernel.
# -----------------------------------------------------------------------------

        .extern DATA(lasttocentry)

kernel_size:
        .long   DATA(lasttocentry)      # lasttocentry comes from last.s and
                                        # marks the end of the kernel

# -----------------------------------------------------------------------------
# There is enough contiguous good memory for the kernel.
# -----------------------------------------------------------------------------

mc_good:

