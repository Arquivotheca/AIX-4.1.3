/* @(#)82	1.2 6/4/91 12:51:20 */

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
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
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kchin.t
 *
 * DESCRIPTIVE NAME:  HINSI CONVERSIONTY TABLE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/************************************************************************/
/************************************************************************/
/* HINsi conversionty table                                             */
/*                                                                      */
/* dtype -> strhin,  jhinshi -> strhin                                  */
/* dtype -> endhin,  fhinshi -> endhin                                  */
/*                                                                      */
/*    The JPN is a table of jiritsu-go own's penalty.                   */
/* This table was updated because TRL debuged or Improved               */
/************************************************************************/

/* hinshi -> hinl conv. */
static uschar hn_hinl[23] =
                      { UNDEF   ,    /*  0  null */
                        S_YOUGEN,    /*  1  ka 5 dan */
                        S_YOUGEN,    /*  2  ka 5 dan -- iku */
                        S_YOUGEN,    /*  3  ga 5 dan */
                        S_YOUGEN,    /*  4  sa 5 dan */
                        S_YOUGEN,    /*  5  ta 5 dan */
                        S_YOUGEN,    /*  6  na 5 dan */
                        S_YOUGEN,    /*  7  ba 5 dan */
                        S_YOUGEN,    /*  8  ma 5 dan */
                        S_YOUGEN,    /*  9  ra 5 dan */
                        S_YOUGEN,    /* 10  wa 5 dan */
                        S_MEIYOU,    /* 11  1 dan -- taigen-ka */
                        S_YOUGEN,    /* 12  1 dan -- hi-taigen-ka */
                        S_MEIYOU,    /* 13  sahen -- taigen */
                        S_YOUGEN,    /* 14  sahen */
                        S_YOUGEN,    /* 15  zahen */
                        S_JOSUSHI,   /* 16  josushi */
                        S_YOUGEN,    /* 17  keiyoushi */
                        S_YOUGEN,    /* 18  keiyoudoushi */
                        S_MEISHI,    /* 19  meishi */
                        S_FUKUSHI,   /* 20  rentaishi */
                        S_FUKUSHI,   /* 21  fukushi */
                        S_OTHER,  }; /* 22  setsuzokushi, kandoushi */

/* dtype -> hinl conv. */
static uschar tp_hinl[15] =
                      { UNDEF,       /*  0  null */
                        50,          /*  1  ippan */
                        S_SETTO,     /*  2  setto */
                        S_SETTO,     /*  3  o */
                        S_SETTO,     /*  4  go */
                        S_SETSUBI,   /*  5  setsubi */
                        S_SUUCHI,    /*  6  suuchi */
                        S_SETTO,     /*  7  num. setto */
                        S_JOSUSHI,   /*  8  josushi */
                        S_KOYUU,     /*  9  koyuu-meishi */
                        S_SETTO,     /* 10  koyuu-setto */
                        S_PSETSUBI,  /* 11  koyuu-setsubi */
                        S_TOUTEN,    /* 12  touten */
                        S_KUTEN,     /* 13  kuten */
                        S_CONVKEY,}; /* 14  conv. key */

/* hinsi -> hinr conv. */
static uschar hn_hinr[128]=
                      { UNDEF,       /*  0  null */
                        UNDEF,       /*  1  ka 5 dan */
                        UNDEF,       /*  2  ka 5 dan -- iku */
                        UNDEF,       /*  3  ga 5 dan */
                        UNDEF,       /*  4  sa 5 dan */
                        UNDEF,       /*  5  ta 5 dan */
                        UNDEF,       /*  6  na 5 dan */
                        UNDEF,       /*  7  ba 5 dan */
                        UNDEF,       /*  8  ma 5 dan */
                        UNDEF,       /*  9  ra 5 dan */
                        UNDEF,       /* 10  wa 5 dan */
                        E_MEIREN,    /* 11  1 dan -- taigen-ka */
                        E_RENYOU,    /* 12  1 dan -- hi-taigen-ka */
                        E_MEISHI,    /* 13  sahen -- taigen */
                        UNDEF,       /* 14  sahen */
                        UNDEF,       /* 15  zahen */
                        E_MEISHI,    /* 16  josushi */
                        UNDEF,       /* 17  keiyoushi */
                        E_MEISHI,    /* 18  keiyoudoushi */
                        E_MEISHI,    /* 19  meishi */
                        E_RENTAI,    /* 20  rentaishi */
                        E_RENYOU,    /* 21  fukushi */
    /* 87-07-03 ya */   E_FNEU,      /* 22  setsuzokushi, kandoushi */
                        UNDEF,       /* 23  setto */
                        UNDEF,       /* 24  setsubi */
                        UNDEF,       /* 25  top of bunsetsu */
                        UNDEF,       /* 26  5 dan mizen --nai */
                        UNDEF,       /* 27  mizen --u */
                        E_RENYOU,    /* 28  renyou */
                        UNDEF,       /* 29  renyou - onbin */
                        UNDEF,       /* 30  renyou - onbin */
                        E_SHUREN,    /* 31  shuushi, rentai */
                        E_SHUUSHI,   /* 32  5 dan katei, meirei */
                        UNDEF,       /* 33  katei, not 5 dan */
                        E_SHUUSHI,   /* 34  meirei, not 5 dan */
                        UNDEF,       /* 35  sahen mizen 'sa' */
                        UNDEF,       /* 36  sahen mizen 'se' */
                        E_RENYOU,    /* 37  sahen mizen, renyou 'shi' */
                        UNDEF,       /* 38  sahen --beshi 'su' */
                        UNDEF,       /* 39  kahen mizen 'ko' */
                        E_RENYOU,    /* 40  kahen renyou 'ki' */
                        UNDEF,       /* 41  keiyou renyou 'kaq' */
                        E_RENYOU,    /* 42  keiyou renyou 'ku' */
                        E_SHUREN,    /* 43  keiyou shuushi, rentai 'i' */
                        UNDEF,       /* 44  keiyou katei 'kere' */
                        UNDEF,       /* 45  keidou renyou 'daq' */
                        E_RENYOU,    /* 46  keidou renyou 'de' */
                        E_RENYOU,    /* 47  keidou renyou 'ni' */
                        E_SHUUSHI,   /* 48  keidou shuushi 'da' */
                        E_RENTAI,    /* 49  keidou rentai 'na' */
                        E_RENYOU,    /* 50  keidou katei 'nara' */
                        UNDEF,       /* 51  'na', 'ta' of nai, tai */
                        E_SHUREN,    /* 52  rashii -- suitei */
                        E_SHUUSHI,   /* 53  sou-da -- youtai */
                        UNDEF,       /* 54  sou-da -- denbun */
                        E_RENYOU,    /* 55  you-da */
     /* 87-07-03 de */  E_FNEU,      /* 56  'da' -- dantei */
                        UNDEF,       /* 57  'na' -- dantei */
                        E_SHUUSHI,   /* 58  'masu' mizen 'mase' */
                        UNDEF,       /* 59  'masu' renyou 'mashi' */
                        E_SHUREN,    /* 60  'masu' shuushi, rentai */
                        UNDEF,       /* 61  'desu' renyou 'deshi' */
                        E_SHUUSHI,   /* 62  'desu' shuushi, rentai */
     /* 87-07-20 bug */ E_SHUREN,    /* 63  'ta', 'da' -- kako */
     /* 87-07-29 bug */ E_SHUREN,    /* 64  'zu' -- uchikeshi */
     /* 87-07-31 bug */ E_SHUREN,    /* 65  'nu' -- uchikeshi */
     /* 88-07-28 Noz */ E_SHUREN,    /* 66  'you' -- ishi */
                        E_RENYOU,    /* 67  'beku' */
                        E_RENTAI,    /* 68  'beki' */
                        E_FYOU,      /* 69  'te', 'de' */
                        E_FYOU,      /* 70  'te' */
                        E_FYOU,      /* 71  'ri'  of tari, dari */
                        E_FYOU,      /* 72  'nagara', 'tsutsu' */
                        E_FNEU,      /* 73  'ba' */
                        E_FNEU,      /* 74  'shi' */
                        E_FNEU,      /* 75  'ga' */
                        E_FTAI,      /* 76  'no' */
                        E_FYOU,      /* 77  'wo' */
                        E_FYOU,      /* 78  'ni' */
                        E_FNEU,      /* 79  'ni' -- zentei */
                        E_FYOU,      /* 80  'he' */
                        E_FTAI,      /* 81  'to' */
                        E_FNEU,      /* 82  'to' -- inyou */
                        E_FNEU,      /* 83  'yori' */
                        E_FNEU,      /* 84  'kara' */
                        E_FYOU,      /* 85  'sae', 'sura' */
                        E_FNEU,      /* 86  'mo', 'koso' */
                        E_FNEU,      /* 87  fukujoshi */
                        E_FNEU,      /* 88  heiretsujoshi */
                        E_FNEU,      /* 89  'yara', 'dano' */
                        E_SHUUSHI,   /* 90  'na' -- kinshi */
                        E_FNEU,      /* 91  'kara' -- riyuu */
                        UNDEF,       /* 92  'noni' */
                        UNDEF,       /* 93  'gozai' */
                        E_MEISHI,    /* 94  daimeishi */
                        E_SHUUSHI,   /* 95  'nasai' */
                        UNDEF,       /* 96  top of fuzokugo */
                        UNDEF,       /* 97  reserved */
                        UNDEF,       /* 98  reserved */
                        UNDEF,       /* 99  reserved */
                        UNDEF,       /*100  reserved */
                        E_RENYOU,    /*101  'suru' renyou 'shi' */
                        E_RENYOU,    /*102  keishiki-meishi, fukushi-youhou */
                        E_RENYOU,    /*103  'you', abbr. of 'youni' */
                        E_FYOU,      /*104  'ni' of --niiku */
                        E_FYOU,      /*105  'ni' of --karani */
                        E_FNEU,      /*106             */
                        E_FNEU,      /*107             */
                        E_FNEU,      /*108             */
                        E_FNEU,      /*109             */
                        E_FNEU,      /*110             */
                        E_FNEU,      /*111             */
                        E_FNEU,      /*112             */
                        E_FNEU,      /*113             */
                        E_FNEU,      /*114             */
                        E_FNEU,      /*115             */
                        E_FNEU,      /*116             */
                        E_FNEU,      /*117             */
                        E_FNEU,      /*118             */
                        E_FNEU,      /*119             */
                        E_FNEU,      /*120             */
                        E_FNEU,      /*121             */
                        E_FNEU,      /*122             */
                        E_FNEU,      /*123             */
                        E_FNEU,      /*124             */
                        E_FNEU,      /*125             */
                        E_FNEU,      /*126             */
                        E_FNEU,  };  /*127             */

/* dtype -> hinr conv. */
static uschar tp_hinr[15] =
                      { UNDEF,       /* null */
                        50,          /* ippan */
                        E_SETTO,     /* setto */
                        E_OSETTO,    /* o */
                        E_GSETTO,    /* go */
                        E_SETSUBI,   /* setsubi */
                        E_SUUCHI,    /* suuchi */
                        E_NSETTO,    /* num. setto */
                        E_SETSUBI,   /* josushi */
                        E_KOYUU,     /* koyuu-meishi */
                        E_PSETTO,    /* koyuu-setto */
                        E_SETSUBI,   /* koyuu-setsubi */
                        E_KUGIRI,    /* touten */
                        E_KUGIRI,    /* kuten */
                        E_CONVKEY,}; /* conv. key */
