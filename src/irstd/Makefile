#
# Makefile for irlist table creation tools
#

CC     = gcc
CFLAGS = -g


convert_tab: convert_tab.c
	$(CC) -o convert_tab convert_tab.c $(CFLAGS)


clean:
	rm -f convert_tab irlist.h

veryclean:
	rm -rf convert_tab irlist.h ./html
	
install: convert_tab
	convert_tab catalogs/*
	rm -f ../include/irlist.h
	mv irlist.h ../include

html:
	rm -rf ./html
	convert_tab -w catalogs/*
