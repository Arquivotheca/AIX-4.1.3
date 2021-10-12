static char sccsid[] = "@(#)87	1.1  src/bos/diag/tu/gga/scroll2tu.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:25";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "exectu.h"
#include "ggamisc.h"
#include "tu_type.h"

/*IBM Confidential                                                           */
/*This file contains routines used in GARDIAN to test the White Oak Graphics */
/* Adapter (GGA).                                                            */

extern unsigned int gga_x_max, gga_y_max;


int scroll_h_pixel1(void)
  {
    unsigned long dummy, inchar, problems=0, base, orig_base, temp1, *pfont;

/* H17_640x480:  "H" font for 17" screen: 640x480(8x14), 800x600(8x14) */
/*               and for a  21" screen: 640x480(8x14), 800x600(8x14).  */
    unsigned long H17_640x480[4]={ 0x00008282, 0x8282fe82, 0x82828200, 0x0};

/* H17_1024x768: "H" font for 17" screen: 1024x768(10x17) */
    unsigned long H17_1024x768[6]={ 0x00000001, 0x86619866, 0x19867f98,
                                    0x66198661, 0x98600000, 0x0};

/* H17_1280x1024: "H" font for 17" screen: 1280x1024(12x21) */
    unsigned long H17_1280x1024[8]={ 0x00000000, 0x00006066, 0x06606606,
                                     0x6067fe7f, 0xe6066066, 0x06606606,
                                     0x60600000, 0x0};

/* H17_1360x1024: "H" font for 17" screen: 1360x1024(13x21) */
    unsigned long H17_1360x1024[8]={ 0x00000000, 0x00000603, 0x301980cc,
                                     0x066033ff, 0x9ffcc066, 0x03301880,
                                     0xcc066030, 0x0};

    int  x0, x1, y1, x2, y3, i, z, t, busy, rc = SUCCESS;
    int  pixel_jump, width, height, mode, mode_count;
    int  font_width, font_height, no_words;
    int  scroll_index, num_lines, initial_mode;

    initial_mode = display_mode;

    for (mode_count = LOW_RES; mode_count <= HIGHEST_RES; mode_count++)
      {
        switch(mode_count)
          {
          case 0: mode = MODE640X480X8_60HZ; break;
          case 1: mode = MODE1024X768X8;     break;
          case 2: mode = MODE1280X1024X8;    break;
          case 3: mode = MODE1600X1280X8;    break;
          default: return(FAIL);
          }

        if(modeset(mode) == FAIL)
          {
            set_tu_errno();
            rc = SET_MODE_ERR;
            LOG_SYSERR("Failed to set display mode in scroll_h_pixel1()");
            return(rc);
          }
        sleep(2);
        gga_cls(BLACK);

        WL(gga_Get_Color(WHITE), W9100_COLOR_1);
        WL(gga_Get_Color(BLACK), W9100_COLOR_0);

        WL((ULONG) 0xFFFFFFFF, W9100_PLANE_MASK);
        wait_for_wtkn_ready();
        WL((SMASK), W9100_RASTER);
        wait_for_wtkn_ready();

        /* Select font parameters for the various modes. */
        if( (gga_x_max == 640 && gga_y_max == 480) ||
            (gga_x_max == 800 && gga_y_max == 600) )
          {
             font_width = 8;
             font_height = 14;
             no_words = 4;
             pfont = H17_640x480;
             scroll_index = 4;
          }
        else if(gga_x_max == 1024 && gga_y_max == 768)
          {
             font_width = 10;
             font_height = 17;
             no_words = 6;
             pfont = H17_1024x768;
             scroll_index = 3;
          }
        else if(gga_x_max == 1280 && gga_y_max == 1024)
          {
             font_width = 12;
             font_height = 21;
             no_words = 8;
             pfont = H17_1280x1024;
             scroll_index = 16;
          }
        else if(gga_x_max == 1360 && gga_y_max == 1024)
          {
             font_width = 13;
             font_height = 21;
             no_words = 8;
             pfont = H17_1360x1024;
             scroll_index = 16;
          }
        else if(gga_x_max == 1600 && gga_y_max == 1280)
          {
             font_width = 13;
             font_height = 21;
             no_words = 8;
             pfont = H17_1360x1024;
             scroll_index = 20;
          }
        else   /* Default to 640x480. */
          {
             font_width = 8;
             font_height = 14;
             no_words = 4;
             pfont = H17_640x480;
             scroll_index = 4;
          }

        width = gga_x_max - 1;
        height = gga_y_max - 1;
        switch(gga_x_max)
          {
            case 640:  num_lines = 35; break;
            case 1024: num_lines = 12; break;
            case 1280: num_lines = 5;  break;
            case 1360: num_lines = 4;  break;
            case 1600: num_lines = 3;  break;
            default:  /* Unexpected gga_x_max = %x ", gga_x_max */
              return(FAIL);
          }

        WL((ULONG) 0, W9100_P_WINMIN);      /* Set clipping to 1 more than  */
        WL((ULONG) 0, W9100_B_WINMIN);      /* normally desired in the      */
        temp1 = (width << 16) | (height+1); /* vertical dimension. This     */
        WL( temp1, W9100_P_WINMAX);         /* allows for smooth scrollong. */
        WL( temp1, W9100_B_WINMAX);

        /***** Fill screen with Hs *****/

        for (y1 = 0; y1 < height; y1 += font_height)
          for (x1 = 0; x1 < width; x1 += font_width)
            {
                x0 = x1;
                x2 = x0 + font_width;
                y3 = 1;
                WL( x0, W9100_COORD_X0); 
                WL( x1, W9100_COORD_X1); 
                WL( y1, W9100_COORD_Y1); 
                WL( x2, W9100_COORD_X2); 
                WL( y3, W9100_COORD_Y3); 

                wait_for_wtkn_ready();

                for( i = 0; i < no_words ; ++ i)
                  {
                    WL(*(pfont+i), (W9100_PIXEL1_CMD | ((32-1)<<2)));
                    wait_for_wtkn_ready();
                  }
            }

        /***** Scroll up a pixel at a time *****/

        WL( 0, W9100_W_OFF_XY);
        wait_for_wtkn_ready();

        Write_Reg_Bitfield( W9100_RASTER, SMASK, MINTERM_FIELD, 16);

        for( t = 0 ; t < num_lines ; t++)
        {
            for(z = 0 ; z < font_height ; z++)
            {
                wait_for_wtkn_ready();

                /* Blit top row of pixels to offscreen VRAM */
                WL(0, W9100_COORD_X0);            
                WL(scroll_index, W9100_COORD_Y0); 
                WL(width, W9100_COORD_X1);        
                WL(scroll_index, W9100_COORD_Y1); 
                WL(0, W9100_COORD_X2);            
                WL((height+1), W9100_COORD_Y2);   
                WL(width, W9100_COORD_X3);        
                WL((height+1), W9100_COORD_Y3);   

                temp1 = RL(W9100_BLIT_CMD); /* Initiate cmd */

                wait_for_wtkn_ready();

                /* Move screen up by one pixel (including offscreen stuff) */
                WL(0, W9100_COORD_X0);          
                WL(1, W9100_COORD_Y0);          
                WL(width, W9100_COORD_X1);      
                WL((height+1), W9100_COORD_Y1); 
                WL(0, W9100_COORD_X2);          
                WL(0, W9100_COORD_Y2);          
                WL(width, W9100_COORD_X3);      
                WL(height, W9100_COORD_Y3);     

                temp1 = RL(W9100_BLIT_CMD); /* Initiate cmd */

                wait_for_wtkn_ready();
            }
        }
        if (monitor_type == TFT_MONITOR)
          break; /* Execute only one loop at low resolution, skip others */
      }

    gga_cls(BLACK);

    if(modeset(initial_mode) == FAIL)
      {
        set_tu_errno();
        rc = SET_MODE_ERR;
        LOG_SYSERR("Failed to reset display mode in scroll_h_pixel1()");
        return(rc);
      }
    sleep(4);
    gga_cls(BLACK);

    width = gga_x_max - 1;
    height = gga_y_max - 1;
    WL((ULONG) 0, W9100_P_WINMIN);  /* Set clipping back to normal */
    WL((ULONG) 0, W9100_B_WINMIN);
    temp1 = (width << 16) | height;
    WL( temp1, W9100_P_WINMAX);
    WL( temp1, W9100_B_WINMAX);

    return(SUCCESS);
}



int scroll_h_pixel1_emc(void)
  {
    unsigned long dummy, inchar, problems=0, base, orig_base, temp1, *pfont;

    /* H17_1280x1024: "H" font for 17" screen: 1280x1024(12x21) */
    unsigned long H17_1280x1024[8]={ 0x00000000, 0x00006066, 0x06606606,
                                     0x6067fe7f, 0xe6066066, 0x06606606,
                                     0x60600000, 0x0};

    int  x0, x1, y1, x2, y3, i, z, t, busy, rc = SUCCESS;
    int  pixel_jump, width, height, mode, mode_count;
    int  font_width, font_height, no_words;
    int  scroll_index, num_lines, initial_mode;

    initial_mode = display_mode;

    if(modeset(MODE1280X1024X8) == FAIL)
      {
        set_tu_errno();
        rc = SET_MODE_ERR;
        LOG_SYSERR("Failed to set display mode in scroll_h_pixel1_emc()");
        return(rc);
      }
    sleep(2);
    gga_cls(BLACK);

    font_width = 12;
    font_height = 21;
    no_words = 8;
    pfont = H17_1280x1024;
    scroll_index = 16;
    num_lines = 5; 

    do
      {
        WL(gga_Get_Color(WHITE), W9100_COLOR_1);
        WL(gga_Get_Color(BLACK), W9100_COLOR_0);

        WL((ULONG) 0xFFFFFFFF, W9100_PLANE_MASK);
        wait_for_wtkn_ready();
        WL((SMASK), W9100_RASTER);
        wait_for_wtkn_ready();

        width = gga_x_max - 1;
        height = gga_y_max - 1;

        WL((ULONG) 0, W9100_P_WINMIN);      /* Set clipping to 1 more than  */
        WL((ULONG) 0, W9100_B_WINMIN);      /* normally desired in the      */
        temp1 = (width << 16) | (height+1); /* vertical dimension. This     */
        WL( temp1, W9100_P_WINMAX);         /* allows for smooth scrollong. */
        WL( temp1, W9100_B_WINMAX);

        /***** Fill screen with Hs *****/

        for (y1 = 0; y1 < height; y1 += font_height)
          for (x1 = 0; x1 < width; x1 += font_width)
            {
                x0 = x1;
                x2 = x0 + font_width;
                y3 = 1;
                WL( x0, W9100_COORD_X0); 
                WL( x1, W9100_COORD_X1); 
                WL( y1, W9100_COORD_Y1); 
                WL( x2, W9100_COORD_X2); 
                WL( y3, W9100_COORD_Y3); 

                wait_for_wtkn_ready();

                for( i = 0; i < no_words ; ++ i)
                  {
                    WL(*(pfont+i), (W9100_PIXEL1_CMD | ((32-1)<<2)));
                    wait_for_wtkn_ready();
                  }
            }

        /***** Scroll up a pixel at a time *****/

        WL( 0, W9100_W_OFF_XY);
        wait_for_wtkn_ready();

        Write_Reg_Bitfield( W9100_RASTER, SMASK, MINTERM_FIELD, 16);

        for( t = 0 ; t < num_lines ; t++)
        {
            for(z = 0 ; z < font_height ; z++)
            {
                wait_for_wtkn_ready();

                /* Blit top row of pixels to offscreen VRAM */
                WL(0, W9100_COORD_X0);            
                WL(scroll_index, W9100_COORD_Y0); 
                WL(width, W9100_COORD_X1);        
                WL(scroll_index, W9100_COORD_Y1); 
                WL(0, W9100_COORD_X2);            
                WL((height+1), W9100_COORD_Y2);   
                WL(width, W9100_COORD_X3);        
                WL((height+1), W9100_COORD_Y3);   

                temp1 = RL(W9100_BLIT_CMD); /* Initiate cmd */

                wait_for_wtkn_ready();

                /* Move screen up by one pixel (including offscreen stuff) */
                WL(0, W9100_COORD_X0);          
                WL(1, W9100_COORD_Y0);          
                WL(width, W9100_COORD_X1);      
                WL((height+1), W9100_COORD_Y1); 
                WL(0, W9100_COORD_X2);          
                WL(0, W9100_COORD_Y2);          
                WL(width, W9100_COORD_X3);      
                WL(height, W9100_COORD_Y3);     

                temp1 = RL(W9100_BLIT_CMD); /* Initiate cmd */

                wait_for_wtkn_ready();
            }
        }
      } while ((!end_tu(get_dply_time())) && (rc == SUCCESS));

    gga_cls(BLACK);

    if(modeset(initial_mode) == FAIL)
      {
        set_tu_errno();
        rc = SET_MODE_ERR;
        LOG_SYSERR("Failed to reset display mode in scroll_h_pixel1_emc()");
        return(rc);
      }
    sleep(4);
    gga_cls(BLACK);

    width = gga_x_max - 1;
    height = gga_y_max - 1;
    WL((ULONG) 0, W9100_P_WINMIN);  /* Set clipping back to normal */
    WL((ULONG) 0, W9100_B_WINMIN);
    temp1 = (width << 16) | height;
    WL( temp1, W9100_P_WINMAX);
    WL( temp1, W9100_B_WINMAX);

    return(SUCCESS);
}

