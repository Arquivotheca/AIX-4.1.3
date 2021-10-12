/* @(#)02	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/compose.h, libmeth, bos411, 9428A410j 9/10/93 11:19:16 */
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
/************************************************************************/
static char Vowels [128] = 
/* In this table, all consonants are mapped to 0x00,
   and all vowels are mapped to their isolated form. */
   {0x00,       /* char 128 --> hamza under alef */
    0x00,       /* char 129 --> times */
    0x00,       /* char 130 --> divided */
    0x00,       /* char 131 --> seen */
    0x00,       /* char 132 --> sheen */
    0x00,       /* char 133 --> sad */
    0x00,       /* char 134 --> dad */
    0xEB,       /* char 135 --> fathatan */
    0x00,       /* char 136 --> blank */
    0x00,       /* char 137 --> box character */
    0x00,       /* char 138 --> box character */
    0x00,       /* char 139 --> box character */
    0x00,       /* char 140 --> box character */
    0x00,       /* char 141 --> box character */
    0x00,       /* char 142 --> box character */
    0x00,       /* char 143 --> box character */
    0xEF,       /* char 144 --> damma */
    0xF0,       /* char 145 --> kasra */
    0xF1,       /* char 146 --> shadda */
    0xF2,       /* char 147 --> soukoun */
    0xEE,       /* char 148 --> fatha */
    0x00,       /* char 149 --> yeh hamza */
    0x00,       /* char 150 --> yeh */
    0x00,       /* char 151 --> yeh nokteten */
    0x00,       /* char 152 --> yeh nokteten */
    0x00,       /* char 153 --> gheen */
    0x00,       /* char 154 --> gheen */
    0x00,       /* char 155 --> gheen */
    0x00,       /* char 156 --> lam alef madda */
    0x00,       /* char 157 --> lam alef hamza */
    0x00,       /* char 158 --> lam alef hamza under */
    0x00,       /* char 159 --> lam alef */
    0x00,       /* char 160 --> RSP */
    0x00,       /* char 161 --> special alef madda */
    0x00,       /* char 162 --> special alef hamza */
    0x00,       /* char 163 --> special alef hamza under */
    0x00,       /* char 164 --> sign */
    0x00,       /* char 165 --> special alef (of lamalef) */
    0x00 ,      /* char 166 --> yeh hamza */
    0x00 ,      /* char 167 --> beh */
    0x00,       /* char 168 --> teh */
    0x00 ,      /* char 169 --> theh */
    0x00 ,      /* char 170 --> geem */
    0x00 ,      /* char 171 --> hah */
    0x00,       /* char 172 --> comma */
    0x00,       /* char 173 --> wasla */
    0x00 ,      /* char 174 --> khah */
    0x00 ,      /* char 175 --> seen */
    0x00,       /* char 176 --> zero */
    0x00,       /* char 177 --> one */
    0x00,       /* char 178 --> two */
    0x00,       /* char 179 --> three */
    0x00,       /* char 180 --> four */
    0x00,       /* char 181 --> five */
    0x00,       /* char 182 --> six */
    0x00,       /* char 183 --> seven */
    0x00,       /* char 184 --> eight */
    0x00,       /* char 185 --> nine */
    0x00,       /* char 186 --> sheen */
    0x00,       /* char 187 --> semi colon */
    0x00,       /* char 188 --> sad */
    0x00,       /* char 189 --> dad */
    0x00,       /* char 190 --> ein */
    0x00,       /* char 191 --> question mark */
    0x00,       /* char 192 --> ein */
    0x00,       /* char 193 --> hamza */
    0x00,       /* char 194 --> alef madda */
    0x00,       /* char 195 --> alef hamza */
    0x00,       /* char 196 --> waw hamza */
    0x00,       /* char 197 --> alef hamza under */
    0x00,       /* char 198 --> yeh hamza */
    0x00,       /* char 199 --> alef */
    0x00,       /* char 200 --> beh */
    0x00,       /* char 201 --> teh marbouta */
    0x00,       /* char 202 --> teh */
    0x00,       /* char 203 --> theh */
    0x00,       /* char 204 --> geem */
    0x00 ,      /* char 205 --> hah */
    0x00,       /* char 206 --> khah */
    0x00,       /* char 207 --> dal */
    0x00,       /* char 208 --> thal */
    0x00,       /* char 209 --> reh */
    0x00,       /* char 210 --> zeen */
    0x00,       /* char 211 --> seen */
    0x00,       /* char 212 --> sheen */
    0x00,       /* char 213 --> sad */
    0x00,       /* char 214 --> dad */
    0x00,       /* char 215 --> tah */
    0x00,       /* char 216 --> thah */
    0x00,       /* char 217 --> ein */
    0x00,       /* char 218 --> ghein */
    0x00,       /* char 219 --> ein */
    0x00,       /* char 220 --> alef madda */
    0x00,       /* char 221 --> alef hamza */
    0x00,       /* char 222 --> alef */
    0x00,       /* char 223 --> feh */
    0x00,       /* char 224 --> wasla */
    0x00,       /* char 225 --> feh */
    0x00,       /* char 226 --> quaf */
    0x00,       /* char 227 --> kaf */
    0x00,       /* char 228 --> lam */
    0x00,       /* char 229 --> meem */
    0x00,       /* char 230 --> noun */
    0x00,       /* char 231 --> heh */
    0x00,       /* char 232 --> waw */
    0x00,       /* char 233 --> yeh */
    0x00,       /* char 234 --> yeh nokteten */
    0xEB,       /* char 235 --> fathatan */
    0xEC,       /* char 236 --> damattan */
    0xED,       /* char 237 --> kassratan */
    0xEE,       /* char 238 --> fatha */
    0xEF,       /* char 239 --> damma */
    0xF0,       /* char 240 --> kasra */
    0xF1,       /* char 241 --> shadda */
    0xF2,       /* char 242 --> soukoun */
    0x00,       /* char 243 --> quaf */
    0x00,       /* char 244 --> kaf */
    0x00,       /* char 245 --> lam */
    0x00,       /* char 246 --> tail */
    0x00,       /* char 247 --> lam alef madda */
    0x00,       /* char 248 --> lam alef hamza */
    0x00,       /* char 249 --> lam alef hamza under */
    0x00,       /* char 250 --> lam alef */
    0x00,       /* char 251 --> meem */
    0x00,       /* char 252 --> noun */
    0x00,       /* char 253 --> heh */
    0x00,       /* char 254 --> heh */
    0x00};      /* char 255 --> blank */

