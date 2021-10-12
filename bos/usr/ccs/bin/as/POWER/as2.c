static char sccsid[] = "@(#)94	1.20  src/bos/usr/ccs/bin/as/POWER/as2.c, cmdas, bos41J, 9523A_all 6/2/95 14:01:30";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: bfmt, checka, checkabs, cvtmask, dfmt, ins1out,
 *	      ins2out, mfmt, scfmt, xflfmt, xfmt, xfxfmt,
 *            checkbo, checkbo2, instrs_warn.
 *
 * ORIGINS:  3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	"as.h"
#include        <stdarg.h>
/****************************************************************/
/* defines for argument access					*/
/****************************************************************/
#define ARG1	(ap->xp->xvalue)
#define ARG2	((ap+1)->xp->xvalue)
#define ARG3	((ap+2)->xp->xvalue)
#define ARG4	((ap+3)->xp->xvalue)
#define ARG5	((ap+4)->xp->xvalue)


/****************************************************************/
/* Function:	ins1out						*/
/* Purpose:	process instructions from the source line 	*/
/* 		during pass 1					*/
/* Input:	op   - the table entry for the instruction 	*/
/*		ap   - the arguments for the instruction	*/
/*		nact - number of actual arguments		*/
/* Comments:	the main function of this routin is to check	*/
/* 		the number of arguments, and increment the  	*/
/*		current location counter			*/
/****************************************************************/
ins1out(op, ap, nact)
 register struct instab *op;
 register struct arg *ap;
{
int minexp,maxexp;
#ifdef IDEBUG
int n;
struct arg *a;
#endif

	if (ccsect->dot & 1) {
                                                 /* message 081 */
		yyerror( 81 );
		ccsect->dot++;
	}

	/* set minimum and maximum number of args expected	*/
	/* check special cases					*/
	if (!strcmp(op->ifmt,"n")) 	/* n stands for NO args	*/
	   minexp = maxexp = 0;
	else if (!strcmp(op->ifmt,"1")) { /* 1 = missing optional*/ 
	   maxexp = strlen(op->ifmt2);	  /* argument,  value 0	*/
	   minexp = 0;
	   }
	else {
	   maxexp = strlen(op->ifmt);
	   if (!(minexp = strlen(op->ifmt2)))
		minexp = maxexp;
	   }

	/* check number of arguments				*/
	if (nact<minexp)
                                                 /* message 082 */
		yyerror( 82 );
	if (nact>maxexp) {
                                                 /* message 083 */
		yyerror( 83 );
		nact=maxexp;
	}

#ifdef IDEBUG
	if (indbg("ins1out")) {
	   printf("ins1out:%lx: %lx 10%s\n",ccsect->dot,
			op->opcode,op->name);
	a = ap; 	/* a points to the current arg		*/
        n = 1;
         while (n<=maxexp) {
           printf("ins1out: argument value: %d name: %s\n", 
		a->xp->_xydu._xvalue,a->xp->xname->name);
           a++;n++;
         }
        }
#endif

	ccsect->dot += 4; 	/* increment location counter */
}

/****************************************************************
*  Function: checkbo
*  purpose: check the BO field value to see if it contains some
*           value that causes an "invalid form".
*           The check only against the PowerPC mode. If the assembly
*           mode is default, then warning will be reported but the
*           assembling is not terminated.
*  Input:   bo_value - contais BO field value
****************************************************************/
checkbo( unsigned short bo_value) /* BO field value */
{
   /* for 32-bit mode and 5-bit BO field, the invalid form values:
        6, 7, 14, 15 or all the values above 20   */

    if ( bo_value == 6 || bo_value == 7 ||
           bo_value == 14 || bo_value == 15 || bo_value > 20 )
           instrs_warn (150, bo_value);
}

/****************************************************************
*  Function: checkbo2
*  purpose: check the third bit of the BO field value to see if
*           it is zero. An error will be reported when it is zero.
*           The check only against the PowerPC mode. If the assembly
*           mode is default, then warning will be reported but the
*           assembling is not terminated.
*  Input:   bo_value  - contains BO field value
*****************************************************************/
checkbo2(unsigned short bo_value)
{
    if (!(bo_value & 0x0004 ) )
       instrs_warn (163,bo_value);
}

/****************************************************************/
/* Function:	checka						*/
/* Purpose:	check adress arguments for valid type and range */
/* Input:	x        - expression which is address		*/
/* 		addrtype - tells whether it is relative or abs	*/
/*		           address				*/
/*		bitlen   - number of bits of address		*/
/* Returns:	the expression with the correct address value	*/
/*		0 - if error					*/
/* Comments:	This routine outputs the appropriate  		*/
/*		RLDs for the branch address			*/
/****************************************************************/
struct exp *
checka(x,addrtype,bitlen)
struct exp *x;
int addrtype,bitlen;
{
int addr;
int dot;

	if (bitlen==16) {
	   dot = ccsect->dot;
	   ccsect->dot += 2;
	   }
	else dot = ccsect->dot;
	addr = 0;

	/* only absolute branches can have absolute expressions	*/
	if (x->xtype == E_ABS) {
  	   if (addrtype==AIDDR)
 	      addr = x->xvalue;
   	   else 
                                                 /* message 086 */
	      yyerror( 86 );
	   }
	else {
	   /* if address type is relative, then the value is either */
	   /* the displacement from here to there, or just minus */
	   /* here if the expression is external		*/
	   if (addrtype==AADDR) {
	      if (x->xtype==E_REL) {
		 addr = x->xvalue-(dot+ccsect->start);
		 /* only output an RLD if the label is in a	*/
		 /* different csect				*/
	         if (x->xloc != ccsect) {
		      addr = (x->xvalue+x->xloc->start) -
			     (dot+ccsect->start);
		      addrld(R_RBR,bitlen,x->xname);
		      }
		 else addr = x->xvalue-dot;
		}
	      else if (x->xtype==E_EXT) {
/*		 if (x->xname->etype==XTY_ER)     
**		    addr = -(dot+ccsect->start); 
**   The above two lines are deleted for defect 82455.  
**    E_EXT +/- E_ABS = E_EXT.  In that case, x->xvalue should be used. */ 
   
                 addr = x->xvalue - (dot+ccsect->start);

		/* if not in same csect, output an RLD, else assume 	*/
		/* that the address is relative				*/
		if (x->xloc!=ccsect)
		 	addrld(R_RBR,bitlen,x->xname);
		 }
                                                 /* message 087 */
	      else yyerror( 87 );
	      }
	   /* here the address type is absolute, which means	*/
	   /* that if the expression is relative, just output   */
	   /* the value of the symbol and if it is external put */
	   /* in a zero, both cases require an RLD		*/
           else {   	/* addrtype == AIDDR */
	      if (x->xtype==E_REL) {
		 addr = x->xvalue+x->xloc->start;
		 if (x->xname==NULL)		/* assume it is a $ */
		    addrld(R_RBA,bitlen,x->xloc->secname);
		 else addrld(R_RBA,bitlen,x->xname);
		}
	      else if (x->xtype==E_EXT) {
		 addr = x->xvalue; 
		 addrld(R_RBA,bitlen,x->xname);
		 }
                                                 /* message 087 */
	      else yyerror( 87 );
	      }
	   }

#ifdef IDEBUG
	if (indbg("checka")) 
	   printf("addr: %d minval: %d maxval: %d bitlen: %d\n",addr,
               -(0x1<<bitlen-1),(0x1<<bitlen-1),bitlen);
#endif

	/* check to make sure the computed value is in range	*/
        if (addr < -(0x1<<bitlen-1) || addr >= (0x1<<bitlen-1)) { 
                                                 /* message 088 */
	   yyerror( 88 );
	   addr = 0;
	   }			/* --- below checks boundry ----*/
	else if( 0x3 & addr)  
                                                 /* message 080 */
	   yyerror( 80 );

	ccsect->dot = dot;	/* restore ccsect->dot value */
	x->xvalue = addr;
	return x;
}

/****************************************************************/
/* Function:	ins2out						*/
/* Purpose:	process instructions from the source line 	*/
/* 		during pass 2 called by as0.y			*/
/* Input:	op   - the table entry for the instruction 	*/
/*		ap   - the arguments for the instruction	*/
/*		nact - number of actual arguments		*/
/* Comments:	this routine generates the code for the instruc-*/
/*		tions.   This routine has two parts, the first	*/
/* 		part checks the input arguments, and the second	*/
/* 		part generates each of the different format	*/
/* 		instructions					*/
/* 		The instructions are described in a file which	*/
/*		is used to generate the actual instruction table*/
/*		the instructions are described in terms of 	*/
/*		the mnemonic, input format 1, input format 2,   */
/*		the output format (actual instruction format)	*/
/*		and an instruction skeleton to be filled in with*/
/*		the processed arguments (file called amer.ops)	*/
/****************************************************************/
ins2out(op, ap, nact)
register struct instab *op;
register struct arg *ap;
int nact;
{
int instr,n, sum_34, diff_34;
char *ifmt; 
struct exp *x,*a3;
struct arg *a, *temp_ptr;
unsigned char xregs;
int i2,i4,i5;
int f2 = 0;
int errbit;
struct exp xexp;
long temp, t1, t2;

	/* start with the instruction skeleton			*/
	instr = op->opcode;		
				
	/* first look at the number of actual arguments and the	*/
	/* to determine which input format to use, ifmt or ifmt2*/
	/* keep the comparison in this order			*/
	if ((n=strlen(op->ifmt2)) && n==nact) {
           f2 = 1;
	   ifmt = op->ifmt2;
        } 
	else ifmt = op->ifmt;

	/* init the following fields so that we know they have 	*/
	/* not been set						*/
	i2 = i4 = i5 = -1;

#ifdef IDEBUG
	if (indbg("ins2out")) 
	   printf("ins2out:%lx: %lx 10%s\n",ccsect->dot,instr,op->name);
#endif
	/* the input format is represented as a string where 	*/
	/* character in the string is an argument type which 	*/
	/* corresponds to an actual argument.  By looking at each*/
	/* argument and each corresponding character in the input*/
	/* format string, the arguments can be range checked and*/
	/* converted to the proper form				*/
	/* this loop goes through the input format string, and	*/
	/* sets up the arguments for the next part which blindly*/
	/* takes the arguments and generates the instructions	*/
	n = 1; 		/* n is the number of the current arg*/
	a = ap; 	/* a points to the current arg		*/
	while (*ifmt) {
	   x = a->xp;
#ifdef IDEBUG
	if (indbg("ins2out")) 
	   printf("argument %d value %d name %s\n",n,x->_xydu._xvalue,
			x->xname->name);
#endif
          
           if ( a->atype == ADISP && 
               ( *ifmt != 's'  && *ifmt != 'A' && *ifmt != 'B'
                  && *ifmt != 'C' && *ifmt != '3' ))
                yywarn (159, n);

           switch (*ifmt) {
                case 'a':               /* 26 bit abs addr (rel)*/
                        a3 = checka(x,AIDDR,26);
                        break;

                case 'b':               /* 1-bit data. for cmp, cmpi,   */
                                        /*  cmpli and cmpl insts.        */
                      if ( !( asm_mode == ANY && modeopt ))
                        if ( asm_mode_type == PowerPC)
                              /* PowerPC requires the L field in the compare */
                              /* inst. to be zero for 32-bit implementation */
                           if ( x->xvalue != 0 )
                              instrs_warn(154);
                        break;
		case 'c':		/* 5-bit cond code	*/
			i2 = x->xvalue;
		case 'g':		/* 5-bit sp purpose reg */
		case 't':		/* 5-bit immediate value */	
					/* also 5-bit TO mask	*/
				        /* these are the mask 	*/ 
			    		/* indexes		*/
		case 'r':		/* 5-bit register number */
			checkabs(x,31,0,n);
			break;
		case 'e':		/* segment reg		*/
		case 'h':		/* 4-bit value		*/
			checkabs(x,15,0,n);
			break;
		case 'f': 		/* 3 bit CR field	*/
			checkabs(x,7,0,n);
			x->xvalue= x->xvalue << 2;
			i2 = x->xvalue;
			break;
                case 'I':               /* for addis and lis   */
                        if (x->xtype != E_ABS )
                           yyerror ( 104, n);
                        if (x->xvalue < -65536 || x->xvalue > 65535)
                           yyerror(107, n);
                        else if ( x->xvalue >= 32768 ||
                                      x->xvalue < -32768 )
                           yywarn1(173, op->name, x->xvalue);
                        break;
                case 'i':               /* 16-bit signed immediate*/
                           checkabs(x,32767,-32768,n);
                        break;
                case 'j':               /* 7-bit field          */
                        checkabs(x,127,0,n);
                        break;
		case 'k':		/* 3-bit field		*/
			checkabs(x,7,0,n);
			break;
                case 'l':               /* 26 bit rel addr (rel)*/
                        a3 = checka(x,AADDR,26);
                        break;
		case 'm':		/* 5-bit immediate val	*/
			if (x->xvalue==32) x->xvalue = 0; 
			checkabs(x,31,0,n);
			break;
                case 'n':               /* no arguments         */
                        break;
		case 'o':		/* 5-bit branch operation */
                        if ( !( asm_mode == ANY && modeopt ))
                             /* Check the BO field of the Branch cond. inst. */
                             /* for PowerPC mode                  */
                          if ( asm_mode_type == PowerPC ) {
                             checkbo((unsigned short)x->xvalue);
                          }
			checkabs(x,31,0,n);
			break;
                case 'O':           /* 5-bit branch operation */
                                    /* for Branch Cond. to CTR Reg.    */
                        if ( !( asm_mode == ANY && modeopt ))
                             /* Check the BO field of the Branch cond. inst. */
                             /* for PowerPC mode                  */
                          if ( asm_mode_type == PowerPC ) {
                                  /* For Branch Cond. to CTR Reg. inst.  */
                                  /* the bit 2 of the BO field should not */
                                  /* be zero and the rest of the BO field */
                                  /* should be checked as well            */
                             checkbo2((unsigned short)x->xvalue);
                             checkbo((unsigned short)x->xvalue);
                          }
                        checkabs(x,31,0,n);
                        break;
                case 'q':               /* 4-bit immediate value to FPSCR */
                        checkabs(x,15,0,n);
                        x->xvalue= x->xvalue << 1;
                        break;
		case 's':		/* base displacement d(ra)*/
			mkdisp(a);
			break;
                case 'A':  /* base displacement d(ra). need invalid form */
                             /* check for "RA is not  be zero            */
                             /* This case for inst. : lfsu, lfdu, stbu,   */
                             /* sthu, stwu (stu in POWER ), stfsu, stfdu  */
                     mkdisp(a);
                     if ( !(asm_mode == ANY && modeopt ))
                        if ( asm_mode_type == PowerPC
                                         && asm_mode != PPC_601 ) {
                           /* when asm mode is 601, no check is needed */
                          if ( a->atype == ADISP ) {
                                 if ( a->areg == 0 )  /* if RA == 0, error */
                                     instrs_warn ( 166 );
                          }
                        }
                        break;
                case 'B':  /* base displacement d(ra). need invalid form */
                             /* check for "RA is not  be zero " and      */
                             /* RT != RA           */
                             /* This case for inst. : lbzu, lhzu, lhau,  */
                             /* lwzu ( and lu in POWER )                 */
                     mkdisp(a);
                     if ( !(asm_mode == ANY && modeopt ))
                        if ( asm_mode_type == PowerPC
                                         && asm_mode != PPC_601 ) {
                           /* when asm mode is 601, no check is needed */
                          if ( a->atype == ADISP ) {
                               if ( a->areg == 0 ) /* if RA == 0, error */
                                    instrs_warn ( 151 );
                               else {
                                   temp_ptr = --a; /* temp_ptr now points */
                                                   /* to RT               */
                                   a++;
                                   if ( a->areg == temp_ptr->xp->xvalue )
                                           /* if RT == RA, error */
                                       instrs_warn(151);
                               }
                          }
                        }
                        break;
                case 'C':  /* base displacement d(ra). need invalid form */
                             /* check for "RA is not in the range of reg.s */
                             /* to be loaded and !(RA=RT=0)                */
                             /* This case for inst. lm or lmw              */
                     mkdisp(a);
                     if ( !(asm_mode == ANY && modeopt ))
                        if ( asm_mode_type == PowerPC
                                         && asm_mode != PPC_601 ) {
                           /* when asm mode is 601, no check is needed */
                          if ( a->atype == ADISP ) {
                                   temp_ptr = --a;
                                   a++;
                                   if ( a->areg  >= temp_ptr->xp->xvalue )
                                  /* if RA is within the range of reg.s to */
                                  /* be loaded, or RA=RT=0, error         */
                                      instrs_warn(164);
                          }
                        }
                        break;

		case 'v':		/* 14 bit svc field	*/
                     if ( !(asm_mode == ANY && modeopt ))
                        if ( asm_mode_type == PowerPC ) {
                          if ( !strcmp(op->name, "svca")) {
                             if ( x->xvalue != 0 ) 
                                instrs_warn(165);
                             if ( asm_mode == COM && warning )
                                yywarn(153, op->name);
                          }
                        } 
			checkabs(x,16383,0,n);
			break;
		case 'u':		/* 16-bit unsigned imm */
			checkabs(x,65535,0,n);
			break;
		case 'w':		/* 8-bit unsigned imm */
			checkabs(x,255,0,n);
			x->xvalue = x->xvalue << 1;
			break;
		case '0': 		/* 16-bit relative disp	*/
					/* with implied 0 	*/
			i2 = 0;		/* set implied 0 argument*/
                              /* Note here no "break", so a3 value   */
                              /* is got from case 'd'                */
		case 'd':		/* 16 bit rel disp value */
			a3 = checka(x,AADDR,16);
			break;

		case '1':	/* missing optional absolute operand */
				/* value 0(zero) example bnec in XL format */
			break;
		case '2': 		/* 16-bit abs disp val	*/
					/* with implied 0 	*/
			i2 = 0;		/* set implied 0 arg	*/
                              /* Note here no "break", so a3 value   */
                              /* is got from case 'p'                */
		case 'p':	/* 16 bit abs value	(rel)	*/
			a3 = checka(x,AIDDR,16);
			break;

                case '3':  /* 16-bit signed disp value that is      */
                           /*    divisible by 4                     */
                        mkdisp(a);
                        if (x->xvalue & 0x0003 ) 
                           yyerror ( 155 );
                        break;
                case '4': /*  2-bit unsigned data ( value 0-3 ),   */
                          /*  for SPR sequence number.    */
                        checkabs(x, 3, 0, n); 
                        break;
                case '5': /* 10-bit unsigned data ( value 0-1023 )    */
                          /* for New SPRs in PowerPC and 5-bit SPR value */
                          /* for POWER. for mftb inst., the TBR value is */
                          /* either 268 or 269, otherwise, error.       */
                        if ( asm_mode == PWR || asm_mode == PWR2 )
                           checkabs(x, 31,0,n);   /* 5-bit spr value range */
                        else if ( asm_mode == COM ) {
                             if ( x->xvalue < 0 || x->xvalue > 31 ) {
                                 if ( !modeopt ) {
                                    if (!warning ) {
                                        warning = 1;
                                        yywarn(107, n);
                                        warning = 0;
                                    } else
                                        yywarn(107, n);
                                } else
                                   yyerror(107, n);
                             }
                        } else {
                          if ( !strcmp(op->name, "mftb") ) {
                              if (x->xvalue != 268 && x->xvalue !=269)
                                yyerror( 107, n); 
                          } else {       /* checking 10-bit spr value range */
                              checkabs(x, 1023, 0, n);
                          }
                        }
                        break;
                case '6': /* 5-bit immediate value (0-31), for number  */
                         /* of bits to be shifted, rotated, cleared or */
                         /* clear and shifted.                        */
                         /* i.e. rotlwi,rotrwi,slwi(sli), srwi(sri),   */
                         /* clrlwi, clrrwi and clrlslwi.               */
                       if ( x->xvalue < 0 || x->xvalue > 31)
                          yyerror(107,n);
                       break;
                case '7': /* 16-bit signed value. The value will be negated */
                          /* for subtract inst. So the upper bound will be  */
                          /* 32768 instead of 32767, and the low bound will */
                          /* be -32767 instead of -32768.                   */
                        checkabs(x,32768,-32767,n);
                        break;

                case '8': /* 5-bit immediate value (1-31), for number  */
                          /* of bits to be extracted, inserted  or      */
                          /* clear and shifted . i.e. extlwi,extrwi,    */
                          /* inslwi, and insrwi.                       */
                       if ( x->xvalue <= 0 || x->xvalue > 31)
                          yyerror(107,n);
                       break;
		case 'z':		/* optional 5-bit mask	*/
					/* used for mform	*/
			if ((errbit = cvtmask(x->xvalue,&i4,&i5))>-1)
                                                 /* message 078 */
			   yyerror( 78, errbit);
			x->xvalue = i4;
			/* put i5 in a temp local expr structure*/
			/* and point the 5th arg at it so that	*/
			/* the next step will not know (or care)*/
			/* that this was the mask form instead	*/
			/* of the two index form		*/
			xexp.xvalue = i5;
			(++a)->xp = &xexp;
			break;
		default:
			/* b - is not used for america */
			/* x - ryo is illegal		*/
			/* y - should not appear	*/
                                                 /* message 112 */
			yywarn( 112, *ifmt);
			break;
	   	}
	   a++; n++;
	   ifmt++;		/* next input format character	*/
	   }

	if (n<=nact) 
                                                 /* message 075 */
	   yyerror( 75 );
	else 	
	   /* now the input args should be set up, just generate*/
	   /* the instruction specified by the output format	*/
	   /* (ofmt) field					*/
	   switch(op->ofmt) {
		case A:			/* A format, 4 args	*/
		case AB:		/* A format, 3 args, 	*/
					/* arg3 in slot 3 	*/
		case AC:		/* arg3 in slot 4	*/
                case AD:                /* A format 2 args in   */
                                        /* slot 1 and slot 3    */ 
			instr |= (ap++)->xp->xvalue <<(32-11);
                        if (op->ofmt != AD )
			        instr |= (ap++)->xp->xvalue <<(32-16);
			if (op->ofmt!=AB && op->ofmt != AD)
				instr |= (ap++)->xp->xvalue <<(32-26);
			if (op->ofmt!=AC) 
				instr |= ap->xp->xvalue <<(32-21);
			break;
                case AE:   /* Four input operands: op1,op2,op3,op4    */	
                           /*        |op1|op2|op3|op4| EO |Rc|       */
                           /*         -----------------------        */
                           /* bit    6   11  16  21  26   31        */

                        instr |= (ap++)->xp->xvalue << (32-11);
                        instr |= (ap++)->xp->xvalue << (32-16);
                        instr |= (ap++)->xp->xvalue << (32-21);
                        instr |= ap->xp->xvalue << (32-26);
                        break;
                case DA:   /* for two input formats: 4 input operands or  */
                           /* 3 input operands.                          */
                        if (f2)
                          instr = dfmt(instr, ARG1, ARG2, ARG3);
                        else
			  instr = dfmt(instr,ARG1,ARG3,ARG4);
			break;
                case DH:   /* for two input formats: 3 input operands or  */
                           /* 2 input operands.                          */
                        if (f2)
                          instr = dfmt(instr, 0, ARG1, ARG2);
                        else
			  instr = dfmt(instr,ARG1,ARG2,ARG3);
			break;
		case DM: 		/* si   SI = -SI 	*/
		case YM:
			instr = dfmt(instr,ARG1,ARG2,-ARG3);
			break;
		case DI:		/* (RT|BF|TO),RA,SI	*/
		case DU:		/* (RT|BF),RA,UI	*/
			instr = dfmt(instr,ARG1,ARG2,ARG3);
			break;
		case DZ:		/* RA,SI		*/
			instr = dfmt(instr,0,ARG1,ARG2);
			break;
		case DS:		/* standard d(r)	*/
                case D3:                /* DS form              */
		   instr |= ((ap++)->xp->xvalue)<<(32-11);  
		   if(ap->atype == ADISP) {
			/* explict base/displacement */
			instr |= ap->areg<<(32-16);
			outh(instr>>16);      /* reg+base */
			x = ap->xp;
			switch (x->xtype) {
		   	   case E_TREL:
                                if ( x->x_rrtype != R_NEG ) {
		        	  addrld(R_TOC,16,x->xloc->secname);
                                } else  /* E_ABS - E_TREL is invalid */
                                        /* for the disp..            */
                                  yyerror(99);
                                break;
		   	   case E_REL:
			   case E_ABS:
			          break;
                           case E_REXT:  /* 75x11 */
                                   /* restricted  external, means need two */
                                   /* RLDs                      */
                                  addrld(x->rldinfo.rrtype,16,x->rldinfo.rname);
                                  addrld(x->x_rrtype,16,x->xname);
                                  break;
                           case E_EXT:
                                 if ( !x->xloc ) {
                                    if ( x->xname->etype == XTY_ER &&
                                         x->xname->eclass == XMC_TD)
                                    /* it is an enternal TD symbol  */
                                    {
                                        if ( tocname == NULL ) 
                                           yyerror ( 184, x->xname->name);
                                        addrld(R_TOC,16,x->xname);
                                        break;
                                    }
                                 } else if ( x->xloc->stype==XTY_SD && 
                                            ( x->xloc->secname->eclass==XMC_TD ||
                                              x->xloc->secname->eclass==XMC_TC )){
                                           /* It is TOC symbol, the TD or TC  */
                                           /* csect name                      */
                                        addrld(R_TOC,16,x->xloc->secname);
                                        x->xvalue -=  tocname->csect->start;
                                        break;
                                 }
                                 else if ( x->xloc->stype==XTY_CM && 
                                              x->xloc->secname->eclass==XMC_TD ){
                                           /* It is TOC symbol, the TD comm name */
                                        addrld(R_TOC,16,x->xloc->secname);
                                        x->xvalue -=  tocname->csect->start;
                                        break;
                                 }
                                 else if (x->xloc->stype==XTY_SD ||
                                             x->xloc->stype==XTY_CM) {
                                     /* a regular csect or comm name   */
                                         x->xvalue -= x->xloc->start;
                                         break;
                                 }
		 		/* deliberate fall through	*/
		   	   default:
                                                 /* message 099 */
				yyerror( 99 );
		   	   }
			if (x->xvalue <-32768 || x->xvalue>32767)
                                /* range check for 16-bit displacement. */
                                                 /* message 171 */
                        {
                       /*    getmsg(171);    */
                           yyerror1(171, -32768, 32767);
                        }
			outh((int)x->xvalue);
			return;  /* NOTICE A RETURN not BREAK	*/
			}
                                                 /* message 100 */
		   else yyerror( 100 );
		   break;
		case DR:		/* Ra and Rs reversed 	*/
			instr = dfmt(instr,ARG2,ARG1,ARG3);
			break;
		case DB:		/* bc & bcl instructions */
		case DP:		/* bca & bcla instruct. */
			instr = bfmt(instr,ARG1,i2,a3);
			break;
                case DG:  /* for extended mnemonics of the relative and   */
                          /* absolute branch prediction with '+' suffix  */
                        if (a3) {
                           if (a3->xvalue < 0)
			     instr = bfmt(instr,0,i2,a3);
                           else
                             instr = bfmt(instr, 0x00000001, i2, a3);
                        }
			break;
                case DJ:  /* for extended mnemonics of the relative and   */
                          /* absolute branch prediction with '-' suffix  */
                      if (a3) {
                        if ( a3->xvalue < 0 )
			   instr = bfmt(instr,0x00000001,i2,a3);
                        else
                          instr = bfmt(instr, 0, i2, a3);
                       }
			break;
                case DK:  /* for basic mnemonics of the relative and   */
                          /* absolute branch prediction with '+' suffix  */
                      if (a3) {
                        if ( a3->xvalue < 0 )
                          ARG1 &= 0xFFFFFFFE;
                        else
                          ARG1 |= 0x00000001;
                       }
			instr = bfmt(instr,ARG1,i2,a3);
			break;
                case DL:  /* for basic mnemonics of the relative and   */
                          /* absolute branch prediction with '-' suffix  */
                      if (a3) {
                        if ( a3->xvalue < 0 )
                          ARG1 |= 0x00000001;
                        else
                          ARG1 &= 0xFFFFFFFE;
                       }
			instr = bfmt(instr,ARG1,i2,a3);
			break;
		case DE:		/* bXX and bXXl, br rel	*/
		case DF:		/* bbt, bbf, bbtl, bbfl	*/
		case DN:		/* bbta, bbfa, bbtla, bbfla*/
		case DO:		/* bXXa and bXXla	*/
			instr = bfmt(instr,0,i2,a3);
			break;
		case DC:		/* svca and svcla	*/
			instr |= ARG1<<(32-30);
			break;
		case DT:		/* li, lil, liu, lis instructions */
			instr = dfmt(instr,ARG1,0,ARG2);
			break;
		case DY:		/* supervisor calls svc svcl */ 
			instr = scfmt(instr, ARG1,ARG2,ARG3);
			break;
		case LA:		/* branch absolute */
		case LI:		/* branch relative */
			instr |= ARG1 & 0x3fffffc;
			/* --- check for last 2 bits being zero --- */
			break;
		case M:			/* rlimi rotate left immediate */
			instr= mfmt(instr,ARG1,ARG2,ARG3,ARG4,ARG5);
			break;
                case MA:   /*  extended mnemonic extlwi   */
                        instr = mfmt(instr,ARG1,ARG2,ARG4,0,ARG3-1);
                        break;
                case MB:   /*  extended mnemonic extrwi   */

                        if ( (sum_34 = ARG3 + ARG4) > 32 )
                            yyerror (156);
                        else if ( sum_34 == 32)
                            instr = mfmt(instr,ARG1,ARG2,0,32-ARG3,31);
                        else
                           instr = mfmt(instr,ARG1,ARG2,sum_34,32-ARG3,31);
                        break;
                case MC:   /*  extended mnemonic rotlwi   */
                case MH:   /*  extended mnemonic rotlw   */
                        instr = mfmt(instr,ARG1,ARG2,ARG3,0,31);
                        break;
                case MD:    /*  extended mnemonic rotrwi   */
                        instr = mfmt(instr,ARG1,ARG2,(32-ARG3)&0x1f,0,31);
                        break;
                case ME:     /*  extended mnemonic clrlwi   */
                        instr = mfmt(instr,ARG1,ARG2,0,ARG3,31);
                        break;
                case MF:      /*  extended mnemonic clrrwi   */
                        instr = mfmt(instr,ARG1,ARG2,0,0,31-ARG3);
                        break;
                case MG:      /*  extended mnemonic clrslwi   */
                        if ((diff_34=ARG3 - ARG4) < 0 )
                           yyerror(157);
                        else
                          instr = mfmt(instr,ARG1,ARG2,ARG4, diff_34,31-ARG4);
                        break;
		case ML:		/* shift left immediate, sli, slwi */
					/* same as rlinm instruction */
			instr= mfmt(instr,ARG1,ARG2,ARG3,0,31-ARG3);
			break;
		case MR:		/* shift right immediate, sri, srwi */
					/* same as rlinm instruction */
			instr= mfmt(instr,ARG1,ARG2,(32-ARG3)&0x1f,ARG3,31);
			break;
                case MJ:    /*  extended mnemonic inslwi   */
                        if ( (sum_34 = ARG3 + ARG4) > 32 )
                           yyerror (156);
                        else 
			   instr= mfmt(instr,ARG1,ARG2,(32-ARG4)&0x1f,ARG4,
                                    sum_34-1);
                        break;
                case MK:     /*  extended mnemonic insrwi   */
                        if ( (sum_34 = ARG3 + ARG4) > 32 )
                           yyerror (156);
                        else
			  instr= mfmt(instr,ARG1,ARG2,(32-sum_34),ARG4,
                                    sum_34-1);
                        break;
		case Q2:		/* bXXr, bXXrl, bXXc, bXXcl */
			if (i2 < 0)	/* the only form that should */
			   i2 = 0;	/* be taken care of is XL-form */
					/* bcr,bcrl,bctr,and bctrl */
			instr |= i2 << (32-16);
			break;
		case X0:    /* really break, no arguments	*/
                        if ( asm_mode == COM && warning ) 
                           if ( !strcmp(op->name, "dcs") ||
                                !strcmp(op->name, "sc") ||
                                !strcmp(op->name, "sync") ||
                                !strcmp(op->name, "ics") ||
                                !strcmp(op->name, "isync" ) ) 
                               yywarn(153, op->name);
			break;
		case X1:
			instr = xfmt(instr,0,0,ARG1);
			break;
		case X2:	
                        if ( i2 < 0  ) /* no input operand  */
				instr = xfmt(instr,0,0,0);
			else
				instr = xfmt(instr,0,ARG1,0);
			break;
		case X3:		
			instr = xfmt(instr,0,ARG1,ARG2);
			break;
		case X4:
			instr = xfmt(instr,ARG1,0,0);
			break;
		case X5:
                       if ( asm_mode == COM && warning )
                           if ( !strcmp(op->name, "mtsrin") )
                                  yywarn(153, op->name);
			instr = xfmt(instr,ARG1,0,ARG2);
			break;
		case X6:
			instr = xfmt(instr,ARG1,ARG2,0);
			break;
                case XP:   /* handling "move from IBAT or DBAT reg "  */
                           /* the ARG2 is a sequence number for Upper  */
                           /* or Lower. So It must be multiplied by 2   */
                        instr = xfmt(instr, ARG1, 2*ARG2, 0);
                        break;
                case XA:  /* for two input formats: 4 input operands or  */
                          /* 3 input operands.                          */
                        if (f2)
			   instr = xfmt(instr,ARG1,ARG2,ARG3);
                        else
			   instr = xfmt(instr,ARG1,ARG3,ARG4);
                        break;
		case X7:
			instr = xfmt(instr,ARG1,ARG2,ARG3);
			break;
                case XB:  /*   duplicates the ARG2 to ARG3   */
                        instr = xfmt(instr,ARG1,ARG2,ARG2);
                        break;
		case XC:	/* mtcrf move to condition register fields */
                                /* or mtspr with 10-bit spr value          */
			instr=xfxfmt(instr,ARG1,ARG2);
			break;
                case XD:  /*   duplicates the ARG1 to ARG2 and ARG3  */
                        instr = xfmt(instr, ARG1, ARG1, ARG1);
                        break;
		case XE:   /* reverse ARG1 and ARG2, ARG3 is zero */
			instr = xfmt(instr,ARG2,ARG1,0);
			break;
                case XF:        /* swap first and 2nd args      */
                        instr = xfmt(instr,ARG2,ARG1,ARG3);
                        break;
                case XG:    /*  handle ARG2 and ARG3 reversed   */
                        instr = xfmt(instr, ARG1, ARG3, ARG2);
                        break;
                case XH:  /* for two input formats: 3 input operands or  */
                          /* 2 input operands.                          */
                        if ( f2 )
                           instr = xfmt(instr, 0, ARG1, ARG2);
                        else
                           instr = xfmt(instr, ARG1, ARG2, ARG3);
                        break; 
                case XI:    /* handling mtspr. ARG2 and ARG1 reversed. */
                            /* ARG1 is 10-bit SPR which contains       */
                            /* low-order 5 bits followed by high-order */
                            /* 5 bits. The Assembler has to swap them  */
                            /* back to the normal order before generating */
                            /* the binary code.                         */
                        temp = (0x03e0&ARG1) >> 5;
                        instr = xfmt(instr,ARG2,0x001f&ARG1,temp);
                        break;
                case XJ:    /* handling mfspr. */
                            /* ARG2 is 10-bit SPR which contains       */
                            /* low-order 5 bits followed by high-order */
                            /* 5 bits. The Assembler has to swap them  */
                            /* back to the normal order before generating */
                            /* the binary code.                         */
                        temp = (0x03e0&ARG2) >> 5;
                        instr = xfmt(instr,ARG1,0x001f&ARG2,temp);
                        break;
                case XK:   /* handling the basic mnemonics of the branch  */
                           /* conditional to LR or CTR with '+'           */
                           /*  suffix prediction  */
                         ARG1 |= 0x00000001;
			 instr = xfmt(instr,ARG1,ARG2,0);
			 break;
                case XL:   /* handling the basic mnemonics of the branch */
                      /* conditional to LR or CTR with '-' suffix prediction  */
                         ARG1 &= 0xFFFFFFFE;
			 instr = xfmt(instr,ARG1,ARG2,0);
			 break;
                case XM:        /* mtfsf instruct. move to FPSCR field */
                        instr = xflfmt(instr,ARG1,ARG2);
                        break;
                case XN:  /* for mftb inst. having two input formats  */
                          if (f2)
                              instr = xfmt(instr, ARG1, 12, 8);
                          else {   /* handling two input operands  */
                            /* ARG2 is 10-bit TBR which contains       */
                            /* low-order 5 bits followed by high-order */
                            /* 5 bits. The Assembler has to swap them  */
                            /* back to the normal order before generating */
                            /* the binary code.                         */
                              temp = (0x03e0&ARG2) >> 5;
                              instr = xfmt(instr,ARG1,0x001f&ARG2,temp);
                          }
                          break;
                case XO:  /* for invalid form check to mtsri inst.    */
                          if ( !( asm_mode == ANY && modeopt )){ 
                             if ( asm_mode_type == PowerPC )
                                if ( ARG2 != 0 )     /* RA should be zero */
                                    instrs_warn(154);
                           }
                           if ( asm_mode == COM && warning )
                               yywarn(153, op->name);
                           instr = xfmt(instr, ARG1, ARG2, ARG3);
                           break;
                case XQ:  /* for "lswi" or "lsi" inst.   */
                        if ( !(asm_mode == ANY && modeopt )) {
                           if ( asm_mode_type == PowerPC
                                  && asm_mode != PPC_601 ) {
                              /* when asm mode is 601, no check is needed */

                              if ( ARG1 == 0 && ARG2 == 0 ) /*RT=RA=0,error*/
                                   instrs_warn(164);
                              else { /* if RA  in the range of registers to */
                                       /* be loaded,  error */
                                 if ( ARG3 == 0 )
                                      temp = 32;
                                 else
                                      temp = ARG3;
                                 if ( ( temp % 4) == 0 )
                                       temp /= 4;
                                 else
                                       temp = ( temp / 4) + 1;
                                 if ((temp = (temp + ARG1 -1)) >= 32 ) {
                                     if ( ARG2 >= ARG1 || 
                                            (ARG2 >= 0 && ARG2 <= (temp -32)))
                                          instrs_warn(164);
                                 } else {
                                       if ( ARG2 >= ARG1 && ARG2 <= temp )
                                           instrs_warn(164);
                                 }
                              }
                           }
                        }
                        instr = xfmt(instr,ARG1,ARG2,ARG3);
                        break;
                case XR:        /* "mr" or "not" inst. same as or, nor inst. */
                        instr=xfmt(instr,ARG2,ARG1,ARG2);
                        break;
                case XS:  /* invalid form check "RA !=0"       */
                      /* inst.: "stbux", "sthux", "stwux" ("stux" for POWER), */
                              /* "lfsux", "lfdux", "stfsux", "stfdux"  */
                      if ( !(asm_mode == ANY && modeopt )) {
                        if ( asm_mode_type == PowerPC
                                    && asm_mode != PPC_601 ) {
                              /* when asm mode is 601, no check is needed */
                                if ( ARG2 == 0 )  /* if RA == 0, error */
                                   instrs_warn(166);
                         }
                       }
                        instr = xfmt(instr,ARG1,ARG2,ARG3);
                        break;
                case XT:   /* invalid form check "RA !=0" and RA!=RT"    */
                           /*  inst. :"lbzux", "lhzux", "lhaux", "lux"   */
                           /*    "lwzux" ("lux" for POWER)            */
                      if ( !(asm_mode == ANY && modeopt )) {
                        if ( asm_mode_type == PowerPC
                                    && asm_mode != PPC_601 ) {
                              /* when asm mode is 601, no check is needed */
                                if ( ( ARG2 == 0 ) || (ARG1 == ARG2 ) )
                                   /* if RA == 0 or RA == RT, error  */
                                    instrs_warn(151);
                         }
                       }
                        instr = xfmt(instr,ARG1,ARG2,ARG3);
                        break;
                case XU:   /* invalid form check "RA !=0 and RA!=0"    */
                           /*  inst. :"lswx",  and "lsx" for POWER     */
                      if ( !(asm_mode == ANY && modeopt )) {
                        if ( asm_mode_type == PowerPC
                                    && asm_mode != PPC_601 ) {
                              /* when asm mode is 601, no check is needed */
                              if ( ARG1 == 0 && ARG2 == 0 ) /*RT=RA=0,error*/
                                   instrs_warn(164);
                         }
                       }
                       if ( asm_mode == COM && warning )
                          yywarn(153, op->name);
                        instr = xfmt(instr,ARG1,ARG2,ARG3);
                        break;

                case XV:   /* handling "move to IBAT or DBAT reg "  */
                           /* ARG1 and ARG2 are reversed.              */
                           /* the ARG1 is a sequence number for Upper  */
                           /* or Lower. So It must be multiplied by 2   */
                        instr = xfmt(instr, ARG2, 2*ARG1, 0);
                        break; 
                case XW:  /* handle mfdec inst.                    */
                          /*  if asm mode is pwr or pwrx or 601, use 6      */
                          /* as the DEC Reg. encoding to generate obj code   */
                          /*  if asm mode is default or any, use 6         */
                          /* as the DEC Reg. encoding to generate obj code   */
                          /* and report warning message                      */
                          /*  if asm mode is ppc or 603 or 604, use 22      */
                          /* as the DEC Reg. encoding to generate obj code   */
                          /* if the asm mode is com, report error            */

                         if ( asm_mode_type == POWER || 
                                            asm_mode == PPC_601)
                            instr = xfmt(instr, ARG1, 6,0);
                         else {
                            if ( asm_mode == COM && modeopt)
                                yyerror1(149, op->name, asm_mode_str);   
                            else if ( asm_mode == COM && !modeopt )
                            {
                                instr = xfmt(instr, ARG1, 6,0);
                                instrs_warn(158, asm_mode_str);
                            }
                            else if ( asm_mode == ANY )
                            {  
                                instr = xfmt(instr, ARG1, 6, 0); 
                                if ( warning ) 
                                     yywarn(158, asm_mode_str);
                             }
                             else       
                                instr = xfmt(instr, ARG1, 22, 0);
                         }
                         break;
		case YI:	/* ilopi instruct. not found */
                                                 /* message 065 */
			yyerror( 65 );
			break;
		case Z0:	/* lopi instr. not found */
                                                 /* message 065 */
			yyerror( 65 );
			break;
		case Z7:	/* ilopr and rtc  instruct. not found */
                                                 /* message 065 */
			yyerror( 65 );
			break;
		case Z8:      /* ryo instruc. not found */
                                                 /* message 065 */
			yyerror( 65 );
			break;
		case Z9:	/* ryod instruction not found */
                                                 /* message 065 */
			yyerror( 65 );
			break;
		default :
                                                 /* message 103 */
			yyerror( 103 );
			break;
		}
 
	outl(instr);		/* output filled in instruction */
        if ( (op->inst_bmap & asm_mode ) == 0 )
             /* redo the unsupported instruction checking in Pass 2 here  */
             /* since assmbler listing is created in Pass 1 and recreated */ 
             /* in Pass 2.                                                */ 

           if (asm_mode != ANY )
           {
               instrs_warn(149, op->name, asm_mode_str);
           }
}
/****************************************************************/
/* function:	checkabs					*/
/* purpose:	checks absolute value and checks range		*/
/* Input:    x  -  the value to be checked ( It is an input 
                   operand value. )
             maxval  -  upper bound of the value
             minval  -  lower bound of the value
             n  -  the position of the input operand            */
/* returns:	nothing						*/
/****************************************************************/
checkabs(x,maxval,minval,n)
struct exp *x;
int maxval,minval,n;
{
	if (x->xtype != E_ABS)
                                                 /* message 104 */
		yyerror( 104 , n);
	if (x->xvalue<minval||x->xvalue>maxval) 
                                                 /* message 107 */
		yyerror( 107 , n);
}
/****************************************************************/
/* Function:	scfmt						*/
/* Purpose:	generate the sc-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		lev    - the first reg 				*/
/*		fl1    - the second reg				*/
/*		fl2    - the third reg 				*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int scfmt(instr,lev,fl1,fl2)
int instr, lev, fl1, fl2;
{
	instr |= lev<< (32 - 27);
	instr |= fl1<< (32 - 20);
	instr |= fl2<< (32 - 30);
	return(instr);
}
/****************************************************************/
/* Function:	xflfmt						*/
/* Purpose:	generate the xflfmt-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		flm    - the first reg 				*/
/*		frb    - the second reg				*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int xflfmt(instr, flm, frb)
int instr, flm, frb;
{
	instr |= flm << (32 - 16);
	instr |= frb << (32 - 21);
	return(instr);
}
/****************************************************************/
/* Function:	xfmt						*/
/* Purpose:	generate the x-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		rt    - the first reg 				*/
/*		ra    - the second reg 				*/
/*		rb    - the third reg 				*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int xfmt(instr,rt,ra,rb)
int instr,rt,ra,rb;
{
	instr |= rt<<(32-11);
	instr |= ra<<(32-16);
	instr |= rb<<(32-21);
	return(instr);
}
/****************************************************************/
/* Function:	xfxfmt						*/
/* Purpose:	generate the xfx-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		fxm    - the first reg 				*/
/*		rs    - the second reg 				*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int xfxfmt(instr, fxm, rs)
int instr, fxm, rs;
{
	instr |= fxm << (32 - 21);
	instr |= rs << (32 - 11);
	return(instr);
}

/****************************************************************/
/* Function:	mfmt						*/
/* Purpose:	generate the m-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		ra    - the first reg 				*/
/*		rs    - the second reg 				*/
/*		sh    - the third reg 				*/
/*		mb    -	the forth reg 				*/
/*		me    - the fifth reg				*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int mfmt(instr, ra, rs, sh, mb, me)
int instr, ra, rs, sh, mb, me; 
{
	instr |= ra << (32-16);
	instr |= rs << (32-11);
	instr |= sh << (32-21);
	instr |= mb << (32-26);
	instr |= me << (32-31);
	return(instr);
}

/****************************************************************/
/* Function:	dfmt						*/
/* Purpose:	generate the d-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		rt    - the first reg 				*/
/*		ra    - the second reg 				*/
/*		d     - the displacement field (also UI, or SI)	*/
/* Returns:	the filled in instruction			*/
/****************************************************************/
int dfmt(instr,rt,ra,d)
int instr,rt,ra,d;
{
	if (rt) 
		instr |= rt<<(32-11);
	instr |= ra<<(32-16);
	instr |= d&0xFFFF;
	return(instr);
}

/****************************************************************/
/* Function:	bfmt						*/
/* Purpose:	generate the b-format instructions		*/
/* Input:	instr - the instruction skeleton to fill in	*/
/*		bo    - branch option field			*/
/*		bi    - branch CR field				*/
/*		a3    - pointer to exp with branch address	*/ 
/* Returns:	the filled in instruction			*/
/****************************************************************/
int bfmt(instr,bo,bi,a3)
int instr,bo,bi;
struct exp *a3;
{
int bd;
           if (a3)
	      bd = a3->xvalue;
           else bd=0; 	
	   if (bo)
	      instr |= bo<<(32-11);
	   instr |= bi<<(32-16);
	   instr |= bd & 0xFFFC;
	   return(instr);
}

#define SUBSTR(a,b,c) ((a)&(0x00000001 << (32-(b))))
/****************************************************************/
/* Function: cvtmask                                            */
/* Purpose:  convert a 32 bit mask to mask begin end indexes	*/
/* Input:    mask -  the mask to convert			*/
/* Returns:  the number of the bit that was in error		*/ 
/*i	     -1 if successful					*/
/* Output:   mb    - the generated beginning index		*/
/*           me    - the generated ending index			*/
/* Comments: Valid masks are described by two 5-bit fields.	*/
/*	MB defines the first '1' bit in the  mask and ME 	*/
/* 	defines the last '1' bit in the mask.  Only certain 	*/
/*	types of masks may be specified.  Consider that the mask*/
/*	is a circular entity with bit 0 following bit 31.	*/
/*	 A valid mask is defined as a single series of one bits */
/*	( one or more ) surrounded by zero bits (zero or more).	*/
/*	A mask of all zero bits may not be specified.		*/
/* Examples of valid masks are:					*/
/*				0            15              31	*/
/*                              |             |               |	*/
/* mb =  0  me = 31             1111111111111111111111111111111	*/
/* mb =  0  me =  0             1000000000000000000000000000000	*/
/* mb =  0  me = 22             1111111111111111111111000000000	*/
/* mb = 12  me = 25             0000000000011111111111111000000	*/
/* mb = 22  me = 31             0000000000000000000011111111111	*/
/* mb = 29  me =  6             1111111000000000000000000000111	*/
/*								*/ 
/*Examples of invalid masks are:0            15              31 */
/*                              |             |               | */
/*                              0000000000000000000000000000000 */
/*                              0101010101010101010101010101010 */
/*                              0000000000011110000011000000000 */
/*                              1111110000011111111111111000000 */
/* Acknowledgement: This routine was converted from the PL8 	*/
/* 	routine of the same function written by Larry Wise	*/
/****************************************************************/
cvtmask(mask,mb,me)
unsigned int mask;  	/* The mask to be converted */
int *mb;  		/* Begin bit number 0-31 */
int *me;		 /* End bit number 0-31 */
{
int errorbit;  	/* Bit where mask detected invalid 0-31 */
int i;
/* init no error and mb = 0 and me = 0 */
errorbit = -1;
*mb       = 0;
*me       = 0;
 
BeginState:
i=1; /* Bit loop count 0-31 */
if (SUBSTR(mask,1,1)) {
State11:                           /* Starting state - bit 0 = 1 */
  for (; (i<32); i++) {
    if (!SUBSTR(mask,i+1,1)) {
      *me = i-1;
      goto State12;
      }
    }
/*  mb = 0;  init did this */
  *me = 31;
  goto done;
 
State12:
  for (; (i<32); i++) {
    if (SUBSTR(mask,i+1,1)) {
      *mb = i;
      goto State13;
      }
    }
/*  mb = 0;   init did this */
  goto done;
 
State13:
  for (; (i<32); i++) {
    if (!SUBSTR(mask,i+1,1)) {
      errorbit = i;
      goto done;
      }
    }
  goto done;
  }
else {
 
State01:                                 /* Starting state - bit 0 = 0 */
  for (; (i<32); i++) {
    if (SUBSTR(mask,i+1,1)) {
      *mb = i;
      goto State02;
      }
    }
  errorbit = 31;
  goto done;
 
State02:
  for (; (i<32); i++) {
    if (!SUBSTR(mask,i+1,1)) {
      *me = i-1;
      goto State03;
      }
    }
  *me = 31;
  goto done;
 
State03:
  for (; (i<32); i++) {
    if (SUBSTR(mask,i+1,1)) {
      errorbit = i;
      goto done;
      }
    }
  goto done;
  }
 
done:
   return errorbit;
}

/*****************************************************************
* Function: instrs_warn
* purpose: report instructional warning if the asm_mode is default 
*          and no "-W" and no "-w" are used. 
*          otherwise report error .
* Input:   err_inx  -  error or warning message number
******************************************************************/
void
instrs_warn(int err_idx, ...)
{
va_list  ap;
   if (asm_mode != ANY ) { 
      va_start(ap, err_idx);
      if (!modeopt )        /* default mode */
      { if ( warning )  /* ( no -w and no -W ) or -w  */
          yywarn2(err_idx, ap);
      }
      else
         yyerror2(err_idx, ap);
      va_end(ap);
   }
}

