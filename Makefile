MODULES = salesforce_id
EXTENSION = salesforce_id
DATA          = $(wildcard *--*.sql) 						# script files to install
TESTS         = $(wildcard test/sql/*.sql)      # use test/sql/*.sql as testfiles

# find the sql and expected directories under test
# load plpgsql into test db
# load base36 extension into test db
# dbname
REGRESS_OPTS  = --inputdir=test         \
                --load-extension=salesforce_id \
				--load-extension=pgtap
REGRESS       = $(patsubst test/sql/%.sql,%,$(TESTS))
OBJS 					= $(patsubst %.c,%.o,$(wildcard src/*.c)) # object files
MODULE_big    = $(EXTENSION)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)