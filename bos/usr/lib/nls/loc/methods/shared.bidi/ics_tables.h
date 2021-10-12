/* @(#)46	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/ics_tables.h, cfgnls, bos411, 9428A410j 8/30/93 15:03:07 */
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

static int LTR_state [24][11] = {
                               /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */
/* 0  L0 Initial */             { 0,0, 1, 4, 1,  1, 13, 13,13,   0,  2},
/* 1  L0 Text */                { 0,0, 1, 3, 1,  1,  1,  1, 1,   1,  2},
/* 2  L0 Text+RSP */            { 0,0, 1, 3, 1,  1,  1,  1, 1,   0,  2},
/* 3  L1 Text */                { 1,0, 1, 3, 1,  7, 19, 15,15,  16,  5},
/* 4  L1 Text SA */             { 1,0, 1, 4, 1, 10, 20,  4, 4,  17,  6},
/* 5  L1 Text+RSP */            { 1,0, 1, 3, 1,  7, 19, 15, 15,  0,  5},
/* 6  L1 Text+RSP SA */         { 1,0, 1, 4, 1, 10, 20,  4,  4,  0,  6},
/* 7  Numeric */                { 2,0, 1, 3, 1,  7,  8, 21, 15, 16,  9},
/* 8  Numeric+suffix */         { 2,0, 1, 3, 1,  7, 19, 15, 15, 16,  9},
/* 9  Numeric+RSP */            { 2,0, 1, 3, 1,  7, 19, 15, 15,  0,  9},
/* 10 Numeric SA */             { 2,0, 1, 4, 1, 10, 11, 22,  4, 17, 12},
/* 11 Numeric+suffix SA */      { 2,0, 1, 4, 1, 10, 20,  4,  4, 17, 12},
/* 12 Numeric+RSP SA */         { 2,0, 1, 4, 1, 10, 20,  4,  4,  0, 12},
/* 13 Neutrals SA */            {-1,0, 1, 4, 1,  1, 13, 13, 13,  1, 14},
/* 14 Neutrals+RSP SA */        {-1,0, 1, 4, 1,  1, 13, 13, 13,  0, 14},
/* 15 L1 Continuation */        {-1,0, 1, 3, 1, -7, 19, 15, 15, 16,  5},
/* 16 L1+Space */               {-1,0, 1, 3, 1, -7, 19, 15, 15, 16,  2},
/* 17 L1+Space SA */            {-1,0, 1, 4, 1,-10, 23, 18, 18, 17,  2},
/* 18 L1+Space+Cont. SA */      {-1,0, 1, 4, 1,-10, 23, 18, 18, 17,  6},
/* 19 Prefix */                 {-1,0, 1, 3, 1, -7, 19, 15, 15, 16,  5},
/* 20 Prefix SA */              {-1,0, 1, 4, 1,-10, 20,  4,  4, 17,  6},
/* 21 Numeric Cont. */          {-2,0, 1,-3, 1, -7,-19,-15,-15,-16, -9},
/* 22 Numeric Cont. SA */       {-2,0, 1,-4, 1,-10,-20, -4, -4,-17,-12},
/* 23 L1+Space+Prefix SA */     {-1,0, 1, 4, 1,-10, 23, 18, 18, 17,  6},
                        };     /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */

          /* RTL state table in Arabic. It shows the new state assigned
		  for each character class enountered.                 */

static int RTL_Astate [22][11] = {
                               /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */
/* 0  L0 Initial */             { 0,0, 1, 1, 4,  7, 20, 13, 13,  0,  2},
/* 1  L0 Text */                { 0,0, 1, 1, 3,  7, 19,  1,  1,  1,  2},
/* 2  L0 Text+RSP */            { 0,0, 1, 1, 3,  7, 19,  1,  1,  0,  2},
/* 3  L1 Text */                { 1,0, 1, 1, 3, 10, 15, 15, 15, 16,  5},
/* 4  L1 Text SA */             { 1,0, 1, 1, 4,  4,  4,  4,  4, 17,  6},
/* 5  L1 Text+RSP */            { 1,0, 1, 1, 3, 10, 15, 15, 15,  0,  5},
/* 6  L1 Text+RSP SA */         { 1,0, 1, 1, 4,  4,  4,  4,  4,  0,  6},
/* 7  Numeric */                { 1,0, 1, 1, 3,  7,  8, 21,  1,  1,  9},
/* 8  Numeric+suffix */         { 1,0, 1, 1, 3,  7, 19,  1,  1,  1,  9},
/* 9  Numeric+RSP */            { 1,0, 1, 1, 3,  7, 19,  1,  1,  0,  9},
/* 10 L1 + Numeric */           { 1,0, 1, 1, 3, 10, 11, 15, 15, 16, 12},
/* 11 L1 + Numeric+suffix  */   { 1,0, 1, 1, 3, 10, 15, 15, 15, 16, 12},
/* 12 L1 + Numeric+RSP */       { 1,0, 1, 1, 3, 10, 15, 15, 15,  0, 12},
/* 13 Neutrals SA */            {-1,0, 1, 1, 4, -7, 20, 13, 13, 13,  2},
/* 14 Neutrals+RSP SA */        {-1,0, 1, 1, 4,  1, 19, 13, 13,  0,  2},
/* 15 L1 Continuation */        {-1,0, 1, 1, 3, 10, 15, 15, 15, 16,  5},
/* 16 L1+Space */               {-1,0, 1, 1, 3, 10, 15, 15, 15, 16,  2},
/* 17 L1+Space SA */            {-1,0, 1, 1, 4,  4, 18, 18, 18, 17,  2},
/* 18 L1+Space+Cont. SA */      {-1,0, 1, 1, 4,  4, 18, 18, 18, 17,  6},
/* 19 Prefix */                 {-1,0, 1, 1,-3,  7,-19,  1,  1,  1,  2},
/* 20 SA+Prefix */              {-1,0, 1, 1, 4, -7, 20,  1,  1,  1,  2},
/* 21 Numeric Cont. */          {-1,0, 1, 1, 3,  7,-19,  1,  1,  1,  9},
                        };     /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */

          /* RTL state table in Hebrew. It shows the new state assigned
                  for each character class enountered.                 */

static int RTL_Hstate [22][11] = {
                               /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */
/* 0  L0 Initial */             { 0,0, 1, 1, 4,  4, 13, 13, 13,  0,  2},
/* 1  L0 Text */                { 0,0, 1, 1, 3,  7, 19,  1,  1,  1,  2},
/* 2  L0 Text+RSP */            { 0,0, 1, 1, 3,  7, 19,  1,  1,  0,  2},
/* 3  L1 Text */                { 1,0, 1, 1, 3, 10, 15, 15, 15, 16,  5},
/* 4  L1 Text SA */             { 1,0, 1, 1, 4,  4,  4,  4,  4, 17,  6},
/* 5  L1 Text+RSP */            { 1,0, 1, 1, 3, 10, 15, 15, 15,  0,  5},
/* 6  L1 Text+RSP SA */         { 1,0, 1, 1, 4,  4,  4,  4,  4,  0,  6},
/* 7  Numeric */                { 1,0, 1, 1, 3,  7,  8, 21,  1,  1,  9},
/* 8  Numeric+suffix */         { 1,0, 1, 1, 3,  7, 19,  1,  1,  1,  9},
/* 9  Numeric+RSP */            { 1,0, 1, 1, 3,  9, 19,  1,  1,  0,  9},
/* 10 L1 + Numeric */           { 1,0, 1, 1, 3, 10, 11, 15, 15, 16, 12},
/* 11 L1 + Numeric+suffix  */   { 1,0, 1, 1, 3, 10, 15, 15, 15, 16, 12},
/* 12 L1 + Numeric+RSP */       { 1,0, 1, 1, 3, 10, 15, 15, 15,  0, 12},
/* 13 Neutrals SA */            {-1,0, 1, 1, 4,  4, 13, 13, 13,  1, 14},
/* 14 Neutrals+RSP SA */        {-1,0, 1, 1, 4,  4, 13, 13, 13,  0, 14},
/* 15 L1 Continuation */        {-1,0, 1, 1, 3, 10, 15, 15, 15, 16,  5},
/* 16 L1+Space */               {-1,0, 1, 1, 3, 10, 15, 15, 15, 16,  2},
/* 17 L1+Space SA */            {-1,0, 1, 1, 4,  4, 18, 18, 18, 17,  2},
/* 18 L1+Space+Cont  SA */      {-1,0, 1, 1, 4,  4, 18, 18, 18, 17,  6},
/* 19 Prefix  */                {-1,0, 1, 1,-3,  7,-19,  1,  1,  1,  2},
/* 20 Prefix SA */              {-0,0, 0, 0, 0,  0,  0,  0,  0,  0,  0},
/* 21 Numeric Cont. */          {-1,0, 1, 1, 3,  7,-19,  1,  1,  1,  9},
                        };     /*   I, Z, R, L,  D, X,   P,  N,  S,  Q */

