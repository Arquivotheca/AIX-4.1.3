CFLAGS = -g
LIB = -l iconv

all: cmap_conv cmap_merge cmap_split cmDB

cmap_conv: cmap_conv.o
	$(CC) -o cmap_conv cmap_conv.o ${LIB}

cmap_conv.o: cmap_conv.c cmap_conv.h

cmap_merge: cmap_merge.o
	${CC} -o cmap_merge cmap_merge.o 

cmap_merge: cmap_merge.c cmap_merge.h

cmap_split: cmap_split.o 
	${CC} -o cmap_split cmap_split.o

cmap_split.o: cmap_split.c cmap_split.h


cmDB:	cmDB.o
	$(CC) -o cmDB cmDB.o ${LIB}

cmDB.o:	cmDB.c cmDB.h

