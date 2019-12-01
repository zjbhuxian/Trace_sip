# $Id$
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=trace_sip.so
LIBS=-L/usr/lib64/mysql -lmysqlclient -levent -pthread -lz -lm -ldl -lssl -lcrypto -lhiredis
#LIBS=-L/usr/lib/x86_64-linux-gnu -lmysqlclient -pthread -lz -lm -ldl -lssl -lcrypto
include ../../Makefile.modules
