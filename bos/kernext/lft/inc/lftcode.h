/* @(#)89	1.1  src/bos/kernext/lft/inc/lftcode.h, lftdd, bos411, 9428A410j 10/15/93 14:31:20 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef LFTCODE_H                        /*dont include if already here*/
#define LFTCODE_H 1


                                        /* ASCII STANDARD GRAPHIC CODE VALUES */
#define IC_SP      0x20                     /* SPACE */
#define IC_EXC     0x21                    /* EXCLAMATION MARK */
#define IC_2QUOT   0x22                  /* DOUBLE QUOTE */
#define IC_POUND   0x23                  /* POUND SIGN */
#define IC_DOLLAR  0x24                 /* DOLLAR SIGN */
#define IC_PERCENT 0x25                /* PERCENT */
#define IC_AMP     0x26                    /* AMPERSAND */
#define IC_1QUOT   0x27                  /* LEFT SINGLE QUOTE */
#define IC_LPAR    0x28                   /* LEFT PARENTHESIS */
#define IC_RPAR    0x29                   /* RIGHT PARENTHESIS */
#define IC_AST     0x2a                    /* ASTERISK */
#define IC_PLUS    0x2b                   /* PLUS SIGN */
#define IC_COM     0x2c                    /* COMMA */
#define IC_DASH    0x2d                   /* DASH */
#define IC_PERIOD  0x2e                 /* PERIOD */
#define IC_SLASH   0x2f                  /* SLASH */
#define IC_0       0x30
#define IC_1       0x31
#define IC_2       0x32
#define IC_3       0x33
#define IC_4       0x34
#define IC_5       0x35
#define IC_6       0x36
#define IC_7       0x37
#define IC_8       0x38
#define IC_9       0x39
#define IC_COLON   0x3a                  /* COLON */
#define IC_SEMI    0x3b                   /* SEMICOLON */
#define IC_LT      0x3c                     /* LESS THAN SIGN */
#define IC_EQ      0x3d                     /* EQUALS SIGN */
#define IC_GT      0x3e                     /* GREATER THAN SIGN */
#define IC_QUES    0x3f                   /* QUESTION MARK */
#define IC_AT      0x40                     /* AT SIGN */
#define IC_UCA     0x41
#define IC_UCB     0x42
#define IC_UCC     0x43
#define IC_UCD     0x44
#define IC_UCE     0x45
#define IC_UCF     0x46
#define IC_UCG     0x47
#define IC_UCH     0x48
#define IC_UCI     0x49
#define IC_UCJ     0x4a
#define IC_UCK     0x4b
#define IC_UCL     0x4c
#define IC_UCM     0x4d
#define IC_UCN     0x4e
#define IC_UCO     0x4f
#define IC_UCP     0x50
#define IC_UCQ     0x51
#define IC_UCR     0x52
#define IC_UCS     0x53
#define IC_UCT     0x54
#define IC_UCU     0x55
#define IC_UCV     0x56
#define IC_UCW     0x57
#define IC_UCX     0x58
#define IC_UCY     0x59
#define IC_UCZ     0x5a
#define IC_LSB     0x5b                    /* LEFT SQUARE BRACKET */
#define IC_BSLASH  0x5c                 /* BACK SLASH */
#define IC_RSB     0x5d                    /* RIGHT SQUARE BRACKET */
#define IC_AND     0x5e                    /* INVERTED V */
#define IC__       0x5f                      /* UNDERSCORE */
#define IC_LQUOT   0x60
#define IC_LCA     0x61
#define IC_LCB     0x62
#define IC_LCC     0x63
#define IC_LCD     0x64
#define IC_LCE     0x65
#define IC_LCF     0x66
#define IC_LCG     0x67
#define IC_LCH     0x68
#define IC_LCI     0x69
#define IC_LCJ     0x6a
#define IC_LCK     0x6b
#define IC_LCL     0x6c
#define IC_LCM     0x6d
#define IC_LCN     0x6e
#define IC_LCO     0x6f
#define IC_LCP     0x70
#define IC_LCQ     0x71
#define IC_LCR     0x72
#define IC_LCS     0x73
#define IC_LCT     0x74
#define IC_LCU     0x75
#define IC_LCV     0x76
#define IC_LCW     0x77
#define IC_LCX     0x78
#define IC_LCY     0x79
#define IC_LCZ     0x7a
#define IC_LBRACE  0x7b                 /* LEFT CURLY BRACE */
#define IC_OR      0x7c                     /* OR SYMBOL */
#define IC_RBRACE  0x7d                 /* RIGHT CURLY BRACE */
#define IC_APPROX  0x7e                 /* APPROXIMATELY */
#define IC_DEL     0x7f                    /*       ?       */
#define IC_UCCCED  0x80                 /* UPPER CASE C CEDILLA */
#define IC_LCUUML  0x81                 /* LOWER CASE U UMLAUT */
#define IC_LCEACC  0x82                 /* LOWER CASE E ACCENT (ACUTE) */
#define IC_LCACRF  0x83                 /* lower case a circumflex */
#define IC_LCAUML  0x84                 /* lower case a umlaut */
#define IC_LCAGRV  0x85                 /* lower case a grave */
#define IC_LCAOVC  0x86                 /* lower case a overcircle */
#define IC_LCCCED  0x87                 /* lower case c cedilla */
#define IC_LCECRF  0x88                 /* lower case e circumflex */
#define IC_LCEUML  0x89                 /* lower case e umlaut */
#define IC_LCEGRV  0x8a                 /* lower case e grave */
#define IC_LCIUML  0x8b                 /* lower case i umlaut */
#define IC_LCICRF  0x8c                 /* lower case i circumflex */
#define IC_LCIGRV  0x8d                 /* lower case i grave */
#define IC_UCAUML  0x8e                 /* upper case a umlaut */
#define IC_UCAOVC  0x8f                 /* upper case a overcircle */
#define IC_UCEACC  0x90                 /* upper case e acute */
#define IC_LCAE    0x91                   /* lower case ae */
#define IC_UCAE    0x92                   /* upper case ae */
#define IC_LCOCRF  0x93                 /* lower case o circumflex */
#define IC_LCOUML  0x94                 /* lower case o umlaut */
#define IC_LCOGRV  0x95                 /* lower case o grave */
#define IC_LCUCRF  0x96                 /* lower case u circumflex */
#define IC_LCUGRV  0x97                 /* lower case u grave */
#define IC_LCYUML  0x98                 /* lower case y umlaut */
#define IC_UCOUML  0x99                 /* upper case o umlaut */
#define IC_UCUUML  0x9a                 /* upper case u umlaut */
#define IC_LCOSLS  0x9b                 /* lower case o slash */
#define IC_STRLNG  0x9c                 /* Pounds Sterling */
#define IC_UCOSLS  0x9d                 /* upper case o slash */
#define IC_MULTPL  0x9e                 /* multiply symbol */
#define IC_FLORIN  0x9f                 /* florin sign */
#define IC_LCAACC  0xa0                 /* lower case a acute */
#define IC_LCIACC  0xa1                 /* lower case i acute */
#define IC_KUTEN   0xa1                  /* kuten */
#define IC_LCOACC  0xa2                 /* lower case o acute */
#define IC_HAJKKK  0xa2                 /* hajime kagi kakko */
#define IC_LCUACC  0xa3                 /* lower case u acute */
#define IC_OWAKKK  0xa3                 /* owari kaji kakko   */
#define IC_LCNTIL  0xa4                 /* lower case n tilde */
#define IC_TOUTEN  0xa4                 /* touten */
#define IC_UCNTIL  0xa5                 /* upper case N tilde */
#define IC_CHUUTN  0xa5                 /* chuuten */
#define IC_LCAUND  0xa6                 /* lower case a underscore */
#define IC_KWO     0xa6                    /* Katakana wo */
#define IC_LCOUND  0xa7                 /* lower case o underscore */
#define IC_SKA     0xa7                    /* small Katakana a */
#define IC_INVQUES 0xa8                /* inverted question mark */
#define IC_SKI     0xa8                    /* small Katakana i */
#define IC_REGTRD  0xa9                 /* registered trademark */
#define IC_SKU     0xa9                    /* small Katakana u */
#define IC_NOTSIGN 0xaa                /* not sign */
#define IC_SKE     0xaa                    /* small Katakana e */
#define IC_HALF    0xab                   /* one half */
#define IC_SKO     0xab                    /* small Katakana o */
#define IC_QUARTER 0xac                /* one quarter */
#define IC_SKYA    0xac                   /* small Katakana ya */
#define IC_INVEXCL 0xad                /* inverted exclamation point */
#define IC_SKYU    0xad                   /* small Katakana yu */
#define IC_LANGQUO 0xae                /* left angle quotes */
#define IC_SKYO    0xae                   /* small Katakana yo */
#define IC_RANGQUO 0xaf                /* right angle quotes */
#define IC_SKTU    0xaf                   /* small Katakana tu */
#define IC_QUAHASH 0xb0                /* quarter hashed */
#define IC_CHONKGO 0xb0                /* choon kigo */
#define IC_HAFHASH 0xb1                /* half hashed */
#define IC_KA      0xb1                     /* Katakana a */
#define IC_FULHASH 0xb2                /* full hashed */
#define IC_KI      0xb2                     /* Katakana i */
#define IC_VERTBAR 0xb3                /* vertical bar */
#define IC_KU      0xb3                     /* Katakana u */
#define IC_LARM    0xb4                   /* vertical bar w/left arm */
#define IC_KE      0xb4                     /* Katakana e */
#define IC_UCAACC  0xb5                 /* upper case a acute */
#define IC_KO      0xb5                     /* Katakana o */
#define IC_UCACRF  0xb6                 /* upper case a circumflex */
#define IC_KKA     0xb6                    /* Katakana ka */
#define IC_UCAGRV  0xb7                 /* upper case a grave */
#define IC_KKI     0xb7                    /* Katakana ki */
#define IC_COPYRT  0xb8                 /* copyright symbol */
#define IC_KKU     0xb8                    /* Katakana ku */
#define IC_KKE     0xb9                    /* Katakana ke */
#define IC_KKO     0xba                    /* Katakana ko */
#define IC_KSA     0xbb                    /* Katakana sa */
#define IC_KASI    0xbc                   /* Katakana si */
#define IC_CENT    0xbd                   /* cents sign */
#define IC_KSU     0xbd                    /* Katakana su */
#define IC_YEN     0xbe                    /* Yen sign */
#define IC_KSE     0xbe                    /* Katakana se */
#define IC_URCORN  0xbf                 /* upper right corner */
#define IC_KSO     0xbf                    /* Katakana so */
#define IC_LLCORN  0xc0                 /* lower left corner */
#define IC_KTA     0xc0                    /* Katakana ta */
#define IC_INVTEE  0xc1                 /* inverted tee */
#define IC_KTI     0xc1                    /* Katakana t */
#define IC_TEE     0xc2                    /* upright tee */
#define IC_KTU     0xc2                    /* Katakana tu */
#define IC_RARM    0xc3                   /* vertical bar w/right arm */
#define IC_KTE     0xc3                    /* Katakana te */
#define IC_HORZBAR 0xc4                /* horizontal bar */
#define IC_KTO     0xc4                    /* Katakana to */
#define IC_CROSS   0xc5                  /* crossed bars */
#define IC_KNA     0xc5                    /* Katakana na */
#define IC_LCATIL  0xc6                 /* lower case a tilde */
#define IC_KNI     0xc6                    /* Katakana ni */
#define IC_UCATIL  0xc7                 /* upper case a tilde */
#define IC_KNU     0xc7                    /* Katakana nu */
#define IC_KNE     0xc8                    /* Katakana ne */
#define IC_KNO     0xc9                    /* Katakana no */
#define IC_KHA     0xca                    /* Katakana ha */
#define IC_KHI     0xcb                    /* Katakana hi */
#define IC_KFU     0xcc                    /* Katakana fu */
#define IC_KHE     0xcd                    /* Katakana he */
#define IC_KHO     0xce                    /* Katakana ho */
#define IC_INTLCU  0xcf                 /* international currency */
#define IC_KMA     0xcf                    /* Katakana ma */
#define IC_LCEICE  0xd0                 /* lower case eth icelandic */
#define IC_KMI     0xd0                    /* Katakana mi */
#define IC_UCEICE  0xd1                 /* upper case eth icelandic */
#define IC_KMU     0xd1                    /* Katakana mu */
#define IC_UCECRF  0xd2                 /* upper case e circumflex */
#define IC_KME     0xd2                    /* Katakana me */
#define IC_UCEUML  0xd3                 /* upper case e umlaut */
#define IC_KMO     0xd3                    /* Katakana mo */
#define IC_UCEGRV  0xd4                 /* upper case e grave */
#define IC_KYA     0xd4                    /* Katakana ya */
#define IC_LCIDTL  0xd5                 /* lower case i dotless */
#define IC_KYU     0xd5                    /* Katakana yu */
#define IC_UCIACC  0xd6                 /* upper case i acute */
#define IC_KYO     0xd6                    /* Katakana yo */
#define IC_UCICRF  0xd7                 /* upper case i circumflex */
#define IC_KRA     0xd7                    /* Katakana ra */
#define IC_UCIUML  0xd8                 /* upper case i umlaut */
#define IC_KRI     0xd8                    /* Katakana ri */
#define IC_LRCORN  0xd9                 /* lower right corner */
#define IC_KRU     0xd9                    /* Katakana ru */
#define IC_ULCORN  0xda                 /* upper left corner */
#define IC_KRE     0xda                    /* Katakana re */
#define IC_BRITE   0xdb                  /* bright character cell */
#define IC_KRO     0xdb                    /* Katakana ro */
#define IC_BOTBRIT 0xdc                /* bottom bright character cell */
#define IC_KWA     0xdc                    /* Katakana wa */
#define IC_VLBROK  0xdd                 /* vetical line broken */
#define IC_KN      0xdd                     /* Katakana n */
#define IC_RBRITE  0xde                 /* right half bright char cell */
#define IC_UCIGRV  0xde                 /* upper case i grave */
#define IC_DAKUTEN 0xde                /* dakuten */
#define IC_TOPBRIT 0xdf                /* top bright character cell */
#define IC_HANDAKU 0xdf                /* handakuten */
#define IC_LCALPHA 0xe0                /* lower case alpha */
#define IC_UCOACC  0xe0                 /* upper case o acute */
#define IC_LCSSHP  0xe1                 /* German double s       */
#define IC_LCGAMM  0xe2                 /* lower case gamma */
#define IC_UCOCRF  0xe2                 /* upper case o circumflex */
#define IC_LCPI    0xe3                   /* lower case pi */
#define IC_UCOGRV  0xe3                 /* upper case o grave */
#define IC_UCSIGM  0xe4                 /* upper case sigma */
#define IC_LCOTIL  0xe4                 /* lower case o tilde */
#define IC_LCSIGM  0xe5                 /* lower case sigma */
#define IC_UCOTIL  0xe5                 /* upper case o tilde */
#define IC_LCMU    0xe6                   /* lower case mu */
#define IC_LCTICE  0xe7                 /* lower case thorn icelandic */
#define IC_UCTICE  0xe8                 /* upper case thorn icelandic */
#define IC_UCTHET  0xe9                 /* lower case theta */
#define IC_UCUACC  0xe9                 /* upper case u acute */
#define IC_OMEGA   0xea                  /* lower case sigma */
#define IC_UCUCRF  0xea                 /* upper case u circumflex */
#define IC_LCDELT  0xeb                 /* lower case sigma */
#define IC_UCUGRV  0xeb                 /* upper case u grave */
#define IC_INFINI  0xec                 /* lower case sigma */
#define IC_LCYACC  0xec                 /* lower case y acute */
#define IC_LCPHI   0xed                  /* lower case sigma */
#define IC_UCYACC  0xed                 /* upper case y acute */
#define IC_OVRBAR  0xee                 /* overbar */
#define IC_INTRSC  0xef                 /* lower case sigma */
#define IC_ACUTE   0xef                  /* acute diacritic */
#define IC_SYLHYP  0xf0                 /* syllable hyphen */
#define IC_PLSMNS  0xf1                 /* plus-minus       */
#define IC_DBLUND  0xf2                 /* double underscore */
#define IC_3QUART  0xf3                 /* three quarters */
#define IC_PARA    0xf4                   /* paragraph symbol */
#define IC_SECT    0xf5                   /* section symbol */
#define IC_DIVIDE  0xf6                 /* division symbol */
#define IC_DBLEQU  0xf7                 /* double equals */
#define IC_CEDILLA 0xf7                /* cedilla diacritic */
#define IC_DEGREE  0xf8                 /* degree symbol */
#define IC_UPDOTF  0xf9                 /* filled upper dot */
#define IC_UMLAUT  0xf9                 /* umlaut diacritic */
#define IC_MDDOTF  0xfa                 /* filled middle dot */
#define IC_SSONE   0xfb                  /* superscript 1 */
#define IC_SSTHRE  0xfc                 /* superscript 3 */
#define IC_SSTWO   0xfd                  /* superscript 2 */
#define IC_VTRECF  0xfe                 /* filled vertical rectangle */
#define IC_FOXSP   0xff                  /* space */


  /***************below are p1 offsets**************************/
#define IC_PARA_OLD   0x34               /* paragraph symbol */
#define IC_SECT_OLD   0x35               /* section symbol */
#define IC_LCATIL_OLD 0x40             /* lower case a tilde */
#define IC_LCSSHP_OLD 0x41             /* lower case s sharp */
#define IC_UCACRF_OLD 0x42             /* upper case a circumflex */
#define IC_UCAGRV_OLD 0x43             /* upper case a grave */
#define IC_UCAACC_OLD 0x44             /* upper case a acute */
#define IC_UCATIL_OLD 0x45             /* upper case a tilde */
#define IC_LCOSLS_OLD 0x46             /* lower case o slash */
#define IC_UCECRF_OLD 0x47             /* upper case e circumflex */
#define IC_UCEUML_OLD 0x48             /* upper case e umlaut */
#define IC_UCEGRV_OLD 0x49             /* upper case e grave */
#define IC_UCIACC_OLD 0x4a             /* upper case i acute */
#define IC_UCICRF_OLD 0x4b             /* upper case i circumflex */
#define IC_UCIUML_OLD 0x4c             /* upper case i umlaut */
#define IC_UCIGRV_OLD 0x4d             /* upper case i grave */
#define IC_UCOSLS_OLD 0x4e             /* upper case o slash */
#define IC_LCEICE_OLD 0x4f             /* lower case eth icelandic */
#define IC_LCYACC_OLD 0x50             /* lower case y acute */
#define IC_LCTICE_OLD 0x51             /* lower case thorn icelandic */
#define IC_CEDILLA_OLD 0x52            /* cedilla diacritic */
#define IC_INTLCU_OLD 0x53             /* international currency */
#define IC_UCEICE_OLD 0x54             /* upper case eth icelandic */
#define IC_UCYACC_OLD 0x55             /* upper case y acute */
#define IC_UCTICE_OLD 0x56             /* upper case thorn icelandic */
#define IC_REGTRD_OLD 0x57             /* registered trademark */
#define IC_3QUART_OLD 0x58             /* three quarters */
#define IC_OVRBAR_OLD 0x59             /* overbar */
#define IC_UMLAUT_OLD 0x5a             /* umlaut diacritic */
#define IC_ACUTE_OLD  0x5b              /* acute diacritic */
#define IC_DBLUND_OLD 0x5c             /* double underscore */
#define IC_LCOTIL_OLD 0x5d             /* lower case o tilde */
#define IC_LCIDTL_OLD 0x5e             /* lower case i dotless */
#define IC_UCOCRF_OLD 0x5f             /* upper case o circumflex */
#define IC_UCOGRV_OLD 0x60             /* upper case o grave */
#define IC_UCOACC_OLD 0x61             /* upper case o acute */
#define IC_UCOTIL_OLD 0x62             /* upper case o tilde */
#define IC_SSTHRE_OLD 0x63             /* superscript 3 */
#define IC_UCUCRF_OLD 0x64             /* upper case u circumflex */
#define IC_UCUGRV_OLD 0x65             /* upper case u grave */
#define IC_UCUACC_OLD 0x66             /* upper case u acute */
#define IC_LCAOGO 0x67                 /* lower case o ogonek */
#define IC_LCECAR 0x68                 /* lower case e caron */
#define IC_LCCCAR 0x69                 /* lower case c caron */
#define IC_LCCACC 0x6a                 /* lower case c acute */
#define IC_LCEOGO 0x6b                 /* lower case e ogonek */
#define IC_LCUOVC 0x6c                 /* lower case u overcircle */
#define IC_LCDCAR 0x6d                 /* lower case d caron */
#define IC_LCLACC 0x6e                 /* lower case l acute */
#define IC_UCAOGO 0x6f                 /* upper case a ogonek */
#define IC_UCECAR 0x70                 /* upper case e caron */
#define IC_UCCCAR 0x71                 /* upper case c caron */
#define IC_UCCACC 0x72                 /* upper case c acute */
#define IC_CARON  0x73                  /* caron diacritic */
#define IC_UCEOGO 0x74                 /* upper case e ogonek */
#define IC_UCUOVC 0x75                 /* upper case u overcircle */
#define IC_UCDCAR 0x76                 /* upper case d caron */
#define IC_UCLACC 0x77                 /* upper case l acute */
#define IC_LCLCAR 0x78                 /* lower case l caron */
#define IC_LCNCAR 0x79                 /* lower case n caron */
#define IC_LCDSTK 0x7a                 /* lower case d stroke */
#define IC_LCRCAR 0x7b                 /* lower case r caron */
#define IC_LCSACC 0x7c                 /* lower case s acute */
#define IC_OVRCRC 0x7d                 /* overcircle diacritic */
#define IC_LCLSTK 0x7e                 /* lower case l stroke */
#define IC_LCNACC 0x7f                 /* lower case n acute */
#define IC_LCSCAR 0x80                 /* lower case s caron */
#define IC_UCLCAR 0x81                 /* upper case l caron */
#define IC_UCNCAR 0x82                 /* upper case n caron */
#define IC_UCRCAR 0x83                 /* upper case r caron */
#define IC_UCSACC 0x84                 /* upper case s acute */
#define IC_OVRDOT 0x85                 /* overdot diacritic */
#define IC_LCZOVD 0x86                 /* lower case z overdot */
#define IC_OGONEK 0x87                 /* ogonek */
#define IC_UCZOVD 0x88                 /* upper case z overdot */
#define IC_LCZCAR 0x89                 /* lower case z caron */
#define IC_LCZACC 0x8a                 /* lower case z acute */
#define IC_UCZCAR 0x8b                 /* upper case z caron */
#define IC_UCZACC 0x8c                 /* upper case z acute */
#define IC_UCLSTK 0x8d                 /* upper case l stroke */
#define IC_UCNACC 0x8e                 /* upper case n acute */
#define IC_UCSCAR 0x8f                 /* upper case s caron */
#define IC_LCTCAR 0x90                 /* lower case t caron */
#define IC_LCRACC 0x91                 /* lower case r acute */
#define IC_LCODAC 0x92                 /* lower case o double acute */
#define IC_LCUDAC 0x93                 /* lower case u double acute */
#define IC_UCTCAR 0x94                 /* upper case t caron */
#define IC_UCRACC 0x95                 /* upper case r acute */
#define IC_UCODAC 0x96                 /* upper case o double acute */
#define IC_UCUDAC 0x97                 /* upper case u double acute */
#define IC_LCABRV 0x98                 /* lower case a breve */
#define IC_LCGBRV 0x99                 /* lower case g breve */
#define IC_UCIOVD 0x9a                 /* upper case i overdot */
#define IC_UCABRV 0x9b                 /* upper case a breve */
#define IC_UCGBRV 0x9c                 /* upper case g breve */
#define IC_BREVE  0x9d                  /* breve diacritic */
#define IC_DBLACU 0x9e                 /* double acute diacritic */
#define IC_LCSCED 0x9f                 /* lower case s cedilla */
#define IC_LITER  0xa0                  /* liter symbol */
#define IC_UCSCED 0xa2                 /* upper case s cedilla */
#define IC_MACRON 0xa3                 /* macron diacritic */
#define IC_LCTCED 0xa4                 /* lower case t cedilla */
#define IC_UCTCED 0xa5                 /* upper case t cedilla */
#define IC_LCAMAC 0xa6                 /* lower case a macron */
#define IC_UCAMAC 0xa7                 /* upper case a macron */
#define IC_LCCCRF 0xa8                 /* lower case c circumflex */
#define IC_UCCCRF 0xa9                 /* upper case c circumflex */
#define IC_LCCOVD 0xab                 /* lower case c overdot */
#define IC_UCCOVD 0xac                 /* upper case c overdot */
#define IC_LCEOVD 0xad                 /* lower case e overdot */
#define IC_UCEOVD 0xae                 /* upper case e overdot */
#define IC_LCEMAC 0xaf                 /* lower case e macron */
#define IC_UCEMAC 0xb0                 /* upper case e macron */
#define IC_LCGACC 0xb1                 /* lower case g acute */
#define IC_LCGCRF 0xb2                 /* lower case g circumflex */
#define IC_UCGCRF 0xb3                 /* upper case g circumflex */
#define IC_LCGOVD 0xb4                 /* lower case g overdot */
#define IC_UCGOVD 0xb5                 /* upper case g overdot */
#define IC_UCGCED 0xb6                 /* upper case g cedilla */
#define IC_LCHCRF 0xb7                 /* lower case h circumflex */
#define IC_UCHCRF 0xb8                 /* upper case h circumflex */
#define IC_LCHSTK 0xb9                 /* lower case h stroke */
#define IC_UCHSTK 0xba                 /* upper case h stroke */
#define IC_LCITIL 0xbb                 /* lower case i tilde */
#define IC_UCITIL 0xbc                 /* upper case i tilde */
#define IC_LCIMAC 0xbd                 /* lower case i macron */
#define IC_UCIMAC 0xbe                 /* upper case i macron */
#define IC_LCIOGO 0xbf                 /* lower case i ogonek */
#define IC_UCIOGO 0xc0                 /* upper case i ogonek */
#define IC_LCIJ   0xc1                   /* lower case ij ligature */
#define IC_UCIJ   0xc2                   /* upper case ij ligature */
#define IC_LCJCRF 0xc3                 /* lower case j circumflex */
#define IC_UCJCRF 0xc4                 /* upper case j circumflex */
#define IC_LCKCED 0xc5                 /* lower case k cedilla */
#define IC_UCKCED 0xc6                 /* upper case k cedilla */
#define IC_LCKGRE 0xc7                 /* lower case k greenlandic */
#define IC_LCLCED 0xc8                 /* lower case l cedilla */
#define IC_UCLCED 0xc9                 /* upper case l cedilla */
#define IC_LCLMDT 0xca                 /* lower case l middle dot */
#define IC_UCLMDT 0xcb                 /* upper case l middle dot */
#define IC_LCNCED 0xcc                 /* lower case n cedilla */
#define IC_UCNCED 0xcd                 /* upper case n cedilla */
#define IC_LCNLAP 0xce                 /* lower case n eng lapp */
#define IC_UCNLAP 0xcf                 /* upper case n eng lapp */
#define IC_LCOMAC 0xd0                 /* lower case o macron */
#define IC_UCOMAC 0xd1                 /* upper case o macron */
#define IC_LCOE   0xd2                   /* lower case oe ligature */
#define IC_UCOE   0xd3                   /* upper case oe ligature */
#define IC_LCRCED 0xd4                 /* lower case r cedilla */
#define IC_UCRCED 0xd5                 /* upper case r cedilla */
#define IC_LCSCRF 0xd6                 /* lower case s circumflex */
#define IC_UCSCRF 0xd7                 /* upper case s circumflex */
#define IC_LCTSTK 0xd8                 /* lower case t stroke */
#define IC_UCTSTK 0xd9                 /* upper case t stroke */
#define IC_LCUTIL 0xda                 /* lower case u tilde */
#define IC_UCUTIL 0xdb                 /* upper case u tilde */
#define IC_LCUBRV 0xdc                 /* lower case u breve */
#define IC_UCUBRV 0xdd                 /* upper case u breve */
#define IC_LCUMAC 0xde                 /* lower case u macron */
#define IC_UCUMAC 0xdf                 /* upper case u macron */
#define IC_LCUOGO 0xe0                 /* lower case u ogonek */
#define IC_UCUOGO 0xe1                 /* upper case u ogonek */
#define IC_LCWCRF 0xe2                 /* lower case w circumflex */
#define IC_UCWCRF 0xe3                 /* upper case w circumflex */
#define IC_LCYCRF 0xe4                 /* lower case y circumflex */
#define IC_UCYCRF 0xe5                 /* upper case y circumflex */
#define IC_UCYUML 0xe6                 /* upper case y umlaut */
#define IC_MULTPL_OLD 0xee             /* multiply symbol */
/********************below are P2 offsets************************/
#define IC_LOGOR  0x24                  /* logical or symbol */
#define IC_PESETA 0x75                 /* Peseta sign */
#define IC_REVNOT 0x76                 /* reversed logical not */
#define IC_LBRITE 0x88                 /* left half bright char cell */
#define IC_LCTAU  0x90                  /* lower case tau   */
#define IC_UCPHI  0x91                  /* upper case phi   */
#define IC_ELEMNT 0x97                 /* element          */
#define IC_IDENTY 0x99                 /* identity symbol  */
#define IC_GTEQ   0x9a                   /* greater than or equal */
#define IC_LTEQ   0x9b                   /* less than or equal */
#define IC_UINTGL 0x9c                 /* upper half integral sign */
#define IC_LINTGL 0x9d                 /* lower half integral sign */
#define IC_SQROOT 0xa0                 /* square root */
#define IC_SSN    0xa2                    /* superscript n */

#define IC_NUL 0x00
#define IC_SOH 0x01
#define IC_STX 0x02
#define IC_ETX 0x03
#define IC_EOT 0x04
#define IC_ENQ 0x05
#define IC_ACK 0x06
#define IC_BEL 0x07
#define IC_BS  0x08
#define IC_HT  0x09
#define IC_LF  0x0a
#define IC_VT  0x0b
#define IC_FF  0x0c
#define IC_CR  0x0d
#define IC_SO  0x0e
#define IC_SI  0x0f
#define IC_DLE 0x10
#define IC_DC1 0x11
#define IC_DC2 0x12
#define IC_DC3 0x13
#define IC_DC4 0x14
#define IC_NAK 0x15
#define IC_SYN 0x16
#define IC_ETB 0x17
#define IC_CAN 0x18
#define IC_EM  0x19
#define IC_SUB 0x1a
#define IC_ESC 0x1b
#define IC_FS  0x1c
#define IC_GS  0x1d
#define IC_RS  0x1e
#define IC_US  0x1f
#define IC_SS4 0x1c
#define IC_SS3 0x1d
#define IC_SS2 0x1e
#define IC_SS1 0x1f
                                 /* Sequence identifiers */
#define IC_CBT 0x00
#define IC_CHA 0x01
#define IC_CHT 0x02
#define IC_CTC 0x03
#define IC_CNL 0x04
#define IC_CPL 0x05
#define IC_CPR 0x06
#define IC_CUB 0x07
#define IC_CUD 0x08
#define IC_CUF 0x09
#define IC_CUP 0x0a
#define IC_CUU 0x0b
#define IC_CVT 0x0c
#define IC_DCH 0x0d
#define IC_DL  0x0e
#define IC_DSR 0x0f
#define IC_DMI 0x10
#define IC_EMI 0x11
#define IC_EA  0x12
#define IC_ED  0x13
#define IC_EF  0x14
#define IC_EL  0x15
#define IC_ECH 0x16
#define IC_GSM 0x17
#define IC_HTS 0x18
#define IC_HVP 0x19
#define IC_ICH 0x1a
#define IC_IL  0x1b
#define IC_IND 0x1c
#define IC_NEL 0x1d
#define IC_PFK 0x1e
#define IC_PLD 0x1f
#define IC_PLU 0x20
#define IC_RI  0x21
#define IC_RIS 0x23
#define IC_RM  0x24
#define IC_SD  0x25
#define IC_SL  0x26
#define IC_SR  0x27
#define IC_SU  0x28
#define IC_SGR 0x29
#define IC_SG0 0x2a
#define IC_SG1 0x2b
#define IC_SM  0x2c
#define IC_TBC 0x2d
#define IC_VTS 0x2e
#define IC_SCP 0x2f
#define IC_RCP 0x30
#define IC_KSI 0x31
#define IC_VTD 0x32
#define IC_VTK 0x33
#define IC_IGNORE 0xfe
#define IC_UNDEF  0x7f
/*Key generated control sequences identifying sequence with specific parameter(s)*/
#define KF_CUU 0x0101
#define KF_CUD 0x0102
#define KF_CUF 0x0103
#define KF_CUB 0x0104
#define KF_CBT 0x0105
#define KF_CHT 0x0106
#define KF_CVT  0x0107
#define KF_HOM  0x0108
#define KF_LL   0x0109
#define KF_END  0x010a
#define KF_CPL  0x010b
#define KF_CNL  0x010c
#define KF_FADV 0x010d
#define KF_FRET 0x010e
#define KF_ECBS 0x010f                 /* error correct backspace */
#define KF_SEL  0x011e
#define KF_PCSEL 0x011f
#define KF_PCU  0x0121
#define KF_PCD  0x0122
#define KF_PCF  0x0123
#define KF_PCB  0x0124
#define KF_PCSU 0x0131
#define KF_PCSD 0x0132
#define KF_PCSL 0x0133
#define KF_PCSR 0x0134
#define KF_SELU 0x0141
#define KF_SELD 0x0142
#define KF_SELL 0x0143
#define KF_SELR 0x0144
#define KF_INS  0x0150
#define KF_DCH  0x0151
#define KF_IL   0x0152
#define KF_DL   0x0153
#define KF_EEOL 0x0154
#define KF_EEOF 0x0155
#define KF_CLR  0x0156
#define KF_INIT 0x0157
#define KF_NL   0x0160
#define KF_RNL  0x0161
#define KF_RI   0x0162
#define KF_IND  0x0163
#define KF_PLU  0x0164
#define KF_PLD  0x0165
#define KF_KAT  0x01a1                  /* katakana           */
#define KF_ALN  0x01a2                  /* alpha/num          */
#define KF_HIR  0x01a3                  /* hiragana           */
#define KF_RKC  0x01a4                  /* rkc function       */
#define KF_CNV  0x01a5                  /* convert            */
#define KF_NOC  0x01a6                  /* no convert         */
#define KF_ACD  0x01a7                  /* all candidates     */
#define KF_REGIS 0x01a8                /* registration       */
#define KF_KNO   0x01a9                  /* kanji no.          */
#define KF_CMDSW 0x01aa                /* conv. mode switch  */
#define KF_DGNSS 0x01ab                /* diagnosis          */
#define KF_PRVCD 0x01ac                /* previous candidate */
#define KF_RSV1  0x01ad
#define KF_RSV2  0x01ae
#define KF_RSV3  0x01af
#define KF_RSV4  0x01b0
#define KF_RSV5  0x01b1
#define KF_RSV6  0x01b2
#define KF_RSV7  0x01b3
#define KF_RSV8  0x01b4                 /*  THESE F.I.D.s RESERVED  */

#define KF_PF1  0x0000
#define KF_PF2  0x0001
#define KF_PF3  0x0002
#define KF_PF4  0x0003
#define KF_PF5  0x0004
#define KF_PF6  0x0005
#define KF_PF7  0x0006
#define KF_PF8  0x0007
#define KF_PF9  0x0008
#define KF_PF10 0x0009
#define KF_PF11 0x000a
#define KF_PF12 0x000b
#define KF_PF13 0x000c
#define KF_PF14 0x000d
#define KF_PF15 0x000e
#define KF_PF16 0x000f
#define KF_PF17 0x0010
#define KF_PF18 0x0011
#define KF_PF19 0x0012
#define KF_PF20 0x0013
#define KF_PF21 0x0014
#define KF_PF22 0x0015
#define KF_PF23 0x0016
#define KF_PF24 0x0017
#define KF_PF25 0x0018
#define KF_PF26 0x0019
#define KF_PF27 0x001a
#define KF_PF28 0x001b
#define KF_PF29 0x001c
#define KF_PF30 0x001d
#define KF_PF31 0x001e
#define KF_PF32 0x001f
#define KF_PF33 0x0020
#define KF_PF34 0x0021
#define KF_PF35 0x0022
#define KF_PF36 0x0023
#define KF_PF37 0x0024
#define KF_PF38 0x0025
#define KF_PF39 0x0026
#define KF_PF40 0x0027
#define KF_PF41 0x0028
#define KF_PF42 0x0029
#define KF_PF43 0x002a
#define KF_PF44 0x002b
#define KF_PF45 0x002c
#define KF_PF46 0x002d
#define KF_PF47 0x002e
#define KF_PF48 0x002f
#define KF_PF49 0x0030
#define KF_PF50 0x0031
#define KF_PF51 0x0032
#define KF_PF52 0x0033
#define KF_PF53 0x0034
#define KF_PF54 0x0035
#define KF_PF55 0x0036
#define KF_PF56 0x0037
#define KF_PF57 0x0038
#define KF_PF58 0x0039
#define KF_PF59 0x003a
#define KF_PF60 0x003b
#define KF_PF61 0x003c
#define KF_PF62 0x003d
#define KF_PF63 0x003e
#define KF_PF64 0x003f
#define KF_PF65 0x0040
#define KF_PF66 0x0041
#define KF_PF67 0x0042
#define KF_PF68 0x0043
#define KF_PF69 0x0044
#define KF_PF70 0x0045
#define KF_PF71 0x0046
#define KF_PF72 0x0047
#define KF_PF73 0x0048
#define KF_PF74 0x0049
#define KF_PF75 0x004a
#define KF_PF76 0x004b
#define KF_PF77 0x004c
#define KF_PF78 0x004d
#define KF_PF79 0x004e
#define KF_PF80 0x004f
#define KF_PF81 0x0050
#define KF_PF82 0x0051
#define KF_PF83 0x0052
#define KF_PF84 0x0053
#define KF_PF85 0x0054
#define KF_PF86 0x0055
#define KF_PF87 0x0056
#define KF_PF88 0x0057
#define KF_PF89 0x0058
#define KF_PF90 0x0059
#define KF_PF91 0x005a
#define KF_PF92 0x005b
#define KF_PF93 0x005c
#define KF_PF94 0x005d
#define KF_PF95 0x005e
#define KF_PF96 0x005f
#define KF_PF97 0x0060
#define KF_PF98 0x0061
#define KF_PF99 0x0062
#define KF_PF100 0x0063
#define KF_PF101 0x0064
#define KF_PF102 0x0065
#define KF_PF103 0x0066
#define KF_PF104 0x0067
#define KF_PF105 0x0068
#define KF_PF106 0x0069
#define KF_PF107 0x006a
#define KF_PF108 0x006b
#define KF_PF109 0x006c
#define KF_PF110 0x006d
#define KF_PF111 0x006e
#define KF_PF112 0x006f
#define KF_PF113 0x0070
#define KF_PF114 0x0071
#define KF_PF115 0x0072
#define KF_PF116 0x0073
#define KF_PF117 0x0074
#define KF_PF118 0x0075
#define KF_PF119 0x0076
#define KF_PF120 0x0077
#define KF_PF121 0x0078
#define KF_PF122 0x0079
#define KF_PF123 0x007a
#define KF_PF124 0x007b
#define KF_PF125 0x007c
#define KF_PF126 0x007d
#define KF_PF127 0x007e
#define KF_PF128 0x007f
#define KF_PF129 0x0080
#define KF_PF130 0x0081
#define KF_PF131 0x0082
#define KF_PF132 0x0083
#define KF_PF133 0x0084
#define KF_PF134 0x0085
#define KF_PF135 0x0086
#define KF_PF136 0x0087
#define KF_PF137 0x0088
#define KF_PF138 0x0089
#define KF_PF139 0x008a
#define KF_PF140 0x008b
#define KF_PF141 0x008c
#define KF_PF142 0x008d
#define KF_PF143 0x008e
#define KF_PF144 0x008f
#define KF_PF145 0x0090
#define KF_PF146 0x0091
#define KF_PF147 0x0092
#define KF_PF148 0x0093
#define KF_PF149 0x0094
#define KF_PF150 0x0095
#define KF_PF151 0x0096
#define KF_PF152 0x0097
#define KF_PF153 0x0098
#define KF_PF154 0x0099
#define KF_PF155 0x009a
#define KF_PF156 0x009b
#define KF_PF157 0x009c
#define KF_PF158 0x009d
#define KF_PF159 0x009e
#define KF_PF160 0x009f
#define KF_PF161 0x00a0
#define KF_PF162 0x00a1
#define KF_PF163 0x00a2
#define KF_PF164 0x00a3
#define KF_PF165 0x00a4
#define KF_PF166 0x00a5
#define KF_PF167 0x00a6
#define KF_PF168 0x00a7
#define KF_PF169 0x00a8
#define KF_PF170 0x00a9
#define KF_PF171 0x00aa
#define KF_PF172 0x00ab
#define KF_PF173 0x00ac
#define KF_PF174 0x00ad
#define KF_PF175 0x00ae
#define KF_PF176 0x00af
#define KF_PF177 0x00b0
#define KF_PF178 0x00b1
#define KF_PF179 0x00b2
#define KF_PF180 0x00b3
#define KF_PF181 0x00b4
#define KF_PF182 0x00b5
#define KF_PF183 0x00b6
#define KF_PF184 0x00b7
#define KF_PF185 0x00b8
#define KF_PF186 0x00b9
#define KF_PF187 0x00ba
#define KF_PF188 0x00bb
#define KF_PF189 0x00bc
#define KF_PF190 0x00bd
#define KF_PF191 0x00be
#define KF_PF192 0x00bf
#define KF_PF193 0x00c0
#define KF_PF194 0x00c1
#define KF_PF195 0x00c2
#define KF_PF196 0x00c3
#define KF_PF197 0x00c4
#define KF_PF198 0x00c5
#define KF_PF199 0x00c6
#define KF_PF200 0x00c7
#define KF_PF201 0x00c8
#define KF_PF202 0x00c9
#define KF_PF203 0x00ca
#define KF_PF204 0x00cb
#define KF_PF205 0x00cc
#define KF_PF206 0x00cd
#define KF_PF207 0x00ce
#define KF_PF208 0x00cf
#define KF_PF209 0x00d0
#define KF_PF210 0x00d1
#define KF_PF211 0x00d2
#define KF_PF212 0x00d3
#define KF_PF213 0x00d4
#define KF_PF214 0x00d5
#define KF_PF215 0x00d6
#define KF_PF216 0x00d7
#define KF_PF217 0x00d8
#define KF_PF218 0x00d9
#define KF_PF219 0x00da
#define KF_PF220 0x00db
#define KF_PF221 0x00dc
#define KF_PF222 0x00dd
#define KF_PF223 0x00de
#define KF_PF224 0x00df
#define KF_PF225 0x00e0
#define KF_PF226 0x00e1
#define KF_PF227 0x00e2
#define KF_PF228 0x00e3
#define KF_PF229 0x00e4
#define KF_PF230 0x00e5
#define KF_PF231 0x00e6
#define KF_PF232 0x00e7
#define KF_PF233 0x00e8
#define KF_PF234 0x00e9
#define KF_PF235 0x00ea
#define KF_PF236 0x00eb
#define KF_PF237 0x00ec
#define KF_PF238 0x00ed
#define KF_PF239 0x00ee
#define KF_PF240 0x00ef
#define KF_PF241 0x00f0
#define KF_PF242 0x00f1
#define KF_PF243 0x00f2
#define KF_PF244 0x00f3
#define KF_PF245 0x00f4
#define KF_PF246 0x00f5
#define KF_PF247 0x00f6
#define KF_PF248 0x00f7
#define KF_PF249 0x00f8
#define KF_PF250 0x00f9
#define KF_PF251 0x00fa
#define KF_PF252 0x00fb
#define KF_PF253 0x00fc
#define KF_PF254 0x00fd
#define KF_PF255 0x00fe
#define KF_PF256 0x00ff


            /* need to be changed to PFK equivalents */
#define KF_IGNORE 0x01ff
#define KF_DO     0x0200
#define KF_IQUIT  0x0201
#define KF_CMD    0x0202
#define KF_CMDRT  0x0203
#define KF_HELP   0x0204
#define KF_KHLP   0x0205
#define KF_PRTS   0x0208
#define KF_PCPGUP 0x0209
#define KF_PCPGDN 0x020a
#define KF_PCBRK  0x020b
#define KF_PCSLK  0x020c
#define KF_PCPSE  0x020d
#define KF_HDQUT  0x0213
#define KF_HDINT  0x0214
#define KF_HDEOF  0x0215
#define KF_HDEOL  0x0216
#define KF_NEXTW  0x0217
#define KF_PREVW  0x0218

#define FE_IND 0x44
#define FE_NEL 0x45
#define FE_HTS 0x48
#define FE_VTS 0x49
#define FE_PLD 0x4a
#define FE_PLU 0x4b
#define FE_RI  0x4c
#define FE_DCS 0x50
#define FE_CSI 0x5b
#define FE_ST  0x5c
#define FE_OSC 0x5d
#define FE_PM  0x5e
#define FE_APC 0x5f
#define FS_DMI 0x60
#define FS_EMI 0x62
#define FS_RIS 0x63

               /* CONTROL SEQUENCE FINAL CODES */
#define CSEQ_F_CBT 0x5a
#define CSEQ_F_CHA 0x47
#define CSEQ_F_CHT 0x49
#define CSEQ_F_CTC 0x57
#define CSEQ_F_CNL 0x45
#define CSEQ_F_CPL 0x46
#define CSEQ_F_CPR 0x52
#define CSEQ_F_CUB 0x44
#define CSEQ_F_CUD 0x42
#define CSEQ_F_CUF 0x43
#define CSEQ_F_CUP 0x48
#define CSEQ_F_CUU 0x41
#define CSEQ_F_CVT 0x59
#define CSEQ_F_DCH 0x50
#define CSEQ_F_DL  0x4d
#define CSEQ_F_DSR 0x6e
#define CSEQ_F_EA  0x4f
#define CSEQ_F_ED  0x4a
#define CSEQ_F_EF  0x4e
#define CSEQ_F_EL  0x4b
#define CSEQ_F_ECH 0x58
#define CSEQ_F_HVP 0x66
#define CSEQ_F_ICH 0x40
#define CSEQ_F_IL  0x4c
#define CSEQ_F_KSI 0x70
#define CSEQ_F_PFK 0x71
#define CSEQ_F_RCP 0x75
#define CSEQ_F_RM  0x6c
#define CSEQ_F_SCP 0x73
#define CSEQ_F_SD  0x54
#define CSEQ_F_SU  0x53
#define CSEQ_F_SGR 0x6d
#define CSEQ_F_SM  0x68
#define CSEQ_F_TBC 0x67
#define CSEQ_F_VTD 0x78
#define CSEQ_F_VTK 0x7a
#define CSEQ_F_GSM 0x42    /*CONTROL SEQUENCE FINAL CHARS WHEN INTERMEDIATE=SP*/
#define CSEQ_F_SL  0x40
#define CSEQ_F_SR  0x41
                                 /* esc sequence intermediate  */
#define I_SG0A 0x28
#define I_SG0B 0x2c
#define I_SG1A 0x29
#define I_SG1B 0x2d

#define DMI	'`'		/* disable manual input `*/
#define EMI	'b'		/* enable manual input */

#endif
