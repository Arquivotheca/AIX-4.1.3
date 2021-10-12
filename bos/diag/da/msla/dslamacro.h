/* @(#)27	1.4  src/bos/diag/da/msla/dslamacro.h, damsla, bos411, 9428A410j 12/10/92 09:12:34 */
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: DA_SETRC
 *		EXECUTE_TU_WITHOUT_MENU
 *		EXECUTE_TU_WITH_MENU
 *		IS_TM
 *		PUT_EXIT_MENU
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define DA_SETRC( a, b, c, d, e ) \
              DA_SETRC_STATUS( a);\
              DA_SETRC_USER  ( b);\
              DA_SETRC_ERROR ( c);\
              DA_SETRC_TESTS ( d);\
              DA_SETRC_MORE  ( e);

/**************************************************************************/
/*                                                                        */
/* IS_TM is a macro takes two input variables VAR1 & VAR2.                */
/*                                                                        */
/* VAR1 is an object of the sturcture tm_input.                           */
/*                                                                        */
/* VAR2 is a variable or a defined value will be compared to the tm_input */
/*      object class.                                                     */
/*                                                                        */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/* MORE_RESOURCE is a macro define if more resource needed or further     */
/*               isalation needed.                                        */
/*                                                                        */
/**************************************************************************/

#define MORE_RESOURCE   (       (       MSLAIOMEM_TEST                     \
                                   ||   MSLAIOREG_TEST                     \
                                )                                          \
                                && !( IS_TM(loopmode, LOOPMODE_EXITLM) )   \
                        )

/**************************************************************************/
/*                                                                        */
/* DISPLAY_TESTING is a macro define when a title line would be displayed */
/*                                                                        */
/**************************************************************************/

#define DISPLAY_TESTING                                                    \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM   )        \
                           ||   IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                           ||   IS_TM( loopmode, LOOPMODE_INLM    )        \
                        )                                                  \
                        && ( IS_TM( console, CONSOLE_TRUE         )   )    \
                )

/**************************************************************************/
/*									  */
/* DISPLAY_LOOP_COUNT is a macro define when loop count will be displayed */
/*									  */
/**************************************************************************/

#define DISPLAY_LOOP_COUNT 						   \
		( 		IS_TM( loopmode, LOOPMODE_INLM  )	   \
			&&	IS_TM( console, CONSOLE_TRUE	)   	   \
		)

/**************************************************************************/
/* DISPLAY_TITLE is a macro that returns TRUE if conditions exist that  */
/*			will allow the display of the testing device      */
/*			message to the display.                           */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define DISPLAY_TITLE		(  					   \
					IS_TM( loopmode,LOOPMODE_NOTLM )   \
				&&	IS_TM( console,CONSOLE_TRUE )	   \
				)

/**************************************************************************/
/* DISPLAY_ADV_TITLE is a macro that returns TRUE if conditions exist that */
/*			will allow the display of the testing menu        */
/*			to the display.                                   */
/* NOTE: macro returns logical TRUE or FALSE				  */
/**************************************************************************/

#define DISPLAY_ADV_TITLE	(  					   \
					(IS_TM( loopmode,LOOPMODE_NOTLM )   \
				||	IS_TM( loopmode,LOOPMODE_ENTERLM )) \
				&&	IS_TM( console,CONSOLE_TRUE )	   \
				&&	IS_TM( advanced,ADVANCED_TRUE )	   \
				)

/**************************************************************************/
/*                                                                        */
/* EXECUTE_TU_WITH_MENU is a macro define execution conditions for Test   */
/*                      Units with a menu(s) to display. The macro takes  */
/*                      one parameter TU_NUM is the test unit number.     */
/*                                                                        */
/**************************************************************************/

#define EXECUTE_TU_WITH_MENU(TU_NUM)                                       \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM     )      \
                           ||   IS_TM( loopmode, LOOPMODE_ENTERLM   )      \
                           ||   IS_TM( loopmode, LOOPMODE_INLM      )      \
                        )                                                  \
                        &&      ( IS_TM( console, CONSOLE_TRUE  )   )      \
                        &&      ( tmode[/**/TU_NUM -1].mflg == TRUE )      \
                )

/**************************************************************************/
/*                                                                        */
/* EXECUTE_TU_WITHOUT_MENU is a macro define execution conditions for     */
/*                         test units without menus. The macro takes one  */
/*                         paramater TU_NUM is the test unit number.      */
/*                                                                        */
/**************************************************************************/

#define EXECUTE_TU_WITHOUT_MENU( TU_NUM )                                  \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM )          \
                           ||   IS_TM(loopmode,LOOPMODE_ENTERLM)           \
                           ||   IS_TM(loopmode,LOOPMODE_INLM)              \
                        )                                                  \
                        &&      ( tmode[/**/TU_NUM -1].mflg == FALSE )     \
                )

/**************************************************************************/
/*                                                                        */
/* DISPLAY_MENU is a macro define when a menu should be dispalyed, if     */
/*              the test unit have a menu to display. This macro takes    */
/*              one paramater TU_NUM is the test unit number              */
/*                                                                        */
/**************************************************************************/

#define DISPLAY_MENU                                                       \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM   )        \
                           ||   IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                        )                                                  \
                        && ( IS_TM( console, CONSOLE_TRUE)        )        \
                )

/**************************************************************************/
/*                                                                        */
/* PUT_EXIT_MENU is a macro define when it has to display an exit menu if */
/*               the test unit have displayed a previous menu. This macro */
/*               takes one paramater is the Test unit number.             */
/*                                                                        */
/**************************************************************************/

#define PUT_EXIT_MENU( TU_NUM )                                            \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM     )      \
                           ||   IS_TM( loopmode, LOOPMODE_EXITLM    )      \
                        )                                                  \
                        && ( IS_TM( console, CONSOLE_TRUE)        )        \
                        && ( tmode[/**/TU_NUM -1].mflg == TRUE    )        \
                )
/**************************************************************************/

#define SW_ERROR   		     \
{ DA_SETRC_ERROR( DA_ERROR_OTHER );  \
DA_SETRC_MORE( DA_MORE_CONT );       \
clean_up();                          \
}
