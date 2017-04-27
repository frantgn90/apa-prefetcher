#
# Makefile
# Juan Francisco Martínez, 2017-04-27 23:23
#

CFLAGS  = -g
INCLUDE = -I../inc/ -I.
LIBS    = ../lib/dpc2sim.a

all: *.c
	@echo "Compiling SPP Prefetcher..."
	gcc $(CFLAGS) -c pf.c pt.c st.c ghr.c $(INCLUDE)
	gcc $(CFLAGS) -o dpc2sim spp_prefetcher.c *.o $(LIBS) $(INCLUDE)
	@echo "Done!"

clean:
	rm *.o
	rm dpc2sim
