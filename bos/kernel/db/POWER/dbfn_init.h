/* @(#)92	1.1.1.14  src/bos/kernel/db/POWER/dbfn_init.h, sysdb, bos411, 9439C411a 9/30/94 12:57:59 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

struct func func[] = {
/* cmd label,paramter descriptions-array[4],pass-parser,function call,
                cmd description */
/* command label: this is the name of the function that is compared to the
                character string entered by the user to execute a command */
/* parameter description: first parm - number of parmaters required 0 =
                command only
                          second/fourth parms - descriptions of the
                permissible types of information that might be
                entered in these parameters. */
/* pass-parser: should the parser structure be passed to this function ? */
/* function call: a pointer to the function */
/* command description: description of the command */

{"alter  ",2,HXV|ADV|RMV,0,0,1,alter,"(a)lter - alter memory"       },
{"back   ",0,HXV,0,0,1,Back,"(b)ack - decrement the IAR"        },
{"ditto  ",0,0,0,0,0,0, "\"\" - blank repeats the last command"         },
{"break  ",0,HXV|ADV,BTV,0,1,Break,"(br)eak - set a breakpoint"     },
#ifdef _POWER_601
{"brat   ",1,HXV|ADV,0,0,1,Brat,(char *)0 },  /* Undocumented for now */
/*
                                "(brat) - set a bratpoint"     },
*/
#endif /* POWER_601 */
{"breaks ",0,0,0,0,0,display_watch_brat_break,
        "(breaks) - list currently set breakpoints"},
/*
        "(breaks) - list currently set watchpoints,bratpoints,breakpoints"},
*/
{"buckets",0,0,0,0,1,buckets,"(bu)ckets - display kmembucket structures"  },
{"clear  ",0,HXV|ADV|ASV|WTC|BRT,BTV,0,1,Clear,
        "(c)lear - clear breakpoint(s)" },
/*
        "(c)lear - clear watch,brat,breakpoint(s)" },
*/
#ifdef _POWER_MP
{"cpu    ",0,DCV,0,0,1,Cpu,"(cpu) - change/list cpu debugging state"},
#endif /* POWER_MP */
{"display",1,HXV|ADV|RMV,HXV,0,1,Display,
        "(d)isplay - display a specified amount of memory"},
{"dmodsw ",0,0,0,0,1,dmodsw,"(dm)odsw - display Streams dmodsw table"},
{"drivers",0,0,0,0,1,drivers,
	"(dr)ivers - display device driver (devsw) table"  },
{"find   ",0,0,ASV|HXV|ADV,ADV|ASV|HXV,1,Find,
        "(f)ind - find a string in memory"      },
#ifdef _POWER
{"float  ",0,0,0,0,1,fpregs,"(fl)oat - display floating point registers"        },
#endif /* _POWER */
{"fmodsw ",0,0,0,0,1,fmodsw,"(fm)odsw - display Streams fmodsw table"},
{"go     ",0,0,0,0,1,Go,"(g)o - start executing the program"          },
{"help   ",0,0,0,0,0,Help,"(h)elp - display the list of valid commands"},
{"loop   ",0,DCV,0,0,1,Loop,
        "(l)oop - execute until control returns to this point"  },
{"map    ",0,0,0,0,1,display_map,"(m)ap - display the system loadlist"  },
{"mblk   ",0,HXV|ADV,0,0,1,mblk,"(mb)lk - display mblk/kmemstat structures"  },
{"next   ",0,HXV,0,0,1,Next,"(n)ext - increment the IAR"        },
{"origin ",1,HXV,0,0,1,Origin,"(o)rigin - set the origin"       },
{"proc   ",0,0,0,0,1,Proc,"(p)roc - process table display"      },
#ifdef _POWER_MP
{"ppd    ",0,DCV,0,0,1,Ppd,"(ppd) - display per processor data area"      },
#endif /* POWER_MP */
{"quit   ",0,0,0,0,1,Quit,"(q)uit - end the debugger session"   },
{"queue  ",1,HXV|ADV,0,0,1,strqueue,"(que)ue - display Streams queues"  },
{"reset  ",1,0,0,0,1,reset_var,"(r)eset - release a user defined variable" },
{"restore",0,YNV,0,0,1,Restore,
        "(re)store - restore or do not restore the screen"      },
{"screen ",0,0,0,0,1,Screen,
        "(s)creen - display a screen containing registers and memory"},
{"set    ",2,0,STV|HXV|DCV,0,1,Set,"(se)t - define an/or set a variable"},
{"sregs  ",0,0,0,0,1,Sregs,"(sr)egs - display segment registers"        },
#ifdef _THREADS
{"ss     ",0,0,0,0,1,stack_traceback,(char *)0},  /* Undocumented command */
#endif
{"st     ",2,HXV|ADV,HXV,0,1,St,"(st) - store a full word into memory"      },
{"stack  ",0,0,0,0,1,fmts,"(sta)ck - formatted stack trace"     },
{"stc    ",2,HXV|ADV,HXV,0,1,Stc,"(stc) - store one byte into memory"       },
{"step   ",0,0,0,0,1,Step,"(ste)p - perform an instruction single-step" },
{"sth    ",2,HXV|ADV,HXV,0,1,Sth,"(sth) - store a half word into memory"    },
{"stream ",0,HXV|ADV,0,0,1,Stream,"(str)eam - display Stream head structures"  },
{"swap   ",0,0,0,0,1,Swap,
        "(sw)ap - switch from the current display/keyboard to RS-232 port"},

#ifdef _THREADS
{"thread ",0,0,0,0,1,Thread,"(th)read - thread table display"	},
#endif /* THREADS */
{"trace  ",0,0,0,0,1,trace_disp,"(tr)ace - print traceback buffer"      },
{"trb    ",0,0,0,0,0,trbdb,"(trb) - display formatted timer request block info"},
{"tty    ",0,0,0,0,1,tty_dump, "(tt)y - Display tty struct"},
{"user   ",0,HXV|ADV|ASV,0,0,1,Fmtu,"(u)ser - formatted user area"      },
#ifdef _THREADS
{"uthread",0,HXV|ADV,0,0,1,Fmtut,"(ut)hread - formatted uthread area"	},
#endif
{"vars   ",0,0,0,0,0,list_vars,
        "(v)ars - display a listing of the user_defined variables"},
{"vmm    ",0,0,0,0,1,pr_vmm,
        "vmm - display virtual memory data structures"},
#if defined (_POWER_RS2) || defined (_POWER_601)
{"watch  ",1,HXV|ADV|LSV,HXV|ADV,0,1,Watch,(char *)0 },
/*
                                           "(w)atch - set a watchpoint"     },
*/
#endif /* _POWER_RS2 or_POWER_601 */
{"xlate  ",1,HXV,0,0,1,Xlate,
        "(x)late - display the real address of a memory location"},
#ifdef _POWER_MP
{"switch ",0,HXV|ADV,0,0,1,Switch,(char *) 0        },
{"lb     ",0,HXV|ADV,BTV,0,1,LBreak, (char *)0},
#endif /* POWER_MP */
{"z",1,HXV|RMV,HXV,0,1,Bdisplay, (char *)0},
#ifdef _POWER
#ifdef DEBUG
{"bpr    ",0,0,0,0,1,bpr, (char *)0},
/* undocumented function "bpr - set an address in the breakpoint register"}, */
#endif /* DEBUG */
#endif /* _POWER */
{"un     ",1,HXV|ADV|RMV,DCV,0,1,Udisplay, (char *)0},
{"reason ",0,0,0,0,0,Reason, (char *)0},
{"sysinfo",0,0,0,0,0,Sysinfo, (char *)0},
{"inpcb  ",1,HXV|ADV,0,0,1,prinpcb, (char *)0},
{"mbuf   ",1,HXV|ADV,0,0,1,prmbuf, (char *)0},
{"ndd    ",0,HXV|ADV,0,0,1,prndd, (char *)0},
{"socket ",1,HXV|ADV,0,0,1,showsock, (char *)0},
{"tcpcb  ",1,HXV|ADV,0,0,1,prtcpcb, (char *)0},
{" ",0,0,0,0,0,0," "    },      /* has to be here - " " marks end of array */
    };
