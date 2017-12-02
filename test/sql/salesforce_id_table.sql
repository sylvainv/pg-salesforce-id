CREATE EXTENSION IF NOT EXISTS salesforce_id;
CREATE EXTENSION IF NOT EXISTS pgtap;

DROP TABLE IF EXISTS salesforce_id_test;
CREATE TABLE salesforce_id_test(id salesforce_id);
INSERT INTO salesforce_id_test (id)
  VALUES
    ('001p000000OoULiAAN'),
    ('0012800000CXmurAAD'),
    ('zzzzzzzzzzzzzzzAAA'),
    ('001p000000ObEcqAAF'),
    ('AAAAAJJJJJPPPPP555')
;

SELECT plan(2);
SELECT results_eq(
    'SELECT id FROM salesforce_id_test',
    $$ VALUES
      ('001p000000OoULiAAN'::salesforce_id),
      ('0012800000CXmurAAD'::salesforce_id),
      ('zzzzzzzzzzzzzzzAAA'::salesforce_id),
      ('001p000000ObEcqAAF'::salesforce_id),
      ('AAAAAJJJJJPPPPP555'::salesforce_id)
    $$,
    'Values have correct output on select'
);

SELECT results_eq(
    'SELECT * FROM salesforce_id_test ORDER BY id',
    $$ VALUES
      ('0012800000CXmurAAD'::salesforce_id),
      ('001p000000ObEcqAAF'::salesforce_id),
      ('001p000000OoULiAAN'::salesforce_id),
      ('AAAAAJJJJJPPPPP555'::salesforce_id),
      ('zzzzzzzzzzzzzzzAAA'::salesforce_id)
    $$,
    'Values have correct output on ordered select'
);

SELECT results_eq(
    'SELECT * FROM salesforce_id_test ORDER BY id DESC',
    $$ VALUES
      ('zzzzzzzzzzzzzzzAAA'::salesforce_id),
      ('AAAAAJJJJJPPPPP555'::salesforce_id),
      ('001p000000OoULiAAN'::salesforce_id),
      ('001p000000ObEcqAAF'::salesforce_id),
      ('0012800000CXmurAAD'::salesforce_id)
    $$,
    'Values have correct output on ordered select'
);

CREATE UNIQUE INDEX ON salesforce_id_test (id);

SELECT throws_ok(
    $$ INSERT INTO salesforce_id_test (id) VALUES ('001p000000OoULi'); $$,
    '23505',
    'duplicate key value violates unique constraint "salesforce_id_test_id_idx"',
    'We should get an error when attempting to insert a unique id'
);

EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where id = 'AAAAAJJJJJPPPPP555';
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where id <>  'AAAAAJJJJJPPPPP555';
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where 'AAAAAJJJJJPPPPP555'::salesforce_id > id;
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where 'AAAAAJJJJJPPPPP555'::salesforce_id < id;

ANALYZE;
SET enable_seqscan TO off;
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where id = 'AAAAAJJJJJPPPPP555';
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where id <>  'AAAAAJJJJJPPPPP555';
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where 'AAAAAJJJJJPPPPP555'::salesforce_id > id;
EXPLAIN (COSTS OFF) SELECT * FROM salesforce_id_test where 'AAAAAJJJJJPPPPP555'::salesforce_id < id;
