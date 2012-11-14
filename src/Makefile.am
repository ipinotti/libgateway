AM_CFLAGS=-D_REENTRANT -D_GNU_SOURCE -fPIC
SUBDIRS = libnetlink

lib_LTLIBRARIES = libamg.la
libamg_la_SOURCES = if.c process.c str.c dahdi.c sip.c extensions.c comcerto.c dsp.c
	
libamg_la_LIBADD = $(top_builddir)/src/libnetlink/libnetlink.la

libamg_ladir = $(includedir)/libamg
libamg_la_HEADERS =	if.h process.h str.h dahdi.h sip.h extensions.h comcerto.h dsp.h log.h
