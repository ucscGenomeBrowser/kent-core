kentSrc = ../..
include ../../inc/common.mk

MYLIBDIR = ../../lib/$(MACHTYPE)
MYLIBS =  $(MYLIBDIR)/jkhgap.a $(MYLIBDIR)/jkweb.a

O = dataSim.o

dataSim: $O $(MYLIBS)
	${CC} ${COPT} ${CFLAGS} -o ../bin/dataSim $O $(MYLIBS) $L
#	${STRIP} ../bin/dataSim${EXE}

clean::
	rm -f ${O}
