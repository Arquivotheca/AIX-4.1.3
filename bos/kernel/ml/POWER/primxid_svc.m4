# @(#)27      1.9  src/bos/kernel/ml/POWER/utrc_svc.m4, bos, bos410 3/3/94 08:31:20
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#******************************************************************************
#
# NAME: primxid_sc
#
# FUNCTION: SVC handler for setting primxid display
#
# INPUT:
#	kernel mode
#	translation on
#	interrupts disabled
#	LR = return address to user-level code
#	CTR bits 16-31 = caller's msr
#	r1 = caller's stack pointer
#	r2 (user's TOC) has be saved by glue code (and will be restored by it)
#	r3 = new ear value
#	r0 = g_kxsrval
#	r11 = user sr2 value
#	r12  = g_ksrval
#
# OUTPUT:
#       Success: returns 0 and
#	         the RID field of EAR register is set up with the input RID
#                and the corresponding display becomes the primary xid display 
#		 for the calling thread
#	
#	Fail:    returns -1 if the calling thread has not registered to access the
#                display associated with RID(input ear)
#
# NOTES:
#	This code must reside in non-privileged memory (read access with key 1)
#	since it executes a few instructions at the beginning and some at the
#	end using the user-level segment register 0.
#
#	This routine and routines it calls are run with interrupts disabled.
#	This simplifies the logic somewhat but care should be taken to
#	minimize path length to avoid an impact to real-time performance.
#
# register usage:
# 	r4 = saved user msr
# 	r5 = ptr to gruprt structure in the list
# 	r8 = ptr to the gruprt structure for the current primary xid display
# 	r7 = new ear value
# 	r9 = ptr to the gruprt structure for the new primary xid display
# 	r12 = ptr to the current thread
# 	EQ(cr6) == 0 iff the current primxid is found
# 	EQ(cr7) == 0 iff the target primxid is found
#
#
# 	int primxid_sc(newear)
# 	{
#	    /* return -1 if running in POWER arch
#            */
#	    if (!__power_pc()) 
#	    {
#		r3 = -1;
#		ctr = mfctr();
#	        mtmsr(ctr);
#		return;
#	    }
#
#	    /* set up kernel addressibility
#            */
#	    mtsr0(r12);
#	    mtsr2(r11 & ~SR_KsKp);
#	    mtsr14(r0);
#	    isync;
#
#    	    EQ(cr6) = EQ(cr7) = 1;
#
#    	    /* find gruprt structures for the current primary xid display and new
#      	       primary xid display of the running thread
#     	     */
#    	    for (gp = curthread->t_graphics; gp; gp = gp->gp_next)
#           {
#                if (gp->gp_type == _BUSPRT_7F_XID || 
#		     gp->gp_type == _BUSPRT_BAT_XID)
#         	 {
#             	     if (EQ(cr6) && GRUPRT_PRIMXID(gp))
#                    {
#                        currentp = gp;
#	                 EQ(cr6)  = 0;
#                 	 if (EQ(cr7) == 0)  break;
#                    }
#
#             	     if (EQ(cr7) && (GRUPRT_EAR(gp) & EAR_DISABLE_MASK) 
#			             == (newear & EAR_DISABLE_MASK))
#                    {
#                 	 newp     = gp;
#			 EQ(cr7)  = 0;
#                 	 if (EQ(cr6) == 0)  break;
#                    }
#                }     
#           }
#
#           if (EQ(cr7) == 0)
#           {
#               /* reset primxid field in the gruprt structure of the current
#                  primary xid display
#                */
#               if (EQ(cr6) == 0)
#               {
#                   GRUPRT_PRIMXID(currentp) = 0;
#               }
#
#               EAR register = GRUPRT_EAR(newp);
#               /* make it the new primary xid display of this thread */
#               GRUPRT_PRIMXID(newp) = 1;
#               rc = 0;
#           }
#           else    rc = -1;
#
#           /* restore user mode segment registers and msr 
#            */	
#	    isync;
#	    mtsr0(u.u_adspace.sr[0]);
#	    mtsr2(u.u_adspace.sr[2]);
#	    mtsr14(u.u_adspace.sr[14]);
#           ctr = mfctr();
#	    mtmsr(ctr);
#	    isync
#	    return(rc);
#       }
#
#*******************************************************************************

	.machine "com"
ENTRY(primxid_sc):				# for mtrace tool
primxid_sc:
	l       r4,syscfg_arch(0)		# architecture field
        cmpi    cr0,r4,POWER_PC 		# check the architecture
        beq+    cr0,primxid_ppc                 # branch if ppc 
	lil	r3,-1 
	mfctr	r4				# r4 = user msr
	mtmsr	r4				# back to problem state
	br                                      # return -1 if power arch

primxid_ppc:
	mtsr	sr0, r12			# load kernel segreg
	rlinm	r11, r11, 0, ~SR_KsKp		# clear user key bit
	mtsr	sr14, r0			# load kernel extension seg
	mtsr	sr2, r11			# load new proc-private seg
	isync					# wait for seg reg loads

	GET_PPDA(cr0, r11)
	mfctr	r4				# r4 = user msr
	l	r12,ppda_curthread(r11)		# r12 = curthread pointer
	lil	r10,0x0022
	l	r5,t_graphics(r12)              # r5 = first gruprt structure
	mtcrf	0x03,r10			# EQ(cr6) = EQ(cr7) = 1
	cmpi	cr0,r5,0
	rlinm	r3,r3,0,0x3F			# r3 = input RID
	beq-	cr0,fail_exit	
	lil	r10,0x0

prim_loop:
	lbz	r6,gp_type(r5)			# r6 = type value
	cmpli	cr1,r6,XID7FTYPE	                
	cmpli	cr0,r6,XIDBATTYPE
	cror	cr1+eq, cr0+eq, 4*cr1+eq        # combine tests
	bne	cr1,next_gruprt

	# r5 ptr to a XID type gruprt structure	
	bne	cr6,search_target

	# look for the current primxid
	lbz	r6,gp_primxid(r5)		# r6 = primxid indicator
	cmpli	cr1,r6,0		
	beq	cr1,current_not_found

	# the gruprt structure for the current primxid is found
	mtcrf	0x02,r10			# set EQ(cr6) = 0	
	mr	r8,r5				# r8 = ptr to the current primxid
	bne	cr7,loop_brk			
	b	search_target

current_not_found:
	bne	cr7,next_gruprt

	# search for the gruprt structure for the new primxid
search_target:
	beq	cr0,bat_xid			# cr0 is preserved
	l	r7,gp_acw1(r5)			# r7 = ear from a 7FXIDTYPE gruprt
	b	check_batxid
bat_xid:
	l	r7,gp_acw2(r5)			# r7 = ear from a BATXIDTYPE gruprt
check_batxid:
	rlinm	r6,r7,0,0x3F			# r6 = RID
	cmp	cr1,r3,r6			
	bne	cr1,next_gruprt

	# the gruprt structure for the new primxid is found	
	mtcrf	0x01,r10			# set EQ(cr7) = 0
	mr	r9,r5				# r9 = ptr to the new primxid
	bne	cr6,loop_brk

next_gruprt:
	lwz	r5,gp_next(r5)			# r5 = prt to next gruprt in list
	cmpi	cr1,r5,0			# test if done
	bne+    cr1,prim_loop			# go check the next gruprt

loop_brk:
	beq-	cr7,fail_exit
	lil	r3,0
	beq-	cr6,no_current
	stb	r3,gp_primxid(r8)		# reset the primary indicator

no_current:
	lil	r5,1
	.machine "any"
	mtear	r7				# set the ear
	.machine "com"
	stb	r5,gp_primxid(r9)	        # set the primary indicator

exit_point:	
	l	r11, t_userp(r12)		# user structure
	l	r8, u_adspace_sr+0*4(r11)	# get user segment 0
	l	r9, u_adspace_sr+2*4(r11)	# get user segment 2
	l	r10, u_adspace_sr+14*4(r11)	# get user segment 14
	isync
	mtsr	sr0, r8				# load segregs with problem
	mtsr	sr2, r9				# state values
	mtsr	sr14, r10
	mtmsr	r4				# back to problem state
	isync
	br					# return to application

fail_exit:
	lil	r3,-1
	b	exit_point

	.globl ENTRY(primxid_sc)
