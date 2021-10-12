static char sccsid[] = "@(#)80	1.1  src/bos/diag/tu/gem/dgsmon1.c, tu_gem, bos411, 9428A410j 5/30/91 12:48:03";
/*
 * COMPONENT_NAME: tu_gem ( dgsmon1 ) 
 *
 * FUNCTIONS: colorbar
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "dgsmon.h"

struct pattern1
{
/* Set interior style to solid:       */
   struct setintsty setsolid;
/* Set interior color direct:         */
   struct setintci setwhite1;
/* Display 2 white rectangles on top of screen:           */
   struct fillrect3 rect1;
   struct fillrect3 rect2;

/* Display 6 VERTICAL COLOR BARS across center screen:    */
/* Display green color bar:     */
   struct setintci setgreen;
   struct fillrect3 bargr;
/* Display yellow color bar:    */
   struct setintci setyelow;
   struct fillrect3 baryel;
/* Display red color bar:       */
   struct setintci setred;
   struct fillrect3 barred;
/* Display magenta color bar:   */
   struct setintci setmag;
   struct fillrect3 barmag;
/* Display blue color bar:      */
   struct setintci setblue;
   struct fillrect3 barblu;
/* Display cyan color bar:      */
   struct setintci setcyan;
   struct fillrect3 barcyn;
/* Display white color bar:     */
   struct setintci setwhite2;
   struct fillrect3 barwhi;

/*  Display GRAY SCALE below color bars.  */
/* Display white:               */
   struct fillrect3 gray0;
/* Display gray1:               */
   struct setintci setgray1;
   struct fillrect3 gray1;
/* Display gray2:               */
   struct setintci setgray2;
   struct fillrect3 gray2;
/* Display gray3:              */
   struct setintci setgray3;
   struct fillrect3 gray3;
/* Display gray4:               */
   struct setintci setgray4;
   struct fillrect3 gray4;
/* Display gray5:               */
   struct setintci setgray5;
   struct fillrect3 gray5;
/* Display gray6:               */
   struct setintci setgray6;
   struct fillrect3 gray6;

/*  Display 'PIANO KEYBOARD' across bottom of screen.    */
   struct setintci setwhite3;
/*  Display 2 rectangles (53 by 128 pixels):   */
   struct fillrect3 piano1;
   struct fillrect3 piano2;
/*  Display 4 rectangles (40 by 128 pixels):   */
   struct fillrect3 piano3;
   struct fillrect3 piano4;
   struct fillrect3 piano5;
   struct fillrect3 piano6;
/*  Display 6 rectangles (20 by 128 pixels):   */
   struct fillrect3 piano7;
   struct fillrect3 piano8;
   struct fillrect3 piano9;
   struct fillrect3 piano10;
   struct fillrect3 piano11;
   struct fillrect3 piano12;
/*  Display 6 rectangles (10 by 128 pixels):   */
   struct fillrect3 piano13;
   struct fillrect3 piano14;
   struct fillrect3 piano15;
   struct fillrect3 piano16;
   struct fillrect3 piano17;
   struct fillrect3 piano18;
/*  Display 6 rectangles (5 by 128 pixels):    */
   struct fillrect3 piano19;
   struct fillrect3 piano20;
   struct fillrect3 piano21;
   struct fillrect3 piano22;
   struct fillrect3 piano23;
   struct fillrect3 piano24;
/*  Display 6 rectangles (3 by 128 pixels):    */
   struct fillrect3 piano25;
   struct fillrect3 piano26;
   struct fillrect3 piano27;
   struct fillrect3 piano28;
   struct fillrect3 piano29;
   struct fillrect3 piano30;

/* Display 9 SINGLE PIXELS in lower right corner of screen.    */
   struct plyline plypt1;
   struct plyline plypt2;
   struct plyline plypt3;
   struct plyline plypt4;
   struct plyline plypt5;
   struct plyline plypt6;
   struct plyline plypt7;
   struct plyline plypt8;
   struct plyline plypt9;
};


/*
 * NAME:
 *	colorbar
 *                                                                    
 * FUNCTION:
 * 	Displays the color bar pattern for the visual tests.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:)
 *	Sets up various structures needed for the color bar pattern
 *	and does a FIFO write
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * INPUT:
 *      unsigned long gmbase;          address of the memory base
 *
 * RETURNS:
 *	None.
 */  

void
colorbar(gmbase)
unsigned long gmbase;
{
   int  ssize;

   static struct pattern1 dgsmon1 =
   {
/* Set interior style to solid:                                     */
   { 0x0008, SE_IS, SOLIDINT },

/* Set interior color index to white:                               */
   { 0x0008, CE_INCI, WHITEI },

/* Display 2 white rectangles on top of screen:                     */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 961., 0., 0., 1023., 0.,
     479., 1023., 0., 479., 961., 0., 0., 961., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 800., 768., 0., 800., 832.,
     0., 1279., 832., 0., 1279., 768., 0., 800., 768., 0. },

/* Display 6 verticle color bars across center screen:              */
/* Display green color bar:                                         */
/* Set interior color index to green:                               */
   { 0x0008, CE_INCI, GREENI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 256., 0.,
     0., 767., 0., 159., 767., 0., 159., 256., 0., 0., 256., 0. },
/* Display yellow color bar:                                        */
/* Set interior color index to yellow:                              */
   { 0x0008, CE_INCI, YELLOWI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 160., 256., 0., 160.,
     767., 0., 319., 767., 0., 319., 256., 0., 160., 256., 0. },
/* Display red color bar:                                           */
/* Set interior color index to red:                                 */
   { 0x0008, CE_INCI, REDI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 320., 256., 0., 320.,
     767., 0., 479., 767., 0., 479., 256., 0., 320., 256., 0. },
/* Display magenta color bar:                                       */
/* Set interior color index to magenta:                             */
   { 0x0008, CE_INCI, MAGENTAI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 480., 256., 0., 480.,
     767., 0., 639., 767., 0., 639., 256., 0., 480., 256., 0. },
/* Display blue color bar:                                          */
/* Set interior color index to blue:                                */
   { 0x0008, CE_INCI, BLUEI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 640., 256., 0., 640.,
     767., 0., 799., 767., 0., 799., 256., 0., 640., 256., 0. },
/* Display cyan color bar:                                          */
/* Set interior color index to cyan:                                */
   { 0x0008, CE_INCI, CYANI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 800., 256., 0., 800.,
     767., 0., 959., 767., 0., 959., 256., 0., 800., 256., 0. },

/* Display white color bar:                                         */
/* Set interior color index to white:                               */
   { 0x0008, CE_INCI, WHITEI },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 1120., 256., 0., 1120.,
     767., 0., 1279., 767., 0., 1279., 256., 0., 1120., 256., 0. },

/*  Display gray scale below color bars.                                 */
/* Display white:                                                        */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 128., 0.,
     0., 255., 0., 159., 255., 0., 159., 128., 0., 0., 128., 0. },
/* Display gray1:                                                        */
/* Set interior color index to gray1:                               */
   { 0x0008, CE_INCI, GRAY1I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 160., 128., 0., 160.,
     255., 0., 319., 255., 0., 319., 128., 0., 160., 128., 0. },
/* Display gray2:                                                        */
/* Set interior color index to gray2:                               */
   { 0x0008, CE_INCI, GRAY2I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 320., 128., 0., 320.,
     255., 0., 479., 255., 0., 479., 128., 0., 320., 128., 0. },
/* Display gray3:                                                        */
/* Set interior color index to gray3:                               */
   { 0x0008, CE_INCI, GRAY3I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 480., 128., 0., 480.,
     255., 0., 639., 255., 0., 639., 128., 0., 480., 128., 0. },
/* Display gray4:                                                        */
/* Set interior color index to gray4:                               */
   { 0x0008, CE_INCI, GRAY4I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 640., 128., 0., 640.,
     255., 0., 799., 255., 0., 899., 128., 0., 640., 128., 0. },
/* Display gray5:                                                        */
/* Set interior color index to gray5:                               */
   { 0x0008, CE_INCI, GRAY5I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 800., 128., 0., 800.,
     255., 0., 959., 255., 0., 959., 128., 0., 800., 128., 0. },
/* Display gray6:                                                        */
/* Set interior color index to gray6:                               */
   { 0x0008, CE_INCI, GRAY6I },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 960., 128., 0., 960.,
     255., 0., 1119., 255., 0., 1119., 128., 0., 960., 128., 0. },

/*  Display 'piano keyboard' across bottom of screen.                     */
/* Set interior color index to white:                               */
   { 0x0008, CE_INCI, WHITEI },
/*  Display 2 rectangles (53 by 128 pixels):                              */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 0., 0., 0., 0., 128., 0.,
     53., 128., 0., 53., 0., 0., 0., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 106., 0., 0., 106., 128.,
     0., 159., 128., 0., 159., 0., 0., 106., 0., 0. },
/*  Display 4 rectangles (40 by 128 pixels):                              */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 212., 0., 0., 212., 128.,
     0., 252., 128., 0., 252., 0., 0., 212., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 292., 0., 0., 292., 128.,
     0., 332., 128., 0., 332., 0., 0., 292., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 372., 0., 0., 372., 128.,
     0., 412., 128., 0., 412., 0., 0., 372., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 452., 0., 0., 452., 128.,
     0., 492., 128., 0., 492., 0., 0., 452., 0., 0. },
/*  Display 6 rectangles (20 by 128 pixels):                              */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 532., 0., 0., 532., 128.,
     0., 552., 128., 0., 552., 0., 0., 532., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 572., 0., 0., 572., 128.,
     0., 592., 128., 0., 592., 0., 0., 572., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 612., 0., 0., 612., 128.,
     0., 632., 128., 0., 632., 0., 0., 612., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 652., 0., 0., 652., 128.,
     0., 672., 128., 0., 672., 0., 0., 652., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 692., 0., 0., 692., 128.,
     0., 712., 128., 0., 712., 0., 0., 692., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 732., 0., 0., 732., 128.,
     0., 752., 128., 0., 752., 0., 0., 732., 0., 0. },
/*  Display 6 rectangles (10 by 128 pixels):                             */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 772., 0., 0., 772., 128.,
     0., 782., 128., 0., 782., 0., 0., 772., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 792., 0., 0., 792., 128.,
     0., 802., 128., 0., 802., 0., 0., 792., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 812., 0., 0., 812., 128.,
     0., 822., 128., 0., 822., 0., 0., 812., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 832., 0., 0., 832., 128.,
     0., 842., 128., 0., 842., 0., 0., 832., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 852., 0., 0., 852., 128.,
     0., 862., 128., 0., 862., 0., 0., 852., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 872., 0., 0., 872., 128.,
     0., 882., 128., 0., 882., 0., 0., 872., 0., 0. },
/*  Display 6 rectangles (5 by 128 pixels):                              */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 892., 0., 0., 892., 128.,
     0., 897., 128., 0., 897., 0., 0., 892., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 902., 0., 0., 902., 128.,
     0., 907., 128., 0., 907., 0., 0., 902., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 912., 0., 0., 912., 128.,
     0., 917., 128., 0., 917., 0., 0., 912., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 922., 0., 0., 922., 128.,
     0., 927., 128., 0., 927., 0., 0., 922., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 932., 0., 0., 932., 128.,
     0., 937., 128., 0., 937., 0., 0., 932., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 942., 0., 0., 942., 128.,
     0., 947., 128., 0., 947., 0., 0., 942., 0., 0. },
/*  Display 6 rectangles (3 by 128 pixels):                              */
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 952., 0., 0., 952., 128.,
     0., 955., 128., 0., 955., 0., 0., 952., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 958., 0., 0., 958., 128.,
     0., 961., 128., 0., 961., 0., 0., 958., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 964., 0., 0., 964., 128.,
     0., 967., 128., 0., 967., 0., 0., 964., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 970., 0., 0., 970., 128.,
     0., 973., 128., 0., 973., 0., 0., 970., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 976., 0., 0., 976., 128.,
     0., 979., 128., 0., 979., 0., 0., 976., 0., 0. },
   { 0x0048, SE_PG3, 0x0, 0x0, 0x40, 982., 0., 0., 982., 128.,
     0., 985., 128., 0., 985., 0., 0., 982., 0., 0. },

/* Display 9 single pixels in lower right corner of screen.         */
   { 0x0014, SE_PL2, 1125., 32., 1125., 32. },
   { 0x0014, SE_PL2, 1165., 32., 1165., 32. },
   { 0x0014, SE_PL2, 1205., 32., 1205., 32. },
   { 0x0014, SE_PL2, 1125., 64., 1125., 64. },
   { 0x0014, SE_PL2, 1165., 64., 1165., 64. },
   { 0x0014, SE_PL2, 1205., 64., 1205., 64. },
   { 0x0014, SE_PL2, 1125., 96., 1125., 96. },
   { 0x0014, SE_PL2, 1165., 96., 1165., 96. },
   { 0x0014, SE_PL2, 1205., 96., 1205., 96. },
};

ssize = sizeof(dgsmon1);
wfifo(0,&dgsmon1, ssize ,gmbase);
}
