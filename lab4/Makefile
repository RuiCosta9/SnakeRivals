PROG=lab4

SRCS = lab4.c timer.c utils.c

CFLAGS += -pedantic
CPPFLAGS += -I../lab2

.PATH: ../lab2

LIB_NAME = libmouse.a
LIB_SRCS = mouse.c
LIB_OBJS = ${LIB_SRCS:.c=.o}

DPADD += ${LIB_NAME} ${LIBLCF}
LDADD += -L. -lmouse -llcf

CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib

lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

.include <minix.lcom.mk>