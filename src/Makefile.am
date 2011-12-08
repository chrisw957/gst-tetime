# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgsttetime.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgsttetime_la_SOURCES = gsttetime.c gsttetime.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgsttetime_la_CFLAGS = $(GST_CFLAGS)
libgsttetime_la_LIBADD = $(GST_LIBS)
libgsttetime_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgsttetime_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gsttetime.h
