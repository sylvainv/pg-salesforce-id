CREATE EXTENSION IF NOT EXISTS salesforce_id;
CREATE EXTENSION IF NOT EXISTS pgtap;

DROP TABLE IF EXISTS salesforce_id_test_sfid;
CREATE TABLE salesforce_id_test_sfid(id salesforce_id);

DROP TABLE IF EXISTS salesforce_id_test_text;
CREATE TABLE salesforce_id_test_text(id text COLLATE "C");

SELECT plan(2);

INSERT INTO salesforce_id_test_sfid (id)
  VALUES
    ('001p000000OoULiAAN'),
    ('0012800000CXmurAAD'),
    ('zzzzzzzzzzzzzzzAAA'),
    ('001p000000ObEcqAAF'),
    ('AAAAAJJJJJPPPPP555')
;

INSERT INTO salesforce_id_test_text (id)
  VALUES
    ('001p000000OoULiAAN'),
    ('0012800000CXmurAAD'),
    ('zzzzzzzzzzzzzzzAAA'),
    ('001p000000ObEcqAAF'),
    ('AAAAAJJJJJPPPPP555')
;

SELECT is(pg_table_size('salesforce_id_test_sfid') < pg_table_size('salesforce_id_test_text'), TRUE, 'We actually save space with our Salesforce Id type');

INSERT INTO salesforce_id_test_sfid (id) SELECT gen_random_salesforce_id() from generate_series(1, 100000);
INSERT INTO salesforce_id_test_text (id) SELECT gen_random_salesforce_id()::text from generate_series(1, 100000);

SELECT is(pg_table_size('salesforce_id_test_sfid') < pg_table_size('salesforce_id_test_text'), TRUE, 'We actually save space with our Salesforce Id type');
