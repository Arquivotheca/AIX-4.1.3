# @(#)88	1.26.1.11  src/bos/kernel/ml/user.m4, sysml, bos411, 9431A411a 8/2/94 11:33:36
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

include(uthread.m4)

        .dsect  user

#       miscellaneous
u_uthread_cb:	.space	32
u_cancel_cb:    .space  32
u_procp:        .long   0
u_handy_lock:	.long	0

#       signal management
u_signal:       .space  64*4
u_sigmask:      .space  64*8
u_sigflags:     .space  64*1

#       user-mode address space mapping
u_adspace:
u_adspace_alloc:   .long   0
u_adspace_sr:      .space  16*4
u_segst:           .space  16*2*4
u_lock_word:       .long   0
u_vmm_lock_wait:   .long   0

# 	audit field for syscall handler (svc.s)
u_auditstat:	.long 0

		.space 0x2E8

#	user file descriptor table
u_maxofile:	.short 0,0
u_fd_lock:	.long 0
u_ufd_fp:	.long 0
u_ufd_flags:	.short 0
u_ufd_count:	.short 0

	.dsect	ublock

ub_uthr0:	.space	ut_end-uthread
ub_user:	.space	0
