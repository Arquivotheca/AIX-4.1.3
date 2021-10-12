static char sccsid[] = "@(#)19  1.6.1.3  src/bos/diag/tu/artic/exectu.c, tu_artic, bos411, 9428A410j 2/18/94 11:12:37";
/* COMPONENT_NAME:  
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 */

#include <stdio.h>
#include <artictst.h>


/*
 * NAME: exectu
 *
 * FUNCTION: Called by the Hardware Exerciser, Munufacturing Application
 *           and the Diagnostic Application to invoke a Test Unit (TU).
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) If the "mfg" member of the struct within TUTYPE is set to
 *          INVOKED_BY_HTX, then exectu() is being invoked by the HTX Hardware
 *          Exerciser so test units know to look at variables in TUTYPE for
 *          values from the rule file.  Else, the test units use pre-defined
 *          values while testing.
 *
 *          NOTE that Test Units 3-8, 10-16, 20-71 call upon the microcode
 *          to perform a specified test.  Therefore TU019 (or in the case of
 *          TU071) TU070 must be called
 *          prior to these which insures that the diag microcode is loaded
 *          and executing.
 * NOTE:    All references to test units for SP-5 have been removed, as
 *          diagnostics does not want to use these test units.
 *
 *
 * RETURNS: A zero (0) is returned if test unit was successful or a non-zero
 *          if the test unit was unsuccessful.
 *
 */
int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        register i, loop, tu;
        static int rc;
        static int base_diag_ucode_running = 0;
/*        static int eib_diag_ucode_running = 0;  SP-5 ONLY */

        extern int tu001();    /* all */
/*        extern int tu002();     SP-5 */
        extern int tu003();    /* all */
        extern int tu004();    /* all */
        extern int tu005();    /* all */
        extern int tu006();    /* MP,X25 */
        extern int tu007();    /* MP,X25 */
        extern int tu008();    /* MP,X25 */
        extern int tu010();    /* X25 */
        extern int tu011();    /* X25 */
        extern int tu012();    /* X25 */
        extern int tu013();    /* X25 */
        extern int tu014();    /* X25 */
        extern int tu015();    /* X25 */
        extern int tu016();    /* X25 */
        extern int tu018();    /* all */
        extern int tu019();    /* all */
        extern int tu020();    /* MP */
        extern int tu021();    /* MP */
        extern int tu022();    /* MP */
        extern int tu023();    /* MP */
        extern int tu024();    /* MP */
        extern int tu025();    /* MP */
        extern int tu026();    /* MP */
        extern int tu027();    /* MP */
        extern int tu028();    /* MP */
        extern int tu029();    /* MP */
        extern int tu030();    /* MP */
        extern int tu032();    /* 4P/MP */
        extern int tu033();    /* 4P/MP */
        extern int tu034();    /* 4P/MP */
        extern int tu035();    /* 4P/MP */
        extern int tu036();    /* 4P/MP */
        extern int tu037();    /* 4P/MP */
        extern int tu038();    /* 4P/MP */
        extern int tu039();    /* 4P/MP */
        extern int tu040();    /* 4P/MP */
        extern int tu041();    /* 4P/MP */
        extern int tu042();    /* 4P/MP */
        extern int tu043();    /* 4P/MP */
        extern int tu044();    /* 4P/MP */
        extern int tu045();    /* 4P/MP */
        extern int tu046();    /* 4P/MP */
        extern int tu047();    /* 4P/MP */
        extern int tu048();    /* 4P/MP */
        extern int tu049();    /* 4P/MP */
        extern int tu050();    /* 4P/MP */
        extern int tu051();    /* PM */
        extern int tu052();    /* PM */
        extern int tu053();    /* PM */
        extern int tu054();    /* PM */
        extern int tu055();    /* PM */
        extern int tu056();    /* PM */
        extern int tu057();    /* PM */
        extern int tu058();    /* PM */
        extern int tu059();    /* PM */
        extern int tu060();    /* PM */
/*        extern int tu070();
        extern int tu071();    SP-5  */
        extern int mktu_rc();

        tu = tucb_ptr->header.tu;

        loop = tucb_ptr->header.loop;

        if (icagetadaptype(fdes, &tucb_ptr->artic_info.reserved)) 
           return (DRV_ERR);

        /*
         * NOTE that Test Units 3-8, 10-16, 20-60, and 71 call on the
         * microcode to perform a specified test.  Therefore
         * TU 19  or TU 70 must be called prior to these which insures
         * that the microcode is loaded and executing.
         */

        for (i = 0; i < loop; i++)
           {
                if (!base_diag_ucode_running)
                   {
                        if ( ((tu >= 3) && (tu <= 8)) ||
                             ((tu >= 10) && (tu <= 16)) ||
                             ((tu >= 20) && (tu <= 60)) )
                           {
                                return(NOUCODE);
                           }
                   }

/*                 if (!eib_diag_ucode_running)
                   {
                        if ( tu == 71)
                           {
                                return(NOUCODE);
                           }
                   }            SP-5      */

                switch(tu)
                   {
                        case  1:        /* POST test */
                                rc = tu001(fdes, tucb_ptr);
                                break;

/*                         case  2:         VPD test */
/*                                rc = tu002(fdes, tucb_ptr);
                                break;    SP-5   */

                        case  3:        /* Interrupt test */
                                rc = tu003(fdes, tucb_ptr);
                                break;

                        case  4:        /* CPU test */
                                rc = tu004(fdes, tucb_ptr);
                                break;

                        case  5:        /* RAM test */
                                rc = tu005(fdes, tucb_ptr);
                                break;

                        case  6:        /* GATEARRAY test */
                                rc = tu006(fdes, tucb_ptr);
                                break;

                        case  7:        /* CIO test */
                                rc = tu007(fdes, tucb_ptr);
                                break;

                        case  8:        /* SCC test */
                                rc = tu008(fdes, tucb_ptr);
                                break;

                        case 10:        /* All wraps (pin) for C2X */
                                rc = tu010(fdes, tucb_ptr);
                                break;

                        case 11:        /* X.21 wrap (pin) for C2X */
                                rc = tu011(fdes, tucb_ptr);
                                break;

                        case 12:        /* V.24 wrap (pin) for C2X */
                                rc = tu012(fdes, tucb_ptr);
                                break;

                        case 13:        /* V.35 wrap (pin) for C2X */
                                rc = tu013(fdes, tucb_ptr);
                                break;

                        case 14:        /* X.21 wrap (cable) for C2X */
                                rc = tu014(fdes, tucb_ptr);
                                break;

                        case 15:        /* V.24 wrap (cable) for C2X */
                                rc = tu015(fdes, tucb_ptr);
                                break;

                        case 16:        /* V.35 wrap (cable) for C2X */
                                rc = tu016(fdes, tucb_ptr);
                                break;

                        case 18:        /* General Reg Test */
                                rc = tu018(fdes, tucb_ptr);
                                break;

                        case 19:        /* Loads base card diagnostic task */
                                rc = tu019(fdes, tucb_ptr);
                                if (!rc)
                                {
                                        base_diag_ucode_running = 1;
/*                                        eib_diag_ucode_running = 0; SP-5  */
                                }
                                else
                                        base_diag_ucode_running = 0;
                                break;

                        case 20:      /* CIO Port A wrap for MP/2 232 */
                                rc = tu020(fdes, tucb_ptr);
                                break;

                        case 21:      /* CIO Port B wrap for MP/2 232 */
                                rc = tu021(fdes, tucb_ptr);
                                break;

                        case 22:     /* Port 0 wrap for MP/2 */
                                  rc = tu022(fdes, tucb_ptr);
                                break;

                        case 23:    /* Port 1 wrap for MP/2 */
                                  rc = tu023(fdes, tucb_ptr);
                                break;

                        case 24:   /* Port 2 wrap for MP/2 */
                                  rc = tu024(fdes, tucb_ptr);
                                break;

                        case 25:  /* Port 3 wrap for MP/2 */
                                  rc = tu025(fdes, tucb_ptr);
                                break;

                        case 26: /* Port 4 wrap for MP/2 */
                                if(tucb_ptr->artic_info.reserved == MP2_4P232)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu026(fdes, tucb_ptr);
                                break;

                        case 27:  /* Port 5 wrap for MP/2 */
                                if(tucb_ptr->artic_info.reserved == MP2_4P232)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu027(fdes, tucb_ptr);
                                break;

                        case 28:  /* Port 6 wrap for MP/2 */
                                if(tucb_ptr->artic_info.reserved == MP2_4P232 ||
                                tucb_ptr->artic_info.reserved == MP2_6PSYNC)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu028(fdes, tucb_ptr);
                                break;

                        case 29:  /* Port 7 wrap for MP/2 */
                                if(tucb_ptr->artic_info.reserved == MP2_4P232 ||
                                tucb_ptr->artic_info.reserved == MP2_6PSYNC)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu029(fdes, tucb_ptr);
                                break;

                        case 30:  /* Counter A for MP/2 */
                                rc = tu030(fdes, tucb_ptr);
                                break;

                        case 31:  /* Counter B for MP/2 ** obsolete */
                                   return(0);
                                break;

                        case 32:  /* 4P/MP EIB Relay for 4/P Selectable */
                                rc = tu032(fdes, tucb_ptr);
                                break;

                        case 33:  /* 4P/MP EIB Wrap for P0/RS232  */
                                rc = tu033(fdes, tucb_ptr);
                                break;

                        case 34:  /* 4P/MP EIB Wrap for P0/RS422  */
                                rc = tu034(fdes, tucb_ptr);
                                break;

                        case 35:  /* 4P/MP EIB Wrap for P0/X21  */
                                rc = tu035(fdes, tucb_ptr);
                                break;

                        case 36:  /* 4P/MP EIB Wrap for P0/V35 */
                                rc = tu036(fdes, tucb_ptr);
                                break;

                        case 37:  /* 4P/MP EIB Wrap for P1/RS232 */
                                rc = tu037(fdes, tucb_ptr);
                                break;

                        case 38:  /* 4P/MP EIB Wrap for P1/V35 */
                                rc = tu038(fdes, tucb_ptr);
                                break;

                        case 39:  /* 4P/MP EIB Wrap for P2/RS232 */
                                rc = tu039(fdes, tucb_ptr);
                                break;

                        case 40:  /* 4P/MP EIB Wrap for P2/RS422 */
                                rc = tu040(fdes, tucb_ptr);
                                break;

                        case 41:  /* 4P/MP EIB Wrap for P3/RS232 */
                                rc = tu041(fdes, tucb_ptr);
                                break;

                        case 42:  /* 4P/MP EIB Cable Wrap for P0/RS232 */
                                rc = tu042(fdes, tucb_ptr);
                                break;

                        case 43:  /* 4P/MP EIB Cable Wrap for P0/RS422 */
                                rc = tu043(fdes, tucb_ptr);
                                break;

                        case 44:  /* 4P/MP EIB Cable Wrap for P0/X21 */
                                rc = tu044(fdes, tucb_ptr);
                                break;

                        case 45:  /* 4P/MP EIB Cable Wrap for P0/V35 */
                                rc = tu045(fdes, tucb_ptr);
                                break;

                        case 46:  /* 4P/MP EIB Cable Wrap for P1/RS232 */
                                rc = tu046(fdes, tucb_ptr);
                                break;

                        case 47:  /* 4P/MP EIB Cable Wrap for P1/V35 */
                                rc = tu047(fdes, tucb_ptr);
                                break;

                        case 48:  /* 4P/MP EIB Cable Wrap for P2/RS232 */
                                rc = tu048(fdes, tucb_ptr);
                                break;

                        case 49:  /* 4P/MP EIB Cable Wrap for P2/RS422 */
                                rc = tu049(fdes, tucb_ptr);
                                break;

                        case 50:  /* 4P/MP EIB Cable Wrap for P3/RS232 */
                                rc = tu050(fdes, tucb_ptr);
                                break;

                        case 51:        /* CIO test */
                                rc = tu051(fdes, tucb_ptr);
                                break;

                        case 52:        /* GATE ARRAY  */
                                rc = tu052(fdes, tucb_ptr);
                                break;

                        case 53:     /* Port 0 wrap for Portmaster */
                                  rc = tu053(fdes, tucb_ptr);
                                break;

                        case 54:    /* Port 1 wrap for PM */
                                  rc = tu054(fdes, tucb_ptr);
                                break;

                        case 55:   /* Port 2 wrap for PM */
                                  rc = tu055(fdes, tucb_ptr);
                                break;

                        case 56:  /* Port 3 wrap for PM */
                                  rc = tu056(fdes, tucb_ptr);
                                break;

                        case 57: /* Port 4 wrap for PM */
                                  rc = tu057(fdes, tucb_ptr);
                                break;

                        case 58:  /* Port 5 wrap for PM */
                                  rc = tu058(fdes, tucb_ptr);
                                break;

                        case 59:  /* Port 6 wrap for PM */
                                if(tucb_ptr->artic_info.reserved == PM_6PV35 ||
                                tucb_ptr->artic_info.reserved == PM_6PX21)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu059(fdes, tucb_ptr);
                                break;

                        case 60:  /* Port 7 wrap for PM */
                                if(tucb_ptr->artic_info.reserved == PM_6PV35 ||
                                tucb_ptr->artic_info.reserved == PM_6PX21)
                                   return(ILLEGAL_EIB_PORT);
                                  rc = tu060(fdes, tucb_ptr);
                                break;

/*                         case 70:       Loads SPV diagnostic task */
/*                                rc = tu070(fdes, tucb_ptr);
                                if(!rc)
                                {
                                        eib_diag_ucode_running = 1;
                                        base_diag_ucode_running = 0;
                                }
                                else
                                        eib_diag_ucode_running = 0;
                                break;  SP-5  */

/*                        case 71:       Tests SPV eib attachment */
/*                                rc = tu071(fdes, tucb_ptr);
                                break;    SP-5    */

                        default:
                                return(ILLEGAL_TU_ERR);
                   };

                if (rc)
                   {
                        rc = mktu_rc(tucb_ptr->header.tu, 0x00, rc);
                        break;

                   }
           } /* end for */

        return(rc);
   }

