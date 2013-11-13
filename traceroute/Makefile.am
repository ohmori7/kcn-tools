bin_PROGRAMS = traceroute
traceroute_SOURCES = as.c traceroute.c traceroute_rumpops.c	\
	ifaddrlist.c traceroute_hostops.c version.c
traceroute_CFLAGS = $(KCN_CFLAGS) $(CURL_CFLAGS) $(JANSSON_CFLAGS)
traceroute_LDADD = $(KCN_LIBS) $(CURL_LIBS) $(JANSSON_LIBS)
