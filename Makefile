MODULES = salesforce_id
EXTENSION = salesforce_id
DATA = salesforce_id--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)