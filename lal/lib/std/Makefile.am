include $(top_srcdir)/gnuscripts/lalsuite_header_links.am

pkginclude_HEADERS = \
	LALAtomicDatatypes.h \
	LALConstants.h \
	LALDatatypes.h \
	LALDebugLevel.h \
	LALError.h \
	LALGSL.h \
	LALMalloc.h \
	LALSIMD.h \
	LALStatusMacros.h \
	LALStddef.h \
	LALStdio.h \
	LALStdlib.h \
	LALString.h \
	LALVCSInfoType.h \
	StringInput.h \
	XLALError.h \
	XLALGSL.h \
	$(END_OF_LIST)

noinst_LTLIBRARIES = libstd.la

libstd_la_SOURCES = \
	LALDebugLevel.c \
	LALError.c \
	LALGSL.c \
	LALMalloc.c \
	LALSIMD.c \
	LALString.c \
	LALVCSInfoType.c \
	StringConvert.c \
	StringToken.c \
	XLALError.c \
	$(END_OF_LIST)

man_MANS = \
	LAL_DEBUG_LEVEL.7 \
	$(END_OF_LIST)

EXTRA_DIST = \
	LALConfig.dox \
	LALDatatypes.dox \
	LALError.dox \
	LALMalloc.dox \
	LALStatusMacros.dox \
	LAL_DEBUG_LEVEL.dox \
	$(MANS) \
	$(END_OF_LIST)
