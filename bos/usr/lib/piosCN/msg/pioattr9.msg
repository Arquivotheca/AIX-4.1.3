$ @(#)01	1.1  src/bos/usr/lib/piosCN/msg/pioattr9.msg, ils-zh_CN, bos41J, 9507B 1/26/95 09:56:43
$
$   COMPONENT_NAME: ils-zh_CN
$
$   FUNCTIONS: None
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1995
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$       Descriptions Of Printer Backend Data Base Attributes
$
$set 1
$ 0 (description not available)
1 对输入数据流 "a" 而言，禁止命令行选项；不予处理：cmnorCDMPRT
2 对输入数据流 "k" 而言，禁止命令行选项；不予处理：cmnorCMDPRT
3 (not used)
4 (not used)
5 (not used)
6 (not used)
7 (not used)
8 (not used)
9 (not used)
10 (not used)
11 (not used)
12 (not used)
13 传回标准错误吗？ 0: no; 1: yes, & 管道；2: yes & 值，管道
14 传回标准错误吗？ 0: no; 1: yes, & 管道； 2: yes, & 值，管道
15 Reserved For Spooler (burst pages)
16 Reserved For Spooler (mail messages instead of displaying them)
17 Reserved for Spooler (mail message instead of displaying them)
18 将已打印的输出传给所设定的用户，而非传给提出者
19 (not used) Double-High Print (!: no; +: yes)
20 超高速打印 (!: no; +: yes)
21 超高速打印。(!: no;  +: yes)
22 (not used) Font file name
23 使用定义的字库文件名称
24 (reserved for future use)
25 要用於置换迸出页(Burst Page)上的主机名称之名称
26 要用於置换迸出页(Burst Page)上的主机名称之名称
27 (not used) Font ID
28 (not used) Font ID (overrides pitch and type style)
29 要在打印工作结束时，复位打印机吗？
30 要在打印工作结束时，复位打印机吗？(!: no;  +: yes)
31 压缩式打印 (!: no;  +: yes)
32 压缩式打印 (!: no; +: yes)
33 折返较长的行 (!: no;  +: yes)
34 折返较长的行 (!: no; +: yes)
35 Reserved For Spooler (message to operator from file)
36 Reserved For Spooler (message to operator from file)
37 Reserved For Spooler (number of copies)
38 (not used) Type of Input Paper Handling (1: manual, 2: pinwheel, 3: sheetfeed)
39 (not used) Type of Input Paper Handling (manual, pinwheel, sheetfeed)
40 输入纸张处理的类型（1: 手动，2: 连续纸，3: 单张纸）
41 Reserved For Spooler (queue name & opt. virtual printer (queue dev.) name)
42 Reserved For Spooler (queue name & opt. virtual printer (queue dev.) name)
43 (reserved for future use)
44 Reserved For Spooler (print job priority)
45 (not used) High Speed Print (!: no; +: yes)
46 (not used) High speed printing
47 高速打印 (!: no; +: yes)
48 Reserved For Spooler (title for header page & queue status)
49 (not used) Unidirectional Printing (!: no; +: yes)
50 (not used) Unidirectional printing
51 单向打印 (!: no; +: yes)
52 (not used) Vertical printing
53 垂直打印 (!: no; +: yes)
54 倍宽打印 (!: no;  +: yes)
55 倍宽打印 (!; no; +: yes)
56 (not used) Code Page Name For Print Data Stream (file with same name in dir."d1")
57 打印数据流的字码页名称（在目录 "d1" 中，文件具有相同的名称）
58 (not used) Duplex Output
59 在复制之间并在工作结束时，发出单张纸送纸 (!: no;  +: yes)
60 在复制之间并在工作结束时，发出单张纸送纸 (!: no; +: yes)
61 命令集合
62 控制码 (ASCII)
63 目录
64 跳出顺序
65 过滤程序选项的命令字串（2 个字符，lst="f"，2nd=flag）
66 过滤程序选项的命令字串（2 个字符，lst="f"，2nd=flag）
67 可能为命令行上的选项所改写的那些值
68 输入数据流的管道（2 个字符，1st="i"，2nd=数据流名称）
69 输入数据流的管道（2 个字符，1st="i"，2nd=数据流名称）
70 杂项
71 输入数据流禁止选项（2 个字符，1st="I"，2nd=数据流名称）
72 输入数据流禁止选项（2 个字符，1st="I"，2nd=数据流名称）
73 系统管理员有意使用的其它值
74 转换表
75 工作变数
76 0: 一般打印；1: 传回选项值 & 管道，但不打印
77 0: 一般打印；1: 传回选项值 & 管道，但不打印
78 底端间距（空白行数）；有时亦称跳过骑缝线
79 底端间距（空白行数）；有时亦称跳过骑缝线
80 Reserved For Spooler (copy file and print from copy)
81 输入数据流类型
82 加重打印
83 加重打印 (!: no;  +: yes)
84 (not used) Single-Character BSD-type Filter Name (e.g., "p" for "pr" filter)
85 单一字符 BSD-键入过滤程序名称（例如，"p" 代表 "pr" 过滤程序）
86 要在该处开始打印的页码
87 (not used) Title Used For "pr" Filter When "-f p" Is Specified (not used)
88 当设定了 "-f p" 时，用於 "pr" 过滤程序的标题
89 缩进值（字符）
90 缩进值（字符数）
91 初始化打印机（对每一个打印文件而言）吗？(!:no; +:yes)
92 初始化打印机（对每一个打印文件而言）吗？(!:no; +:yes)
93 (not used) Print Color
94 页面长度（行数； 0 表示不处理页面长度）
95 Reserved For Spooler (msg to operator using cmd line text)
96 Reserved For Spooler (notify submitter (if -D, that user, too) when done)
97 Reserved For Spooler (protects backend flags passed to "enq" cmd)
98 间距（每英寸字符数）
99 (not used) Print Quality (1: draft; 2: letter)
100 打印质量（1: 草稿；2: 信函）
101 Reserved For Spooler (remove print file when done)
102 Reserved for Spooler (remove print file when done)
103 (not used) Type Style (name)
104 类型的型式 (m10、m12、courier、elite、gothic)
105 顶端间距（空白行数）
106 (not used) Paper Source Drawer
107 (not used) Paper Source Drawer (1: Upper  2: Lower)
108 托纸盘（1: 上层 2: 下层）
109 行密度（每英寸行数）
110 行密度（每英寸行数）例如：6、8
111 页面宽度（字符数）
112 0: CR->CR, LF->LF, VT->VT;  1: CR->CRLF  2: LF->CRLF, VT->CRVT;
113 (not used) Double-Strike Print (!: no;  +: yes)
114 (not used) Double-Strike Print (!: no; +: yes)
115 重复打印 (!: no;  +: yes)
116 重复打印 (!: no;  +: yes)
117 (not used) Rotate Page Printer Output
118 (not used) Rotate Page Printer Output (# quarter turns clockwise: 0-3)
119 旋转打印
120 倒退一个字符的 ASCII 控制码
121 返回左边缘的 ASCII 控制码（CR 不用 LF）
122 取消倍宽打印的 ASCII 控制码
123 使打印机失效（队列控制 3 ）的 ASCII 控制码
124 使打印机生效（队列控制 1 ）的 ASCII 控制码
125 使纸张上升到次页顶端 (FF) 的 ASCII 控制码
126 (not used; handled by formatter) Horizontal Tab
127 移入（亦即，启动压缩式打印）的 ASCII 控制码
128 使纸张上升一行（LF 不用 CR）的 ASCII 控制码
129 空字符
130 移出（亦即，设定行为倍宽型式）的 ASCII 控制码
131 铃响一次的 ASCII 控制码
132 取消打印工作（取消）的 ASCII 控制码
133 每英寸选取 10 个字符的 ASCII 控制码；取消连续，12 CPI
134 (not used; handled by formatter) Vertical Tab
135 选取主要 (ROM) 字库
136 选取替代（已下载的）字库
137 无法使用斜体字
138 设定启动／关闭压缩状态
139 以 Hres 为单位空出水平空格（用於缩进，LF 不用 CR）的命令
140 以 Hres 为单位空出水平空格（用於缩进，LF 不用 CR）
141 初始化打印机的命令
143 传送换行回车与跳行控制的命令
144 传送换行回车与跳行控制
145 在 Pri. 状态 & Alt. （安全性标签）状态之间做转换时的命令
146 设定间距
147 设定打印质量（0: 高速字库；1: 草稿；2: NLQ；3: 加强型）
148 在工作结束时复位打印机
149 初始化元比例
150 设定启动／关闭随字体宽度调整空格
151 设定垂直空格
152 (not used) Directory Containing Stage 1 Translate Tables (data stream to intermed.)
153 目录含有阶段 1 转换表（数据流至中间）
154 (not used) Directory Containing Stage 2 Translate Tables (intermediate to printer)
155 目录含有阶段 2 转换表（中间至打印机）
156 目录含有表头 & 表尾文件
157 目录含有表头 & 表尾本文文件
158 目录含有摘录的资料库文件
159 目录含有杂项模组
160 目录含有可载入的Format器常式
161 目录含有欲下载的字库
162 暂存文件的目录
163 (not used) Set Paper Source Drawer
164 将行距设定为每英寸 8 行
165 设定托纸盘
166 (not used) Text Rotation Command
167 将行距设定为每行 7/72 英寸
168 文本旋转命令
169 (not used) Set Number of Copy Command
170 设定复制份数命令
171 开启以 SET_LS_CMD 所设定的行距
172 (not used) Set Logical Page Command
173 设定图形行距
174 设定逻辑页命令
175 (not used) Set Line Type Command
176 (not used) Set Top of Form
177 设定行类型命令
178 (not used) Set Line Width Command
179 （仅用於Passthru初始化）启动／关闭换行回车时自动跳行
180 设定行宽命令
181 (not used) Draw Box Command
182 （仅用於初始化／复位）选取字集 2
183 拉动框命令
184 (not used) Overlay Command
185 (not used) Select Character Set 1
186 覆盖命令
187 (not used) Segment Command
188 打印全部字符
189 区段命令
190 (not used) Page Map Primitive Command
191 (not used; would conflict with formatter processing) Set Code Page
192 页对映原始命令
193 设定本文行距的命令
194 设定 DBCS 数据流状态（仅用於初始化／复位）
195 （仅用於初始化／复位）设定垂直制表符停驻
196 初始化左／右间距（仅用於初始化／复位）
197 （仅用於初始化／复位）设定页面长度
198 初始化打印机（仅用於初始化／复位）
199 (not used) Set/Reset Uni-directional Print Mode
200 （仅用於初始化／复位）设定水平制表符
201 设定／重设单向打印状态
202 (not used) Set/Reset High Speed Print Mode
203 设定／重设高速打印状态
204 启动／关闭加重打印
205 设定／重设压缩式打印状态
206 (not used) Turn On/Off Double-Strike Printing
207 设定／重设倍宽打印状态
208 启动／关闭重复式打印
209 设定／重设垂直打印状态
210 选取间距、类型的型式及质量
211 设定／重设加重打印状态
212 (not used) Set/Reset Double Strike Print Mode
213 使纸张以可变总数上升
214 设定／重设重复打印状态
215 以横向 60 点／英寸 & 直向 72 点／英寸的方式打印图形
216 设定／重设底线
217 以横向 120 点／英寸 & 直向 72 点／英寸的方式（低速）打印图形
218 设定／重设重复
219 （仅用於初始化／复位）初始化水平制表符
220 (not used; handled by formatter) Set Automatic Perforation Skip
221 （仅用於初始化／复位）初始化垂直制表符
222 （仅用於初始化／复位）取消自动跳过骑缝线
223 设定每英寸字符数
224 设定每英寸行数
225 启动／关闭随字体宽度调整空格
226 设定以 1/6 为单位的页面长度
227 (not used; handled by formatter) Reset Hor. Tabs to Every 8 Pos. & Clear Vert. Tabs
228 变更字库型式
229 变更字符比例
230 启动／关闭下标的打印
231 单张报表纸插入
232 启动／关闭上标的打印
233 (not used) Change From/To Left-to-Right To/From Two-Direction Printing
234 变更「自／至」由左至右「至／自」两个方向的打印
235 单张报表纸跳出
236 启动／关闭上／下标状态
237 启动／关闭连续倍宽打印
238 设定左及右间距
239 以横向 120 点／英寸 & 直向 72 点／英寸的方式（一般速度）打印图形
240 以横向 240 点／英寸 & 直向 72 点／英寸的方式打印图形
241 （仅用於初始化／复位）关闭换行回车时自动跳行
242 设定撞击两次式打印
243 图像传输及放大
244 自全部字符图表中打印单一字符
245 取消撞击两次式打印
246 取消垂直打印
247 设定垂直打印
248 向右移 n/120 英寸
249 向前垂直跳出
250 向左移 n/120 英寸
251 所设定点位置的换行回车
252 设定倍宽打印
253 取消倍宽打印
254 设定倍高打印
255 取消倍高打印
256 (not used) Change Form Handling
257 变更表格处理
258 设定高密度打印方式
259 设定高速打印方式
260 设定单向打印方式
261 设定双向打印方式
262 设定重打方式
263 取消重打方式
264 设定倍宽及倍高方式
265 图象输出(ascii字符)
266 图象输出(gb2312)
267 设定 24/180 英寸换行量
268 初始化打印机
269 设定 1/6 英寸换行量
270 执行 24/180 英寸换行量
271 设定 45/180 英寸换行量
272 设定高速打印
273 取消高速打印
274 反转跳行
275 垂直行跳出
276 (not used) Write Image Control Command
277 写入图像控制命令
278 (not used) Write Image Command
279 写入图像命令
280 (not used) Load Font Pattern Command
281 载入字库型样命令
282 (not used) Load Font Control Command
283 (not used) Printer initialization - this will override formatter initialization - DO NOT USE !!!
284 载入字库控制命令
285 (not used) Set Font Local Command
286 设定字库区域命令
287 (not used) Delete Font Local Command
288 删除区域字库命令
289 (not used) Character Rotation Command
290 替代图形状态：在所设定的状态下打印图形
291 字符反转命令
292 (not used) Character Baseline Offset Command
293 字符基准线位移命令
294 设定 SBCS 数据流状态
295 (not used) Command String for the "c" Filter.
296 "c" 过滤程序的命令字串。
297 (not used) Command String for the "d" Filter.
298 "d" 过滤程序的命令字串。
299 (not used) Command String for the "f" Filter.
300 "f" 过滤程序的命令字串。
301 (not used) Command String for the "g" Filter.
302 "g" 过滤程序的命令字串。
303 (not used) Command String for the "l" Filter.
304 "l" 过滤程序的命令字串。
305 (not used) Command String for the "n" Filter.
306 "n" 过滤程序的命令字串。
307 (not used) Command String for the "p" Filter.
308 "p" 过滤程序的命令字串。
309 (not used) Command String for the "t" Filter.
310 "t" 过滤程序的命令字串。
311 (not used) Command String for the "v" Filter.
312 "v" 过滤程序的命令字串。
313 输入数据流 "a" 的管道（扩充型 ASCII）
314 输入数据流 "k" 的管道（日文汉字）
315 输入数据流 "t" 的管道（Passthru）
316 (not used) Path Name of Stage 2 Translate Table For Resident Font
317 (not used) Path Name of Stage 2 Translate Table For Download Font
318 压缩式状态类型        1;4207m2  2;4201m3
319 (not used) Path Name of Font File To Be Downloaded (must include download commands)
320 欲下载的字库文件之路径名称（必须加入下载命令）
321 图形单位处理选项       1;4207m2  2;4201m3
322 分割水平英寸的单位数目
323 Esc I 处理选项        1;4207m2  2;4201m3
324 打印机型号        4201      4207
325 顶端间距（其余为底端间距）的跳过骑缝线值之百分比
326 (not used) Character To Be Used When Real One Can't Be Printed
327 省缺字符
328 分割垂直英寸的单位数目
329 分割垂直英寸的单位数目
330 (not used) Name of PostScript Data Base File; If Null, Search In "dd" Dir.
331 (not used) Name of Postscript Data Base File; If Null, Search In "dd" Dir.
332 当工作被取消时，要送到打印机 "mz" 次的字串
333 输出数据流类型（例如： ascii）；由 "piodigest" 来初始化　
334 输出数据流类型（例如：日文汉字）；由 "piodigest" 来初始化　
335 Formatter的路径名称（在单独执行时使用）
336 Formatter的路径名称（在单独执行时使用）
337 PostScript的输入数据流名称（一个字符）
338 PostScript的输入数据流名称（一个字符）
339 （摘录的）资料库之文件名称；由 "piodigest" 来初始化 (mt.md.mn.mq:mv)
340 队列名称（例如：lp0）；由 "piodigest" 来初始化
341 队列名称（例如：lp0）；由 "piodigest" 来初始化
342 用於呼叫Formatter Driver I/F 程序（管道尾端）的命令字串
343 用於识别 PostScript （替代 ASCII）数据流的字串
344 等待序列名称；由 "piodigest" 来初始化
345 (not used) Path Name of "read printer" routine ('load'ed by Dev. Dri. I/F pgm)
346 打印机类型（例如：ar2463）；由 "piodigest" 来初始化　
347 打印机类型（例如：lq1600k）；由 "piodigest" 来初始化　
348 打印机类型（例如：laserjet2）；由 "piodigest" 来初始化　
349 打印机类型（例如：5587）；由 "piodigest" 来初始化　
350 打印机类型（例如：5575）；由 "piodigest" 来初始化　
351 打印机类型（例如：5587）；由 "piodigest" 来初始化　
352 欲打印多份副本时： 0: 返回管道；1: 重新打印所储存的副本
353 欲打印多份副本时： 0: 返回管道；1: 重新打印所储存的副本
354 虚拟打印机（亦即，等待序列队列）名称；由 "piodigest" 初始化　
355 虚拟打印机（亦即，等待序列队列）名称；由 "piodigest" 初始化　
356 可以被定义的水平制表符之最大数目
357 可以被定义的垂直制表符之最大数目
358 当工作被取消时，将字串 "mc" 传到打印机的次数
359 (not used) Code page number
360 表头页的管道
361 接收到需要人为介入讯息的用户（内设值：工作提出者）
362 对全部输入数据流而言，禁止命令行选项；不予处理：cmnorCDMPRT
363 对全部输入数据流而言，禁止命令行选项；不予处理：cmnorCMDPRT
364 表尾页的管道
365 表头页上属性值区域的宽度（0 表示不处理宽度）
366 阶段 2 转换表的路径名称
367 第一个替代阶段 2 转换表的路径名称
368 下一个替代转换阶段 2 转换表的路径名称
369 同上
370 同上
371 同上
372 同上
373 同上
374 同上
375 同上
376 由Format程序所设定的整数，可供各种命令使用
377 直向打印
378 右方横向打印
379 由下至上直向打印
380 左方横向打印
381 打印全部命令的其余字符
382 其余图像资料
383 垂直减量的工作变数（以 Vres 为单位）
384 从前一行储存水平位置的工作变数
385 由 ESC J & ESC 3 所使用的垂直索引增量
386 每个字符水平增量的工作变数（以 Hres 为单位）
387 图像传输状态（3: 3个位元组的传输， 2: 2个位元组的传输）
388 此打印机没有斜体字可供使用
389 图像资料宽度（由 FS 命令所使用）
390 以字符为单位的页面长度，使用来自资料库的长度（在管路线路中使用）
391 以 Hres 为单位的左间距
392 由 ESC A 所使用的垂直索引增量
393 垂直增量的工作变数（以 Vres 为单位）
394 以字符为单位的页面宽度，使用来自资料库的宽度（在管道中使用）
395 工作整数
396 目前在下标状态下吗？ (!: no;  +: yes)
397 目前在下标状态下吗？ (!:no; +:yes)
398 重复命令的字符码（1 位元组码： 0x0000XX00； 2 位元组码： 0x0000XXXX）
399 目前在"行"倍宽状态下吗？ (!: no;  +: yes)
400 向前垂直跳出的名称
401 目前在上标状态下吗？ (!: no;  +: yes)
402 目前在上标状态下吗？ (!:no; +:yes)
403 现行换页命令的名称
404 重复命令的字符选项（3: 半形； 4: 全形； 1: 取消）
405 由 SET_GRA_LS_CMD 所使用的图形行距
406 现行水平制表符的设定（由空字符来终止）
407 现行垂直增量命令的名称
408 每次跳行时，向下空格的行数
409 现行垂直空格状态 (0: ESC 0; 1: ESC 1; 2: ESC 2; 3: ESC 3)
410 重复命令的状态选项 (B'0000 00nm)
411 目前在连续顶线状态下吗？ (!: no;  +: yes)
412 以Passthru代替Format（亦即，数据流未被修正）吗？
413 以Passthru代替Format（亦即，数据流未被修正）吗？
414 目前在单向打印状态下吗？ (!: no;  +: yes)
415 目前在随字体宽度调整空格状态下吗？ (!: no;  +: yes)
416 行距：（值/Tunits）每行英寸数；由 SET_LS_CMD 所使用
417 向後垂直跳出的名称
418 目前在连续底线状态下吗？ （B'0000 00nm; n: 1:on 0:off; m: 1: 空白）
419 目前在连续底线状态下吗？ (!: no;  +: yes)
420 现行垂直制表符的设定（由空字符来终止）
421 页面宽度的工作变数（以 Hres 为单位）
422 偏离的下标命令之名称
423 现行垂直减量命令的名称
424 偏离的上标命令之名称
