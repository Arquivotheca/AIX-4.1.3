/* @(#)70	1.3  src/bos/usr/lib/nls/loc/jim/jmnt/kjdebug.h, libKJI, bos411, 9428A410j 6/6/91 14:30:55 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _kj_debug
#define _kj_debug
/*
        Program definition list.
*/
#define SNAP_CLOSE  0L
#define SNAP_OPEN   1L
#define SNAP_SELECT 2L
#define SNAP_LEVEL  3L
#define SNAP_DUMP   4L
#define SNAP_FUNC   5L
#define SNAP_NAME   6L

#define SNAP_DECB  0x00000001
#define SNAP_EXT   0x00000002
#define SNAP_KCB   0x00000004
#define SNAP_TRB   0x00000008
#define SNAP_KMISA 0x00000010
#define SNAP_KKCB  0x00000020
#define SNAP_FSB   0x00000040
#define SNAP_ALL   0x0000007f

#define SNAP_UBASE   60000
#define SNAP_USER(x) (SNAP_UBASE+x)

#define SNAP_deacif_  0
#define SNAP_declos_  1
#define SNAP_dedaif_  2
#define SNAP_dedfif_  3
#define SNAP_deerif_  4
#define SNAP_deeviq_  5
#define SNAP_deevwt_  6
#define SNAP_deflev_  7
#define SNAP_deopen_  8
#define SNAP_derdrw_  9
#define SNAP_dersif_  10
#define SNAP_derssh_  11
#define SNAP_desmpl_  12
#define SNAP_destrk_  13
#define SNAP_desvft_  14
#define SNAP_desvlg_  15
#define SNAP_desvln_  16
#define SNAP_destbp_  17

#define SNAP_Eax1ap  100
#define SNAP_Eax1cr  101
#define SNAP_Eax1ec  102
#define SNAP_Eax2ap  103
#define SNAP_Eax2cr  104
#define SNAP_Eax2ec  105
#define SNAP_Eax3ap  106
#define SNAP_Eax3cr  107
#define SNAP_Eax3ec  108
#define SNAP_Eax4ap  109
#define SNAP_Eax4cr  110
#define SNAP_Eax4ec  111
#define SNAP_Eaxap   112
#define SNAP_Eaxec   113
#define SNAP_Ecmd    114
#define SNAP_Econv   115
#define SNAP_Ecur    116
#define SNAP_Eecac   117
#define SNAP_Eeccr   118
#define SNAP_Eecho   119
#define SNAP_Eerfd   120
#define SNAP_Eerrgsl 121
#define SNAP_Ersft   122
#define SNAP_Ebeep   123
#define SNAP_Equery  124

#define SNAP_Jclos   200
#define SNAP_Jclr    201
#define SNAP_Jcrst   202
#define SNAP_Jinit   203
#define SNAP_Jinpr   204
#define SNAP_Jopen   205
#define SNAP_Jshrs   206
#define SNAP_Jterm   207
#define SNAP_Jtrak   208

#define SNAP_MCN_rs  300
#define SNAP_MC_rtn  301
#define SNAP_MD_rtn  302
#define SNAP_ME_01   303
#define SNAP_ME_02   304
#define SNAP_ME_03   305
#define SNAP_ME_04   306
#define SNAP_ME_05   307
#define SNAP_ME_06   308
#define SNAP_ME_07   309
#define SNAP_ME_09   310
#define SNAP_ME_0a   311
#define SNAP_ME_0b   312
#define SNAP_ME_0c   313
#define SNAP_ME_0d   314
#define SNAP_ME_0e   315
#define SNAP_MKL_rs  316
#define SNAP_MK_a2   317
#define SNAP_MK_b4a  318
#define SNAP_MK_b4b  319
#define SNAP_MK_c1   320
#define SNAP_MK_c3   321
#define SNAP_MK_e3   322
#define SNAP_MK_e4   323
#define SNAP_MMSG_rs 324
#define SNAP_MM_09   325
#define SNAP_MM_rtn  326
#define SNAP_MRG_a   327
#define SNAP_MRG_b   328
#define SNAP_MRM_rs  329
#define SNAP_MR_rtn  330
#define SNAP_MS_rtn  331
#define SNAP_Macaxst 332
#define SNAP_Macifst 333
#define SNAP_Maddch  334
#define SNAP_Maxcrmv 335
#define SNAP_Maxmst  336
#define SNAP_Mckbk   337
#define SNAP_Mcrmv   338
#define SNAP_Mdisv   339
#define SNAP_Mexchng 340
#define SNAP_Mfmrst  341
#define SNAP_Mhrktsw 342
#define SNAP_Mifmst  343
#define SNAP_Mindset 344
#define SNAP_Mkanagt 345
#define SNAP_Mkjgrst 346
#define SNAP_Mlock   347
#define SNAP_Mnxprps 348
#define SNAP_Mregrs  349
#define SNAP_Mreset  350
#define SNAP_Mrscvym 351
#define SNAP_Mrstym  352
#define SNAP_Msetch  353
#define SNAP_Mstlcvl 354
#define SNAP_Mstrl   355
#define SNAP_ME_rtn  356
#define SNAP_MK_rtn  357
#define SNAP_Mbsins  358
#define SNAP_Mbsrepn 359
#define SNAP_Mbsrepr 360
#define SNAP_Mdagend 361
#define SNAP_Mdagmsg 362
#define SNAP_Mdagst  363
#define SNAP_Mdaha   364
#define SNAP_Mdelins 365
#define SNAP_Mdelrep 366
#define SNAP_Mecho   367
#define SNAP_Mgetchm 368
#define SNAP_Mhtdc   369
#define SNAP_Minsch  370



/* #(B) 1987.12.15. Fling Conversion Change */
#define SNAP_Minssv  371
#define SNAP_Mkanasd 372
#define SNAP_Mkcnxpr 373
#define SNAP_Mkkcclr 374
#define SNAP_Mktec   375
#define SNAP_Mktnc   376
#define SNAP_Mlfrtc  377
#define SNAP_Mmvch   378
#define SNAP_Mnumgt  379
#define SNAP_Mrepch  380
/* #(E) 1987.12.15. Flying Conversion Change */



#define SNAP_Mrsstrh 381
#define SNAP_Mymstl  382
#define SNAP_Myomic  383
#define SNAP_Mkcflcn 384
#define SNAP_Mnxtpre 385
#define SNAP_Mnxtopn 386



/* #(B) 1987.12.15. Flying Conversion Add */
#define SNAP_Mflypro 387
#define SNAP_Mflycnv 388
#define SNAP_Midecid 389
#define SNAP_Mansave 390
#define SNAP_Mflyrst 391
#define SNAP_MK_c1f  392
#define SNAP_Msglop  393
#define SNAP_Msglfw  394
#define SNAP_Msglbw  395
/* #(E) 1987.12.15. Flying Conversion Add */



#define SNAP_Rkc      400

#define SNAP_Traced  500
#define SNAP_Tracef  501
#define SNAP_Tracep  502

/*
 *      Macros. For Debugging.
 */
#if NDEBUG || lint
/*
 *      Ignore Debuggin Macros.
 */
#define PRINT(x,f)
#define CPRINT(x)
#define FPRINT(func)
#define ASSERT(exp)
#define snap(mode,file)
#define snap2(mode)
#define snap3(mode,id,cmd)

#else
/*
 *      Debugging Print Limited Confirm.
 */
#ifdef  NDEBUGPRINT
#define PRINT(var,form)
#define CPRINT(string)
#define ASSERT(exp)
#else
#define PRINT(var,form) xfprintf(1,"%s:var=%form\n",__FILE__,(int)var)
#define CPRINT(string)  xfprintf(1,"string\n",__FILE__,NULL)
#define ASSERT(exp)                                             \
        ( (exp) ? 0: fprintf(stderr,"Fail exp %-14.14s Line %5d\n"\
                              ,__FILE__,__LINE__)               \
        )
#endif

/*
 *      Debugging Snap Shot Dump Confirm.
 */
#ifdef  NDEBUGSNAP
#define snap(mode,file)
#define snap2(mode)
#define snap3(mode,id,cmd)
#else
#define snap(ctl,file)     snapf(SNAP_OPEN,(char *)ctl,(char *)file);
#define snap2(mode)        snapf(SNAP_DUMP,mode,__FILE__,(long)__LINE__)
#ifdef  NDEBUGPRINT
#define snap3(mode,id,cmt) snapf(SNAP_FUNC,mode,__FILE__,(long)id,cmt)
#else
#define snap3(mode,id,cmt) snapf(SNAP_FUNC,mode,__FILE__,(long)id,cmt),\
                           xfprintf(1,"%s\n",__FILE__,NULL)
#endif
#endif

/*
 *      Dummy Function Generete.
 */
#define FPRINT(fn)                                              \
int fn() {                                                      \
        xfprintf(1,"%s:fn call.\n",__FILE__,NULL);              \
        return(0);                                              \
}
#endif
#endif

