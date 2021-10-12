/* @(#)40	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/csd.h, cfgnls, bos411, 9428A410j 8/30/93 15:01:55 */
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define False 0
#define True  1
/************************************************************************/
/************************************************************************/
/* A state indicates how the previous character is to change the following 
   character, i.e. Initial means that the following character must
   be in initial shape because the preceding character does not connect to 
   the left. There are two basic states Initial and Middle. All the
   other states affect the normal characters as Middle. They have special 
   meaning only in certain cases. 
   There are 6 state :*/

#define InitialState        'I'  /* next char is initial */
#define MiddleState         'M'  /* next char is middle */
#define LamIsoState         'L'  /* next char is middle, unless it is an alef, 
                                    then it becomes an isolated lam-alef */
#define InitialLamIsoState  'O'  /* next char is initial, unless it is an alef,
                                    then it becomes an isolated lam-alef */
#define LamConnState        'C'  /* next char is middle, unless it is an alef, 
                                    then it becomes a connected lam-alef */
#define InitialLamConnState 'N'  /* next char is initial, unless it is an alef,
                                    then it becomes a connected lam-alef */
#define SeenState           'S'  /* next char is middle, unless it is a 
                                    SpaceGroup char, it becomes a Tail */
#define InitialSeenState    'E'  /* next char is initial, unless it is a 
                                    SpaceGroup char, it becomes a Tail */
#define YehHamzaState       'Y'  /* next char is middle, unless it is a 
                                    SpaceGroup char, it becomes a Hamza */

/************************************************************************/
/* All the characters are devided into groups. The group of the character
   indicates whether the character is right connected only, or connects
   on both sides or doesn't connect at all. This determines the state at
   which the character that follows it will be in. The groups also indicates
   the characters that require any sort of special handling.
   There are 8 groups that are characterized as follows :
   0 : Symbols --> These characters are neither left nor right connectors.
                   These are all the Latin characters, box characters and the 
                   punctuation. 
   1 : Isolated Vowels --> In Arabic, this group includes the dammatan and
	           the kasratan. These are vowels that need special handling,
                   but they neither connect to the left nor to the right.
   2 : Spaces  --> These are neither left nor right connectors, but they may 
                   be overridden if something needs to be expanded.
                   In Arabic : These are the space, RSP and tail. 
   3 : ALefs   --> Alefs are right connectors only and need special handling 
                   following a lam. This group contains all the shapes of all 
                   the alefs.
   4 : R_connectors --> These are all the characters that are right connectors 
                   only.
                   In Arabic: It contains characters like the reh, dal, waw, 
                   all the lamalefs, and the alef maksoura.
   5 : Normal  --> This group contains all the normal characters that connect
                   both to the left and to the right. They need no special 
                   handling.
   6 : Seen    --> In Arabic : This group contains all the shapes of the seen, 
                   sheen, sad and dad. They connect both to the left and to 
                   the right, but they need specail handling if a Space 
                   character follows.
   7 : YehHamza -> This group contains all the shapes of the yeh hamza. They
                   connect both to the left and to the right, but they need 
                   specail handling if a Space character follows in 
                   in HOST shaping mode.
  8 : Lam      --> This group contains the lam characters. They connect to the
                   left and to the right, but they affect a following alef
                   in a special way.
  9 : Vowel    --> This group contains all the connectable vowels. 
                   Vowels do not influence connectivity, but they need 
                   special consideration. 
  */
#define Symbols_Group    0
#define Iso_Vowels_Group 1
#define Spaces_Group     2
#define Alef_Group       3
#define R_Conn_Group     4
#define Normal_Group     5
#define Seen_Group       6
#define Yeh_Hamza_Group  7
#define Lam_Group        8
#define Vowels_Group     9

#define ISO  0
#define INIT 1
#define MID  2
#define FIN  3
/************************************************************************/
typedef unsigned char PCHAR [5];
/************************************************************************/
/* Codepage : IBM 1046  */
/************************************************************************/
/* This array contains the four different shapes of every character,
   namely isolated, initial, middle and final, and the group of the
   character. */
static PCHAR CHRGRP [128] = {

/*  ISO  INIT MID  FIN  GROUP   */
   {0xC5,0xC5,0x80,0x80,3},       /* char 128 --> hamza under alef */
   {0x81,0x81,0x81,0x81,0},       /* char 129 --> times */
   {0x82,0x82,0x82,0x82,0},       /* char 130 --> divided */
   {0x83,0xAF,0xAF,0x83,6},       /* char 131 --> seen */
   {0x84,0xBA,0xBA,0x84,6},       /* char 132 --> sheen */
   {0x85,0xBC,0xBC,0x85,6},       /* char 133 --> sad */
   {0x86,0xBD,0xBD,0x86,6},       /* char 134 --> dad */
   {0xEB,0xEB,0x87,0xEB,9},       /* char 135 --> fathatan */
   {0x88,0x88,0x88,0x88,0},       /* char 136 --> blank */
   {0x89,0x89,0x89,0x89,0},       /* char 137 --> box character */
   {0x8A,0x8A,0x8A,0x8A,0},       /* char 138 --> box character */
   {0x8B,0x8B,0x8B,0x8B,0},       /* char 139 --> box character */
   {0x8C,0x8C,0x8C,0x8C,0},       /* char 140 --> box character */
   {0x8D,0x8D,0x8D,0x8D,0},       /* char 141 --> box character */
   {0x8E,0x8E,0x8E,0x8E,0},       /* char 142 --> box character */
   {0x8F,0x8F,0x8F,0x8F,0},       /* char 143 --> box character */
   {0xEF,0xEF,0x90,0xEF,9},       /* char 144 --> damma */
   {0xF0,0xF0,0x91,0xF0,9},       /* char 145 --> kasra */
   {0xF1,0xF1,0x92,0xF1,9},       /* char 146 --> shadda */
   {0xF2,0xF2,0x93,0xF2,9},       /* char 147 --> soukoun */
   {0xEE,0xEE,0x94,0xEE,9},       /* char 148 --> fatha */
   {0xC6,0xA6,0xA6,0x95,7},       /* char 149 --> yeh hamza */
   {0xE9,0xE9,0x96,0x96,4},       /* char 150 --> yeh */
   {0xEA,0x97,0x97,0x98,5},       /* char 151 --> yeh nokteten */
   {0xEA,0x97,0x97,0x98,5},       /* char 152 --> yeh nokteten */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 153 --> gheen */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 154 --> gheen */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 155 --> gheen */
   {0xF7,0xF7,0x9C,0x9C,4},       /* char 156 --> lam alef madda */
   {0xF8,0xF8,0x9D,0x9D,4},       /* char 157 --> lam alef hamza */
   {0xF9,0xF9,0x9E,0x9E,4},       /* char 158 --> lam alef hamza under */
   {0xFA,0xFA,0x9F,0x9F,4},       /* char 159 --> lam alef */
   {0xA0,0xA0,0xA0,0xA0,2},       /* char 160 --> RSP */
   {0xA1,0xA1,0xA1,0xA1,3},       /* char 161 --> special alef madda */
   {0xA2,0xA2,0xA2,0xA2,3},       /* char 162 --> special alef hamza */
   {0xA3,0xA3,0xA3,0xA3,3},       /* char 163 --> special alef hamza under */
   {0xA4,0xA4,0xA4,0xA4,0},       /* char 164 --> sign */
   {0xA5,0xA5,0xA5,0xA5,3},       /* char 165 --> special alef (of lamalef) */
   {0xC6,0xA6,0xA6,0x95,7} ,      /* char 166 --> yeh hamza */
   {0xC8,0xA7,0xA7,0xC8,5} ,      /* char 167 --> beh */
   {0xCA,0xA8,0xA8,0xCA,5},       /* char 168 --> teh */
   {0xCB,0xA9,0xA9,0xCB,5} ,      /* char 169 --> theh */
   {0xCC,0xAA,0xAA,0xCC,5} ,      /* char 170 --> geem */
   {0xCD,0xAB,0xAB,0xCD,5} ,      /* char 171 --> hah */
   {0xAC,0xAC,0xAC,0xAC,0},       /* char 172 --> comma */
   {0xAD,0xAD,0xAD,0xAD,5},       /* char 173 --> wasla */
   {0xCE,0xAE,0xAE,0xCE,5} ,      /* char 174 --> khah */
   {0xD3,0xAF,0xAF,0xD3,6} ,      /* char 175 --> seen */
   {0xB0,0xB0,0xB0,0xB0,0},       /* char 176 --> zero */
   {0xB1,0xB1,0xB1,0xB1,0},       /* char 177 --> one */
   {0xB2,0xB2,0xB2,0xB2,0},       /* char 178 --> two */
   {0xB3,0xB3,0xB3,0xB3,0},       /* char 179 --> three */
   {0xB4,0xB4,0xB4,0xB4,0},       /* char 180 --> four */
   {0xB5,0xB5,0xB5,0xB5,0},       /* char 181 --> five */
   {0xB6,0xB6,0xB6,0xB6,0},       /* char 182 --> six */
   {0xB7,0xB7,0xB7,0xB7,0},       /* char 183 --> seven */
   {0xB8,0xB8,0xB8,0xB8,0},       /* char 184 --> eight */
   {0xB9,0xB9,0xB9,0xB9,0},       /* char 185 --> nine */
   {0xD4,0xBA,0xBA,0xD4,6},       /* char 186 --> sheen */
   {0xBB,0xBB,0xBB,0xBB,0},       /* char 187 --> semi colon */
   {0xD5,0xBC,0xBC,0xD5,6},       /* char 188 --> sad */
   {0xD6,0xBD,0xBD,0xD6,6},       /* char 189 --> dad */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 190 --> ein */
   {0xBF,0xBF,0xBF,0xBF,0},       /* char 191 --> question mark */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 192 --> ein */
   {0xC1,0xC1,0xC1,0xC1,0},       /* char 193 --> hamza */
   {0xC2,0xC2,0xDC,0xDC,3},       /* char 194 --> alef madda */
   {0xC3,0xC3,0xDD,0xDD,3},       /* char 195 --> alef hamza */
   {0xC4,0xC4,0xC4,0xC4,4},       /* char 196 --> waw hamza */
   {0xC5,0xC5,0x80,0x80,3},       /* char 197 --> alef hamza under */
   {0xC6,0xA6,0xA6,0x95,7},       /* char 198 --> yeh hamza */
   {0xC7,0xC7,0xDE,0xDE,3},       /* char 199 --> alef */
   {0xC8,0xA7,0xA7,0xC8,5},       /* char 200 --> beh */
   {0xC9,0xC9,0xC9,0xC9,4} ,      /* char 201 --> teh marbouta */
   {0xCA,0xA8,0xA8,0xCA,5},       /* char 202 --> teh */
   {0xCB,0xA9,0xA9,0xCB,5},       /* char 203 --> theh */
   {0xCC,0xAA,0xAA,0xCC,5},       /* char 204 --> geem */
   {0xCD,0xAB,0xAB,0xCD,5} ,      /* char 205 --> hah */
   {0xCE,0xAE,0xAE,0xCE,5},       /* char 206 --> khah */
   {0xCF,0xCF,0xCF,0xCF,4},       /* char 207 --> dal */
   {0xD0,0xD0,0xD0,0xD0,4},       /* char 208 --> thal */
   {0xD1,0xD1,0xD1,0xD1,4},       /* char 209 --> reh */
   {0xD2,0xD2,0xD2,0xD2,4},       /* char 210 --> zeen */
   {0xD3,0xAF,0xAF,0xD3,6},       /* char 211 --> seen */
   {0xD4,0xBA,0xBA,0xD4,6},       /* char 212 --> sheen */
   {0xD5,0xBC,0xBC,0xD5,6},       /* char 213 --> sad */
   {0xD6,0xBD,0xBD,0xD6,6},       /* char 214 --> dad */
   {0xD7,0xD7,0xD7,0xD7,5},       /* char 215 --> tah */
   {0xD8,0xD8,0xD8,0xD8,5},       /* char 216 --> thah */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 217 --> ein */
   {0xDA,0x9A,0x9B,0x99,5},       /* char 218 --> ghein */
   {0xD9,0xC0,0xDB,0xBE,5},       /* char 219 --> ein */
   {0xC2,0xC2,0xDC,0xDC,3},       /* char 220 --> alef madda */
   {0xC3,0xC3,0xDD,0xDD,3},       /* char 221 --> alef hamza */
   {0xC7,0xC7,0xDE,0xDE,3},       /* char 222 --> alef */
   {0xE1,0xDF,0xDF,0xE1,5},       /* char 223 --> feh */
   {0xE0,0xE0,0xE0,0xE0,5},       /* char 224 --> wasla */
   {0xE1,0xDF,0xDF,0xE1,5},       /* char 225 --> feh */
   {0xE2,0xF3,0xF3,0xE2,5},       /* char 226 --> quaf */
   {0xE3,0xF4,0xF4,0xE3,5},       /* char 227 --> kaf */
   {0xE4,0xF5,0xF5,0xE4,8},       /* char 228 --> lam */
   {0xE5,0xFB,0xFB,0xE5,5},       /* char 229 --> meem */
   {0xE6,0xFC,0xFC,0xE6,5},       /* char 230 --> noun */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 231 --> heh */
   {0xE8,0xE8,0xE8,0xE8,4},       /* char 232 --> waw */
   {0xE9,0xE9,0x96,0x96,4},       /* char 233 --> yeh */
   {0xEA,0x97,0x97,0x98,5},       /* char 234 --> yeh nokteten */
   {0xEB,0xEB,0x87,0xEB,9},       /* char 235 --> fathatan */
   {0xEC,0xEC,0xEC,0xEC,1},       /* char 236 --> damattan */
   {0xED,0xED,0xED,0xED,1},       /* char 237 --> kassratan */
   {0xEE,0xEE,0x94,0xEE,9},       /* char 238 --> fatha */
   {0xEF,0xEF,0x90,0xEF,9},       /* char 239 --> damma */
   {0xF0,0xF0,0x91,0xF0,9},       /* char 240 --> kasra */
   {0xF1,0xF1,0x92,0xF1,9},       /* char 241 --> shadda */
   {0xF2,0xF2,0x93,0xF2,9},       /* char 242 --> soukoun */
   {0xE2,0xF3,0xF3,0xE2,5},       /* char 243 --> quaf */
   {0xE3,0xF4,0xF4,0xE3,5},       /* char 244 --> kaf */
   {0xE4,0xF5,0xF5,0xE4,8},       /* char 245 --> lam */
   {0xF6,0xF6,0xF6,0xF6,2},       /* char 246 --> tail */
   {0xF7,0xF7,0x9C,0x9C,3},       /* char 247 --> lam alef madda */
   {0xF8,0xF8,0x9D,0x9D,3},       /* char 248 --> lam alef hamza */
   {0xF9,0xF9,0x9E,0x9E,3},       /* char 249 --> lam alef hamza under */
   {0xFA,0xFA,0x9F,0x9F,3},       /* char 250 --> lam alef */
   {0xE5,0xFB,0xFB,0xE5,5},       /* char 251 --> meem */
   {0xE6,0xFC,0xFC,0xE6,5},       /* char 252 --> noun */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 253 --> heh */
   {0xFE,0xE7,0xFD,0xFE,5},       /* char 254 --> heh */
   {0xFF,0xFF,0xFF,0xFF,0}        /* char 255 --> blank */
};
/************************************************************************/
/* some special characters */
#define   START_NATIONAL       0x80
#define   SPACE                0x20

/* Arabic codepoints */
#define   A_RSP                  0xa0
#define   A_TAIL                 0xF6
#define   A_YEH_FINAL            0x96
#define   A_YEH_ISOLATED         0xE9
#define   A_HAMZA                0xC1
#define   A_YEH_HAMZA_FINAL      0x95
#define   A_YEH_HAMZA_ISOLATED   0xC6
#define   A_YEH_HAMZA_INITIAL    0xA6
#define   A_HEH_INITIAL          0xE7
#define   A_HEH_ISOLATED         0xFE
#define   A_INITIAL_SEEN         0xAF
#define   A_INITIAL_SHEEN        0xBA
#define   A_INITIAL_SAD          0xBC
#define   A_INITIAL_DAD          0xBD
#define   A_THREEQUARTER_SEEN    0x83
#define   A_THREEQUARTER_SHEEN   0x84
#define   A_THREEQUARTER_SAD     0x85
#define   A_THREEQUARTER_DAD     0x86
#define   A_ONECELL_SEEN         0xD3
#define   A_ONECELL_SHEEN        0xD4
#define   A_ONECELL_SAD          0xD5
#define   A_ONECELL_DAD          0xD6
#define   A_ALEF_FINAL           0xDE
#define   A_ALEF_ISOLATED        0xC7
#define   A_ALEF_SPECIAL         0xA5
#define   A_ALEF_HAMZA_FINAL     0xDD
#define   A_ALEF_HAMZA_ISOLATED  0xC3
#define   A_ALEF_HAMZA_SPECIAL   0xA2
#define   A_ALEF_MADDA_FINAL     0xDC
#define   A_ALEF_MADDA_ISOLATED  0xC2
#define   A_ALEF_MADDA_SPECIAL   0xA1
#define   A_ALEF_HAMZA_UNDER_FINAL 0x80
#define   A_ALEF_HAMZA_UNDER_ISOLATED 0xC5
#define   A_ALEF_HAMZA_UNDER_SPECIAL 0xA3
#define   A_LAM_ALEF_ISOLATED    0xFA
#define   A_LAM_ALEF_CONNECTED   0x9F
#define   A_LAM_ALEF_HAMZA_ISOLATED 0xF8
#define   A_LAM_ALEF_HAMZA_CONNECTED 0x9D
#define   A_LAM_ALEF_MADDA_ISOLATED  0xF7
#define   A_LAM_ALEF_MADDA_CONNECTED 0x9C
#define   A_LAM_ALEF_HAMZA_UNDER_ISOLATED 0xF9
#define   A_LAM_ALEF_HAMZA_UNDER_CONNECTED 0x9E
#define   A_LAM_ISOLATED         0xE4
#define   A_LAM_CONNECTED        0xF5

/************************************************************************/
/* function prototypes */
int Group();
unsigned char IsoFinalShape();
unsigned char YehFinal();
unsigned char InitialShape();
unsigned char InitMidShape();
unsigned char ThreeQuarterSeen();
unsigned char SpecialAlef();
unsigned char IsoLamAlef();
unsigned char ConnLamAlef();
void reset_alefs();
void reset_tail();
