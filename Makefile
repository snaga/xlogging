# contrib/logging/Makefile

MODULE_big	= logging
OBJS		= logging.o

EXTENSION = logging
DATA = logging--0.1.sql logging--unpackaged--0.1.sql

#REGRESS = pgstattuple

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/logging
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
