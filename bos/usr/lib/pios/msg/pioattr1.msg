$ @(#)64	1.16.1.18  src/bos/usr/lib/pios/msg/pioattr1.msg, cmdpios, bos411, 9428A410j 6/21/94 10:36:02
$
$   COMPONENT_NAME: CMDPIOS
$
$   FUNCTIONS: 
$
$   ORIGINS: 27, 83
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$   LEVEL 1, 5 Years Bull Confidential Information
$
$       Descriptions Of Printer Backend Data Base Attributes
$
$set 1
1 Command Line Flags Prohibited For Input Data Stream; Ignored: cmnrBDMPRT
3 (not used)
13 stderr returned? 0: no;  1: yes, & pipelines;  2: yes, & values, pipelines
17 Double spacing flag (!; no = single  +; yes = double)
18 Double-High Print. (!: no;  +: yes)
19 Italics print (!: no;  +: yes)
20 (not used) Font file name
22 Name To Replace Host Name On Burst Page
23 (not used) Font ID
24 Font ID (overrides pitch and type style)
25 Font ID.  Use internal fonts if blank.
27 Restore the Printer at the End of the Print Job? (!: no;  +: yes)
29 Condensed Print (!: no;  +: yes)
30 Wrap Long Lines (!: no;  +: yes)
31 (not used) Page format flag
32 Page format (!: use only printable page   +: use entire addressable area)
33 Type of Input Paper Handling
34 Sheet Feeder (0: Disabled or not available  1: Enabled)
35 Type of Input Paper Handling (1: manual, 3: sheetfeed)
36 Type of Input Paper Handling (1: manual, 2: pinwheel, 3: sheetfeed)
38 (reserved for future use)
40 High speed printing
42 Unidirectional printing
43 Vertical printing
44 Continuous double wide
45 Double-Wide Print (!: no;  +: yes)
46 Code Page Name For Print Data Stream (file with same name in dir. "d1")
47 Duplex Output
48 Duplex Output (0: Simplex  1: Duplex  2: Duplex with tumble)
50 Issue Form Feed Between Copies & At Job End (!: no;  +: yes)
51 COMMAND AGGREGATES
52 CONTROL CODES (ASCII)
53 DIRECTORIES
54 ESCAPE SEQUENCES
55 COMMAND STRINGS FOR FILTER FLAGS (2 char, 1st="f",2nd=flag)
56 VALUES THAT MAY BE OVERRIDDEN WITH FLAGS ON THE COMMAND LINE
57 PIPELINES FOR INPUT DATA STREAMS (2 char, 1st="i",2nd=data stream name)
58 MISCELLANEOUS
59 FLAGS PROHIBITED FOR INPUT DATA STREAMS (2 char,1st="I",2nd=data str name)
60 OTHER VALUES OF INTEREST TO THE SYSTEM ADMINISTRATOR
61 STAGE 2 TRANSLATE TABLES
62 WORK VARIABLES
63 0: Normal Print;  1: Return Flag Values & Pipeline, But Don't Print
64 Bottom Margin (number of blank lines); Sometimes Called Skip Perf.
66 Input Data Stream Type
69 Emphasized Print (!: no;  +: yes)
70 Single-Character BSD-type Filter Name (e.g., "p" for "pr" filter)
71 Page Number Where Printing Should Begin
72 Title Used For "pr" Filter When "-f p" Is Specified
73 Indent Value (characters)
76 Initialize the printer (for each print file)? (!: no;  +: yes)
78 Print Color
79 Page Length (lines; 0 means ignore page length)
83 Pitch (characters per inch)
84 Pitch (characters per inch)  Initially set to value implied by font ID.
85 (not used) Print Quality
86 Print Quality (0:12 cpi fastfont;  1:10 cpi draft;  2:NLQ;  3:NLQ II)
87 Print Quality (1: draft;  2: letter  3:NLQ)
88 Print Quality (1: draft;  2: letter)
90 Type Style (name)
91 Top Margin (number of blank lines)
92 Paper Source Drawer
93 Paper Source Drawer (1: Lower  2: Upper or no sheet feeder  3: Envelopes)
96 Paper Source Drawer (1: Upper or no sheet feeder  2: Lower  3: Envelopes)
98 Paper Source Drawer (1; drawer  2; manual  3; envelope)
99 Line Density (lines per inch) examples: 6, 8
100 Page Width (characters)
101 0: CR->CR, LF->LF, VT->VT;  1: CR->CRLF  2: LF->CRLF, VT->CRVT;
103 Double-Strike Print (!: no;  +: yes)
107 Rotate Page Printer Output (!:no - portrait  +:yes - landscape)
108 Rotate Page Printer Output (# quarter turns clockwise: 0-3)
110 ASCII Control Code to Backspace One Character
111 ASCII Control Code to Return to the Left Margin (CR without LF)
112 ASCII Control Code to Cancel Double-Wide Printing by Line
113 ASCII Control Code to Advance the Paper to Top of Next Page (FF)
114 (not used; handled by formatter) Horizontal Tab
116 ASCII Control Code to Shift In (i.e., start condensed printing)
117 ASCII Control Code to Advance the Paper One Line (LF without CR)
118 ASCII Null Character
120 ASCII Control Code to Shift Out (i.e., set double-wide for the line)
121 ASCII Control Code to Ring the Bell Once
122 ASCII Control Code to Select 10 Char per Inch; Cancels Cond., 12 CPI
123 (not used; handled by formatter) Vertical Tab
125 Select the IBM-437 character set
126 Select Primary (ROM) Font
127 Select the Roman-8 symbol set
129 Select Alternate (downloaded) Font
130 Select the US symbol set
131 Select the ECMA-94 (Latin 1) symbol set
136 Turn on or off overstrike
138 Turn on or off underscore
139 Initialize local font and Select local character set
141 Set Condensed Mode On/Off
142 Number of nulls to send to let printer shift into emulation mode
143 Space Horizontally In Hres Units (used for ident, LF w/o CR)
144 Command To Initialize the Printer
146 Send Carriage Return & Line Feed Controls
147 Command When Switching Between Pri. Mode & Alt. (security label) Mode
148 Set Pitch
149 Set Print Quality
150 Set Print Quality (0: fastfont; 1: draft; 2:NLQ; 3:NLQ II)
151 Set Print Quality (0: fastfont; 1: draft; 2:NLQ; 3:enhanced)
152 Command To Restore the Printer at Job End
154 Set Proportional Spacing On/Off
155 Tell the printer what the Horizontal Motion Index is
156 Set line termination mode
157 Set Vertical Spacing
159 Send Carriage Return & Reverse Line Feed Controls
160 Directory Containing Stage 1 Translate Tables (data stream to intermed.)
161 Directory Containing Stage 2 Translate Tables (intermediate to printer)
162 Directory Containing Header and Trailer Text Files
163 Directory Containing Digested Data Base Files
164 Directory Containing Miscellaneous Modules
165 Directory Containing Loadable Formatter Routines
166 Directory Containing Fonts To Be Downloaded
167 Directory For Temporary Files
168 Set Line Spacing to 8 Lines Per Inch
169 Set Line Spacing to 7/72 Inch Per Line
170 Turn On Line Spacing Specified With SET_LS_CMD
171 Set Graphics Line Spacing
172 Set Top of Form
174 (used only for pass-through init) Turn On/Off Automatic Line Feed With CR
175 (used only for init/restore) Select Character Set 2
176 Select Character Set 1
178 Print All Characters
179 (not used; would conflict with formatter processing) Set Code Page
180 Command to Set Text Line Spacing
182 (used only for init/restore) Set Vertical Tab Stops
183 (used only for init/restore) Set Page Length
184 (used only for init/restore) Set Horizontal Tabs
185 Turn On/Off Emphasized Printing
187 Select font if specified
188 Specify sheetfeed options
189 Turn On/Off Double-Strike Printing
191 Select Pitch, Type Style, and Quality
192 Select Quality
193 Select Quality or Font
194 Select Quality, Font, and Italic mode
195 Advance the Paper by a Variable Amount
196 Print Graphics at 60 dots/inch hor. & 72 dots/inch vert.
197 Print Graphics at 120 dots/inch hor. & 72 dots/inch vert. (low speed)
199 Select desired print color
200 (not used; handled by formatter) Set Automatic Perforation Skip
201 (used only at init/restore) Cancel Automatic Perforation Skip
202 Specify the page orientation
204 Select pitch - default is 10
205 Turn On/Off Proportional Spacing
206 (not used; handled by formatter) Reset H. Tabs to 8 Pos; Clear Vert. Tabs
208 PCL Reset
209 Turn On/Off Subscript Printing
210 Turn On/Off Superscript Printing
212 Change From/To Left-to-Right To/From Two-Direction Printing
213 Send out a two by escape sequence
214 Turn On/Off Continuous Double-Wide Printing
216 Send out a multi byte escape sequence
217 Set Left and Right Margins
218 Print Graphics at 120 dots/inch hor. & 72 dots/inch vert. (normal spd)
219 Send out a multi byte escape sequence followed by binary data
220 Print Graphics at 240 dots/inch hor. & 72 dots/inch vert.
221 Select the symbol set to be used
223 (used only at init/restore) Turn Off Automatic Line Feed With CR
225 Print a Single Character From the All Characters Chart
227 Set Character Spacing to 12 Characters Per Inch
229 Move to the Right n/120 Inch
231 Move to the Left n/120 Inch
233 Italics Printing
234 Turn On Double-High and/or Double-Wide Printing
235 Turn On/Off Continuous Overscore
237 Select font (global) command
238 Stop Printing
240 Turn On/Off Continuous Underscore
242 Set Vertical Units for VAR_LS_CMD and SET_GRA_LS_CMD
244 Set printer aspect ratio
246 High-Resolution Graphics
248 Pass thru a printer PMP command
250 Download Character Fonts
253 Reverse Line Feed
254 Null string for ignoring pass thru commands - THIS IS USED - DO NOT CHANGE
255 Null string for ignoring pass thru commands
256 Generalized Pass thru command
257 Printer initialization - overrides formatter init. - DO NOT USE !!!
258 Send out a two by escape sequence
259 Send out a three byte escape sequence
261 Alternate Graphics Mode: Print Graphics In Specified Mode
263 (used only on init/restore) Set initial conditions
264 Command String for the "c" Filter.
265 Command String for the "d" Filter.
266 Command String for the "f" Filter.
267 Command String for the "g" Filter.
268 Command String for the "l" Filter.
269 Command String for the "n" Filter.
270 Command String for the "p" Filter.
271 Command String for the "t" Filter.
272 Command String for the "v" Filter.
273 PostScript
274 ASCII
275 PCL
276 GL
277 pass-through
278 Diablo 630
279 Default overstrike character
280 Default overstrike character = '*'
282 Bypass whitespace on overstrike (!: no  +:yes)
284 Bypass whitespace on underscore (!: no  +:yes)
285 Condensed mode type          1;4207m2  2;4201m3
286 Dbl hi/wide type             1;5202    2;3816
287 Path Name of Font File To Be Downloaded (must include download commands)
288 Graphic unit handling flag   1;4207m2  2;4201m3
289 Graphic unit handling flag   1;5202    2;3816
290 Number of Units Into Which a Horizontal Inch Is Divided
291 Esc I processing flag        1;4207m2  2;4201m3
292 Esc I processing flag        1;5202    2;3816
293 Was condensed print set on the command line? (0:No    1: Yes)
294 Does this printer support font management (!: no  +:yes)
295 Printer model number
297 Percentage of Perf. Skip Value For Top Margin (rest for bottom margin)
298 Character To Be Used When Real One Can't Be Printed
299 Number of Units Into Which a Vertical Inch Is Divided
301 String to Send to Printer "mz" Times When Job Is Cancelled
302 Output Data Stream Type (example: ascii); Initialized By "piodigest"
303 Path Name of the Default Formatter (used when running standalone)
304 Input Data Stream Names (1 character, separated by commas) for mp Attribute
305 File Name Of (Digested) Data Base; Init. By "piodigest" (mt.md.mn.mq:mv)
306 Device name (example: lp0); Initialized By "piodigest"
307 Command String to Invoke Device Driver I/F Program (end of pipeline)
308 Strings (separated by commas) That Identify Print File Data Types (see mi)
309 Queue Name; Initialized By "piodigest"
311 Path Name of "read printer" routine ('load'ed by Dev. Dri. I/F pgm)
312 Was the pitch set on the command line? (0:No    1: Yes)
313 Printer type; Initialized By "piodigest"
314 Printer type; Initialized By "piodigest"
316 Virtual Printer (i.e., queue device) Name; Initialized By "piodigest"
317 Maximum Number of Horizontal Tabs That Can Be Defined
318 Maximum Number of Vertical Tabs That Can Be Defined
319 Number of Times To Send String "mc" to Printer When Job Is Cancelled
320 Code page 850 available (!: no  +:yes)
321 Pipeline for Header Page
322 Users, Separated by Commas, to Get Intervention Messages; Null \
String Is Job Submitter
323 Command Line Flags Prohibited For All -d values; Ignored: cmnrDMPRT
324 Pipeline for Trailer Page
325 Width of Attribute Value Area On Header Page (0 means ignore width)
326 Path Name of Stage 2 Translate Table
327 Path Name of First Alternate Stage 2 Translate Table
328 Path Name of Next Alternate Stage 2 Translate Table
330 Printer Data Stream Description
331 Printer Description
332 Name of message catalog Containing Attribute Descriptors
337 Integer Set By Formatter For Use By Various Commands
338 Used internally by formatter for status of fonts (0: ok  1: bad)
339 Work Variable For Vertical Decrement (in Vres units)
340 Work Variable In Which To Save Horizontal Position From Previous Line
341 The group identifier
342 Vertical Index Increment Used By ESC J & ESC 3
343 Work Variable For Horizontal Increment Per Character (in Hres units)
347 Page Length In Chars, Using Length From Data Base (used in pipelines)
348 Points to string which will hold numeric parameters found in datastream
350 Overstrike (!; off  +; on)
351 Char width (1440s) implied by the active font id
352 The parameterized character in an escape sequence
353 Spacing (fixed/prop) implied by the active font id
354 The command specifier
355 Vertical Index Increment Used By ESC A
357 Underscore (!; off  +; on)
358 Work Variable For Vertical Increment (in Vres units)
359 Page Width In Characters, Using Width From Data Base (used in pipelines)
360 Rotate page
362 Currently In Subscript Mode? (!: no;  +: yes)
363 Currently In "line" Doublewide Mode? (!: no;  +: yes)
364 Currently In Superscript Mode? (!: no;  +: yes)
365 Name of Current Form Feed Command
366 Graphics Line Spacing Used By SET_GRA_LS_CMD
367 Current Horizontal Tab Settings (terminated by null)
368 Name of Current Vertical Increment Command
369 Number of Lines to Space Down For Each Line Feed
370 Smallest legal sheetfeeder drawer number
371 Current Vertical Spacing Mode (0: ESC 0; 1: ESC 1; 2: ESC 2; 3: ESC 3)
372 Currently In Continuous Overscore Mode? (!: no;  +: yes)
373 Work Variable for Pass-Through Instead of Formatting (i.e., data stream is not modified) - DO NOT CHANGE
375 Currently In One-Direction Print Mode? (!: no;  +: yes)
376 Currently In Proportional Spacing Mode?  Init. value depends on font ID.
378 Currently In Proportional Spacing Mode? (!: no;  +; yes)
379 Line Spacing: (value/Tunits) inches per line; used by SET_LS_CMD
380 Currently In Continuous Underscore Mode? (!: no;  +: yes)
381 Largest legal sheetfeeder drawer number
382 Current Vertical Tab Settings (terminated by null)
383 Work Variable For Page Width (in Hres units)
384 Name of Current Vert. Decrement Command
385 Print Quality - 1 is the only legal value - DO NOT CHANGE
387 Does Escape X set quality and pitch (!: no;  +: yes)
388 Is used to determine whether or not the printer is to re-load its fonts
389 Currently In "line" Doublewide Mode? (!: no;  +: yes)
390 Currently In "line" Doublehigh Mode? (!: no;  +: yes)
391 Set pitch
392 Set double hi mode
393 Turn On/Off Continuous Double-Wide Printing
394 Turn On Line Double-High Printing (Printronix 9012 only)
395 Turn On/Off Continuous Overscore
396 Send character font data
399 Does Escape X set quality and pitch (!: no;  +: yes)
406 Turn On/Off Continuous Double-Wide Printing
407 Turn On Line Double-High Printing
412 Select alternate translate table
413 Pass thru graphics images
414 Directory Containing Flags files (keeps track of loaded fonts)
415 Maximum number of fonts to cache (0 - 24)
416 Listing of allowable typeface/pitch combinations
417 Name of typeface/pitch loaded by printer initialization
418 Pitch value in string format - used by the formatter - do not modify
419 Font number for cached font set - set by the formatter
420 COMMANDS TO MANAGE FONTS
421 COMMANDS TO CONVERT CODE PAGE P0 (437) TO CODE PAGE P1 (850)
422 Copy Fonts From Cache To Fonts 0-6
423 Copy Fonts From Fonts 0-6 To Cache
424 Initialize Fonts 0-6
425 Unload Fonts 0-6
426 Logic Included By Commands F0-F6, FA-FB
427 Load Font For Code Page P1 As Font 7
428 Load Bold Font For Code Page P1 As Font 7
429 Unload Font 7
430 Load Base Font
431 Load Bold Font
432 Load Double-Wide Font
433 Load Bold Double-Wide Font
434 Load Condensed Font
435 Load Bold Condensed Font
436 Load Superscript/Subscript Font
437 (font number is in internal variable f; P1 font is Font 7)
438 Activate font
439 Copy Characters From CP437 Code Points To CP850 Code Points
440 Copy Characters From P1 To P0 (437) To Create Code Page 850
441 Continue Copying Characters From P1 To P0 (437) To Create Code Page 850
442 Unload Font 7
443 Delay between print jobs of differing type (seconds) - increase value \
to increase reliability
444 Pipeline prefix to control and record the status of the printer
445 Pipeline suffix to record the time of completion of this pipeline
446 Paper Size For Manual Feed (1: Letter;  2: Legal;  3: B5;  4: A4;  5: Exec)
447 Paper Size For Tray 1 (1: Letter;  2: Legal;  3: B5;  4: A4;  5: Exec)
448 Paper Size For Tray 2 (1: Letter;  2: Legal;  3: B5;  4: A4;  5: Exec)
449 Envelope Size For Automatic Feed (1: 7 3/4;  2: 9;  3: 10;  4: DL;  \
5: C5;  6: B5;  7: Other)
450 Envelope Size For Manual Feed (1: 7 3/4;  2: 9;  3: 10;  4: DL;  \
5: C5;  6: B5;  7: Other)
451 Paper or Envelope Size For the Paper Source Selected By the -O and -u \
Flag Values (Refer to the s0, s1, s2, s3, and s4 attributes); \
Default value: %IwQ
452 m0-m4 Attributes Are s0-s4 Attributes Updated To Reflect -Q Flag Value \
Specified On Command Line
453 Paper Source (1: Tray 1;  2: Tray 2;  3: Automatic Envelope Feeder)
454 Default Page Width (characters)
455 Default Page Length (lines)
456 Primary Page Width (-z 0) or Secondary Page Length (-z 1), in pels
457 Primary Page Length (-z 0) or Secondary Page Width (-z 1), in pels
458 Paper or Envelope Size For the Paper Source Selected By the -O and -u \
Flag Values (Refer to the s0, s1, s2, s3, and s4 attributes)
459 Page Length (lines); Default Value: %IwY \
(value based on paper size specified with s0 - s5 attributes)
460 Page Width (characters); Default Value: %IwX (value based on paper size \
specified with s0 - s5 attributes)
461 Page Width (pels) For Portrait Orientation
462 Page Length (pels) For Portrait Orientation
463 Page Width (pels) For Landscape Orientation
464 Page Length (pels) For Landscape Orientation
465 troff (ditroff) intermediate output
466 Page Headings Wanted For Text Converted to PostScript? (!: no;  +: yes)
467 Use Two Columns for Text Converted to PostScript? (!: no;  +: yes)
468
469 Gaudy Mode Wanted for Text Converted to PostScript? (!: no;  +: yes)
470 Print Garbage File Anyway for Text Converted to PostScript? \
(!: no;  +: yes)
471 List Characters Not In Font When Converting Text to PostScript? \
(!: no;  +: yes)
472 Font Name for Header Line of Text Converted to PostScript
473 Main Part of PostScript Pipeline
474 State of the File When in the "custom" Directory \
(!: inherited attributes excluded; +: inherited attributes included)
$ Do not translate "custom"
475 Attributes reserved for system use
476 Parent File From Which Attributes Are Inherited \
(assumed to be in the "predef" directory)
$ Do not translate "predef"
477 (DO NOT CHANGE THIS VALUE) Current State of the File \
(!: inherited attributes excluded; +: inherited attributes included)
478 Paper Size for the Paper Source Selected by the -u Flag Value \
(refer to the s1 and s2 attributes); Default Value: %IwQ
479 Paper Source Drawer (1: primary (top);  2: alternate (bottom))
480 Paper Size for Primary Drawer (1: Letter;  2: Legal;  3: B5;  \
4: A4;  5: Exec 1;  6: Exec 2;  7: Exec 3)
481 Paper Size for Alternate Drawer (1: Letter;  2: Legal;  3: B5;  \
4: A4;  5: Exec 1;  6: Exec 2;  7: Exec 3)
482 Set Page Size
483 Formatting Flags for Header Page
484 Formatting Flags for Trailer Page
485 Link Input Paper Drawers
486 Formatting Flags for Header Page (example: -u2 specifies the alternate \
paper drawer)
487 Formatting Flags for Trailer Page (example: -u2 specifies the alternate \
paper drawer)
488 Select IBM Personal Computer Code Page 437 (United States)
489 Select IBM Personal Computer Code Page 850 (Multilingual)
490 Select IBM Personal Computer Code Page 851 (Greece)
491 Select IBM Personal Computer Code Page 853 (Latin 3)
492 Select IBM Personal Computer Code Page 857 (Latin 5, Turkey)
493 Select IBM Personal Computer Code Page 869 (Greece)
494 Select IBM Personal Computer Code Page 852 (Latin 2)
495 Select the Denmark/Norway symbol set
496 Select Printer Code Page P0
497 Select Printer Code Page P1
498 Select Printer Code Page P2
499 Path Names of Stage 2 Translate Tables, Separated by Commas. The \
formatter will prepend path names of stage 2 translate tables derived \
from code page information of resident and cartridge fonts defined by \
the mU attribute.
500 PIPELINES FOR HEADER AND TRAILER PAGES
501 Identifies the Font Card Installed in the Top Font Card Slot. The \
Two-Digit Number Can be Found on the Font Card Just Above the Arrow.
502 Identifies the Font Card Installed in the Bottom Font Card Slot. The \
Two-Digit Number Can be Found on the Font Card Just Above the Arrow.
503 FONT CARD DEFINITIONS
504 Description of Resident Fonts
505 Description of Font Card 15 (Greek)
506 Description of Font Card 10 (Turkey)
507 Include Description of Font Card Identified by Internal Variable i.
508 Download Updated Form Feed Macro With Paper Drawer Specified
509 Directory Containing Dummy Device Files For Printers Driven By, But Not \
Attached To, the Host Computer (example: printers attached to Xstations)
510 Path Names of Stage 2 Translate Tables, Separated by Commas
511 Duplex Output (0: Simplex  1: Duplex Long-Edge  2: Duplex Short-Edge)
512 PostScript Command to Set Simplex/Duplex and Tumble Mode
513 Set Duplex/Simplex Mode
514 Command to Restore Printer Datastream Mode (used only on restore)
515 Does the PostScript Option Have Automatic Mode Switching? \
(!: no;  +: yes)
516 Datastream Mode to Which Printer is Restored at End of Job \
(0: IBM PPDS;  1: HP PCL;  2: Plotter;  3: PostScript)
517 Page Width in Current Orientation (pels)
518 Page Length in Current Orientation (pels)
519 Effective Line Density (lines per inch x 100)
520 Part Number of the Font Cartridge Installed in the Cartridge Slot
521 Identifies the Printed Language Selected from the Operator Panel \
(02: PC Multilingual;  08: Greek;  10: Turkish)
522 Description of Font Card 01 (Italic)
523 Description of Font Card 02 (Prestige)
524 Description of Font Card 03 (Letter Gothic)
525 Description of Font Card 04 (Delegate)
526 Description of Font Card 05 (Script/OCR)
527 Description of Font Card 07 (Compressed)
528 Description of Font Card 16 (Latin 2)
529 Description of Font Card 24 (Combo (Data Stream 1))
530 Return TRUE in Internal Variable r if Resident Code Page \
Exists For Requested Typestyle and Pitch.
531 Full Path Name of Font File to Download
532 Download Font File (if needed) and Select Code Page Specified in \
Internal Variable c
533 Font Diskette Installed in Printer (P0: Feature Code 3155, \
Part 11F7734;  CP851: Language Group 3 (Code Page 851 - Greece);  \
CP853: Language Group 3 (Code Page 853 - Turkey)
534 Font Diskette Installed in Printer (MLP: Feature Code 7652, \
CP851: Language Group 3 (Code Page 851 - Greece);  \
CP857: Language Group 3 (Code Page 857 - Turkey)
535 Set Code Page
536 Path Names of Stage 2 Translate Tables to be Used (separated \
by commas)
537 Description of Font Card 29 (Presentations)
538 Description of Font Card 27 (NLS I)
539 Description of Font Card 27 (NLS I) Continued
540 PostScript Command to Set the Paper Source
$ Note: do not translate PostScript
541 PostScript Command to Set the Print Resolution
$ Note: do not translate PostScript
542 Description of Font Card 14 (Japan)
543 Initialize Download Font Area at Start and End of Job? \
(!: no;  +: yes)
544 Description of Font Card 33 (Word Processing)
545 Does PostScript Support Duplex Setting in statusdict? \
(!: no;  +: yes)
$ Note: do not translate PostScript or statusdict
546 Description of Font Card 32 (OCR/Symbol)
547 Pass mk Attribute String to Shell to Determine inode Number \
of Downloaded Font File (if any)
$ Note: do not translate inode
548 String Passed to Shell to Determine inode Number of Downloaded \
Font File
$ Note: do not translate inode
549 String Passed to Shell to Save inode Number of Downloaded Font File
$ Note: do not translate inode
550 Pass cX Attribute String to Shell to Determine Font File's \
inode Number.
$ Note: do not translate inode
551 String Passed to Shell to Determine Font File's inode Number
$ Note: do not translate inode
552 Select IBM Personal Computer Code Page 860 (Portugal)
553 Select IBM Personal Computer Code Page 863 (Canadian French)
554 Select IBM Personal Computer Code Page 865 (Nordic)
555 Select IBM Personal Computer Code Page 864 (Arabic)
556 Select IBM Personal Computer Code Page 862 (Hebrew)
557 Select IBM Personal Computer Code Page 855 (Cyrillic)
558 Indentifies Country (+: Greece, Turkey;  !: Others)
559 Generate List of Code Pages
560 Identifies the Font Card Installed in the Left Font Card Slot. The \
value is the Last Two Digits of the Part Number Shown on the Font Card.
561 Identifies the Font Card Installed in the Right Font Card Slot. The \
value is the Last Two Digits of the Part Number Shown on the Font Card.
562 Description of Font Card 00 (Universal)
563 32 K print buffer installed? (!: no;  +: yes)
564 Identifies the Active Code Page for the Installed Print Band \
(Examples: 437, 850, 851, 852, 853, 855, 857, 860, 862, 863, 864, 865, 869)
565 List of Stage 2 Translate Tables (code pages) Supported by the Font
566 Work Variable for Typeface Number
567 Work Variable for Type Style
568 Work Variable for Stroke Weight
569 Set Typeface/style/weight
570 Set Paper Tray Variable (1: upper 2: lower 3: envelope feeder)
$
$  Messages 571-584 Reserved for Arabic/Hebrew/Cyrillic-unique attributes
$
571 Select IBM Personal Computer Code Page 1046 (Arabic)
572 bidi filter (Hebrew/Arabic)
573 Description of Font Card 50 (Arabic 1)
574 Description of Font Card 51 (Arabic 2)
575 Description of Font Card 52 (Arabic 3)
576 Description of Font Card 11 (Hebrew 1)
577 Description of Font Card 19 (Hebrew 2)
578 Description of Font Card 40 (Hebrew 3)
580 Select IBM Personal Computer Code Page 855 (Cyrillic)
581 Select IBM Personal Computer Code Page 864 (Arabic)
582 Select IBM Personal Computer Code Page 862 (Hebrew)
583 Description of Font Card 17 (Cyrillic)
599 PARAMETERS FOR ASCII TERMINAL ATTACHED PRINTERS
600 Maximum number of characters to send to the ASCII terminal attached \
printer in one output operation.
601 Delay (microseconds) between outputs to ASCII terminal attached printer.
602 Priority with which printing will be done relative to terminal activity, \
the higher the number the greater the printer priority over the terminal.
603 Maximum Characters Per Second (CPS) rate at which characters are output \
to the ASCII terminal attached printer.  The rate chosen should be just \
below the average print speed.
604 Maximum number of characters the device driver will place in the \
output queue. 
605 The size, in characters, of the printer's input buffer. 
606 Select Quality (300; 600)
607 Paper Source Drawer (0: Top Tray; 1: Bottom Tray; 2: Envelope Feeder)
608 Command to set printer resolution 
609 Reset command
610 Command to set printer language
611 Select Quality (1: Quality; 2: Bold Black; 3: Draft; 4: High Quality; 5: Transparency)
612 PostScript command to set print quality
613 Identifies the hardware line discipline for the terminal configured on \
this queue. (rs: Native Port (s1, s2), 8-Port Adapter, or 16 -Port Adapter;\
 lion: 64-Port Adapter; cxma: 128-Port Adapter)
614 Send FORM FEED after each file?
615 EMPHASIZED print?
616 INITIALIZE printer?
617 DOUBLE-STRIKE print?
618 Page ORIENTATION
619 yes,no
620 portrait,landscape
621 HOSTNAME for "PRINTED AT:" on HEADER PAGE
622 DIAGNOSTIC LEVEL
623 PAGE NUMBER where printing should begin
624 FLAGS PROHIBITED for GL print files
625 -------------- Text Print Options -------------
626 \n----------- Job Processing Options ------------
627 \n----------- Text Formatting Options -----------
628 \n------------- Paper/Page Options --------------
629 \n--------- Header/Trailer Page Options ---------
630 \n------------- Messages/Diagnostics ------------
$ IMPORTANT: The above six messages must be EXACTLY 48 characters
$            in length (not counting the newline character at the
$            beginning of the message).  Use the '-' character (dash) to fill
$            out the message to 48 characters.  The messages
$            should be centered within the dashes.   The messages MUST
$            start with an newline character (\n) except for 625
631 (diag1) - do not print job; display main pipeline and pre-processing filter,(diag2) - do not print job; display all pipelines and filters,(display) - print job; display all pipelines and filters,(ignore) - print job; ignore stderr produced by filters,(normal) - print job; exit if filters produce stderr
632 Command to list pre-processing filter names
633 Command to validate pre-processing filter name
634 TOP MARGIN (lines)
635 BOTTOM MARGIN (lines)
636 manual,sheetfeed
637 Number of columns to INDENT
638 Input PAPER SOURCE
639 manual paper,tray 1 (upper),tray 2 (lower),envelope feeder,manual envelope 
640 Add LINE FEED or CARRIAGE RETURN
641 PAPER SIZE for manual paper feed
642 PAPER SIZE for tray 1 (upper)
643 PAPER SIZE for tray 2 (lower)
644 ENVELOPE SIZE for envelope feeder
645 ENVELOPE SIZE for manual envelope feed
646 ID of font card in top font card slot
647 ID of font card in bottom font card slot
648 ALIGN page if printer is idle
649 Number of FORM FEEDS when printer goes idle
650 Command to run to get current value of 'align' field
651 Command to run to get current value of 'feed' field
652 USERS to get intervention messages
653 WIDTH of printable area on header page
654 Default VERTICAL SPACING (lines per inch)
655 Input PAPER TRAY for header page
656 Input PAPER TRAY for trailer page
657 Command to list datastreams
658 Print file TYPE
659 Pre-processing FILTER NAME
660 cifplot filter
661 TeX (DVI) filter
662 FORTRAN filter
663 plot filter
664 pass-through filter
665 ditroff filter
666 'pr' filter
667 troff filter
668 Raster image filter
669 User defined filter 1
670 User defined filter 2
671 User defined filter 3
672 User defined filter 4
673 User defined filter 5
674 TYPESTYLE and PITCH
675 Attribute to build ring for pitch and typestyle
676 Attribute to validate pitch and typestyle
677 COLUMNS per page
678 LINES per page
679 Determine lines per page based on lines per inch
680 Command to display the string 'lines per inch'
681 Command to validate attribute 'sp'
682 FLAGS PROHIBITED for all print files
683 PAPER SIZE override for input paper source
684 FLAGS PROHIBITED for ASCII print files
685 Validate Attribute Value Strings During Digestion by "piodigest"? \
(!: no;  +: yes)
686 Level of colon files (1: old files converted to new format; \
2: new files)
687 Attachment Type
688 Printer Manufacturer
689 Attribute to validate paper size
690 lines per inch
691 Remote queue device 
692 Remote print server hostname
693 FLAGS PROHIBITED for PostScript print files
694 Generates a yes/no option ring
695 DUPLEX output
696 simplex,duplex,duplex with tumble
697 Type of INPUT PAPER HANDLING (backward compatibility purpose only)
698 manual,sheetfeed
699 drawer,manual,envelope
700 PITCH (characters per inch)
701 TYPE STYLE (name)
702 LINE DENSITY (lines per inch)
703 PAPER SIZE for primary drawer
704 PAPER SIZE for alternate drawer 
705 FONT DISKETTE installed in printer
706 Feature Code 7652; Part 1332634,Language Group 3 (Code Page 851 - Greece),\
Language Group 3 (Code Page 852 - Latin2),\
Language Group 3 (Code Page 855 - Cyrillic),\
Language Group 3 (Code Page 857 - Turkey),Language Group 4 \
(Code Page 862 - Hebrew),Language Group 2 (Code Page 864 - Arabic)
707 LINK input paper drawers
708 FORMATTING FLAGS for header page
709 FORMATTING FLAGS for trailer page
710 primary,alternate
711 fast draft,draft,letter,enhanced letter
712 Print QUALITY 
713 fastdraft,draft,near letter quality
714 CODE PAGE 850 available?
715 COUNTRY 
716 Arabic 1046,Greece; Turkey; Eastern Europe; Arabic; Hebrew,others
717 TITLE STRING for 'pr' filter
718 PAGE HEADINGS for ASCII print files?
719 TWO COLUMNS for ASCII print files?
720 GAUDY MODE for ASCII print files?
721 PRINT GARBAGE ASCII FILE anyway?
722 LIST CHARACTERS NOT IN FONT for ASCII files?
723 Calculates the tray operand for the setpapertray command
724 AUTOMATIC MODE SWITCHING for PostScript?
725 portrait,landscape right,portrait upside-down,landscape left
726 POINT SIZE for ASCII print files
727 DOUBLE SPACING?
728 no (single),yes (double)
729 Continuous DOUBLE WIDE?
730 Calculate value for paper source based on _O and _u.
734 ASCII Printer
735 Plotter
736 PostScript Printer
737 HP LaserJet II Emulation Printer
738 RESOLUTION of text converted to PostScript
739 manual paper feed,tray 1 (upper),tray 2 (lower),envelope feeder,manual envelope feed
740 manual paper feed,tray 1 (upper),tray 2 (lower),envelope feeder,manual envelope feed,100-sheet auxiliary feeder
741 quality,bold black,high speed,high quality,transparency
742 WIDTH for 'pr' filter for ASCII print files
743 upper tray,lower tray
744 DELAY between print jobs of differing type (sec)
746 CODE PAGE of printer
747 AUTOMATIC detection of print file TYPE to be done?
748 manual,drawer 1,drawer 2,envelope feeder,single envelope,pinwheel
749 primary paper casette,alternate paper casette
750 draft,quality,enhanced
751 portrait,landscape left,portrait upside-down,landscape right
752 FONT DISKETTE installed in printer
753 Feature Code 3155; Part 70X0759,Language Group 3 (Code Page 851 - Greece),Language Group 3 (Code Page 852 - Latin 2),Language Group 3 (Code Page 853 - Turkey),Language Group 3 (Code Page 855 - Cyrillic),Language Group 4 (Code Page 862 - Hebrew),Language Group 2 (Code Page 864 - Arabic)
754 fastfont,draft,NLQ,enhanced
755 draft,quality
756 Calculate value for print quality based on _q and _6
757 PAPER SIZE for upper tray
758 PAPER SIZE for lower tray
759 simplex (single-sided),duplex-- long-edge binding,duplex-- short-edge binding
760 Generate a ring list for paper source
761 manual paper feed,upper tray,lower tray,envelope feeder,manual envelope feed
762 Generate a ring list for paper types
763 Generate a ring list for envelope types
764 Generate the paper source escape sequence
765 PAPER SIZE for tray 1 
766 PAPER SIZE for tray 2
767 PAPER SIZE for envelope tray
768 lower tray,upper tray or no sheet feeder,envelope tray
770 Generate the correct 'name' parameter for the PostScript setpapertray command
771 manual paper,paper tray,envelope feeder,manual envelope
772 manual,tray 1,tray 2,envelope tray
773 upper tray,lower tray,envelope feeder
774 Set the vertical line spacing (VMI)
775 Set the vertical line spacing (lpi)
776 PAPER SIZE for paper tray
777 Is the PostScript Cartridge Plus INSTALLED?
778 12 cpi fastfont,10 cpi draft,NLQ,NLQ II
779 draft,letter
780 pinwheel,sheetfeed
781 ITALICS print?
782 Print COLOR
783 black,blue,red,magenta,green,cyan,yellow,black,brown
784 DP (fast draft),DP Text (draft),NLQ (near letter quality)
785 no initialization,full initialization,send PCL language switch command only
786 PRINTED LANGUAGE selected from the operator panel
787 PC Multilingual,Latin2,Cyrillic,Arabic,Greek,Turkish,Hebrew
788 draft,data-processing,near letter quality
789 DOUBLE-HIGH print?
790 Use WHOLE PAGE COORDINATE SYSTEM?
791 RESTORE printer?
792 CONDENSED print?
793 WRAP long lines?
794 DOUBLE-WIDE print?
795 CODE PAGE of print file
796 manual paper,lower paper cassette,multipurpose tray (paper),envelope feeder,manual envelope,multipurpose tray (envelope),optional 500-sheet cassette
797 Arabic 1046,others
798 Greece or Turkey,others
799 ACTIVE CODE PAGE for the installed print band
800 Error: PostScript virtual printer does not exist.\n
801 Error: The /usr/bin/enscript command was not found.\n
802 Error: The /usr/bin/ibm3812 command was not found.\n
803 Error: The /usr/bin/ibm3816 command was not found.\n
804 Error: The /usr/bin/hplj command was not found.\n
805 Error: ASCII virtual printer does not exist.\n
806 Remote Printer
807 PCL Printer
808 (Attribute referenced by formatter but not used)
809 Error: The /usr/bin/bprt command was not found.\n
810 ID of font card installed in left font card slot
811 ID of font card installed in right font card slot
812 no initialization,full initialization,send PostScript language switch command only
813 no initialization,full initialization,send ASCII language switch command only
814 Error: The /usr/bin/psplot command was not found.\n
815 Error: The /usr/ucb/fpr command was not found.\n
816 UNIDIRECTIONAL print?
817 manual paper,automatic sheet feeder,envelope feeder,manual envelope
818 verify code page of print file
819 Error: Cannot print separator page because no %s print queue exists for this printer.\n
820 Error: The %s command was not found.\n
821 Error: Cannot print %1$s file because no %2$s print queue exists for this printer.\n
822 TRAILER page LENGTH
823 POSTSCRIPT option INSTALLED?
824 PAPER SIZE for 100-sheet auxiliary feeder
825 32 K print buffer installed?
826 manual paper feed,paper tray,manual envelope feed
827 TYPE STYLE for ASCII print files
828 TITLE STRING for 'pr' filter for ASCII files
829 WRAP long lines for ASCII print files?
830 LINES per page for ASCII print files
831 Page ORIENTATION for ASCII print files
832 PAPER SIZE override for ASCII print files
833 PAPER SIZE for standard 250-sheet cassette
834 PAPER SIZE for multipurpose tray
835 PAPER SIZE for optional 500-sheet lower cassette
836 ENVELOPE SIZE for multipurpose tray
837 ENVELOPE SIZE for optional envelope feeder
838 standard 250-sheet cassette,multipurpose tray (paper),optional 500-sheet lower cassette,optional envelope feeder
839 standard 250-sheet cassette,multipurpose tray (paper),optional 500-sheet lower cassette,manual paper feed,multipurpose tray (envelope),optional envelope feeder,manual envelope feed
840 Command string for setting portrait orientation for envelopes
841 MODE in which to leave printer at end of job
842 Calculate page length based on paper size
843 PCL5 emulation option INSTALLED?
$ Note: do not translate PCL5
$
$ Messages Unique to Bull Printers
$
900 Type of Input Paper Handling (1: manual, 2: sheetfeed)
901 Font Style (0; 1; 32; 64; 128; 160)
902 Set Print Direction (0; 90; 180; 270)
903 Duplex Output (0: Simplex  1: Duplex Long Edge  2: Duplex Short Edge)
904 Select Stroke Weight (From 0:Medium to 7:Ultra Black)
905 Initialize the printer (for each print file)? (0: no;  1: yes)
906 Decimal Part of Pitch
907 Set Spacing for Font (0: Fixed; 1: Proportional)
908 Line Density (lines per inch) (1,2,3,4,6,8,12)
909 Select Primary or Secondary Font (0: Primary;  1: Secondary)
910 Rotate Page Output (0:Portrait;1:Landscape;2:Reverse Port;3:Reverse Land)
911 Select the IBM-US symbol set
912 Pipeline for Input Data Stream "s" (PostScript)
913 Pipeline for Input Data Stream "a" (extended ASCII)
914 Pipeline for Input Data Stream "c" (PCL)
915 Pipeline for Input Data Stream "g" (IBM Graphics)
916 Pipeline for Input Data Stream "p" (pass-through)
917 Pipeline for Input Data Stream "d" (Diablo 630)
918 Paper Source (1: Tray 1;  2: Tray 2;  3: Manual Envelope Feeder)
919 Pipeline for Input Data Stream "n" (troff (ditroff) intermediate output)
920 Attributes for Hierarchical Files
921 Select the IBM-Denmark/Norway symbol set
922 Select default Typeface : Courier
923 Select default Style : Upright
924 Select default value for Bold : 3
925 Set initial default values
926 Defines cursor position for scalable fonts, with different rotation
927 
928 
929 
930 not used
931 
932 
933 
934 Print Quality (1: draft)
935 Set TypeStyle
936 
937 Select double strike printing (! : no ; + : yes)
938 
939 Specify the pitch value
940 Octal value for NLQ
941 Octal value for OCR-B
942 Octal value for DP
943 Octal value for HS
944 Octal value for HSB
945 Octal value for HSC
946 Octal value for OCR-A
947 Set Typestyle and Pitch.
948 Type of Input Paper Handling (0: automatic, 1: manual)
949 Number of additional copies for each page (0:default)
950 Rotate Page Printer Output (0: portrait  1: landscape)
951 PostScript command to reset the Paper Source
$ Note: do not translate PostScript
952 PostScript Command to Set the Number of Copies
$ Note: do not translate PostScript
953 PostScript command to validate the Manual Feeder
$ Note: do not translate PostScript
954 PostScript command to reset the device
$ Note: do not translate PostScript
955 PostScript command to reset the Policies
$ Note: do not translate PostScript
956 Does PostScript Support Duplex Setting in statusdict? \
(!: no;  +: yes)
$
$
$
$
$set 2  Descriptors for system attributes @0, @1, ...
$
1 Path Name of File Being Printed
2 Number of Bytes To Be Passed Through For %x
3 Interface Type (0: Other;  1: parallel;  2: serial;  3: tty)
4 Base Directory for Static Files (normally, /usr/lib/lpd/pio)
5 Base Directory for Dynamic Files (normally, /var/spool/lpd/pio/@local)
6 Null String
7 Base Directory for Dynamic Files (normally, /var/spool/lpd/pio)
