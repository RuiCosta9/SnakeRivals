PROG=lab3

SRCS = lab3.c

CFLAGS += -pedantic -DLAB3
CPPFLAGS += -I../lab2

LIB_NAME = libkbd.a
LIB_SRCS = kbc.c
LIB_OBJS = ${LIB_SRCS:.c=.o}

DPADD += ${LIB_NAME} ../lab2/libtimer.a ${LIBLCF}
LDADD += -L. -lkbd -L../lab2 -ltimer -llcf

CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib

lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

.include <minix.lcom.mk>