plugin_LTLIBRARIES = libgstlrc.la

libgstlrc_la_SOURCES = gstlrc.c gstlrcdemux.c gstlrcsink.c

libgstlrc_la_CFLAGS_general = $(GST_CFLAGS) $(GST_BASE_CFLAGS) -I/vobs/linuxjava/platform/api/include

libgstlrc_la_LIBADD_general = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstbase-$(GST_MAJORMINOR) 

libgstlrc_la_LIBADD = $(libgstlrc_la_LIBADD_general)

libgstlrc_la_CFLAGS = $(libgstlrc_la_CFLAGS_general)

libgstlrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstlrcdemux.h gstlrcsink.h




