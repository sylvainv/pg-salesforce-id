CREATE EXTENSION IF NOT EXISTS salesforce_id;
CREATE EXTENSION IF NOT EXISTS pgtap;

SELECT '0012800000CXn0kAAD'::salesforce_id;
SELECT '0012800000CXmbXAAT'::salesforce_id;
SELECT '001p000000OoULiAAN'::salesforce_id;
SELECT '000000000000000AAA'::salesforce_id;
SELECT 'AAAAAJJJJJPPPPP555'::salesforce_id;
SELECT 'zzzzzzzzzzzzzzzAAA'::salesforce_id;
SELECT 'zzzzzzzzzzzzzzz'::salesforce_id;
SELECT '0012800000CXmbX'::salesforce_id;

SELECT plan(3);
SELECT throws_ok(
    $$ SELECT 'zzzzzzzzzzzzzzzAA'::salesforce_id; $$,
    '22026',
    'Invalid salesforce_id, should be 18 or 15 characters long, got 17 instead',
    'We should get an error when length is not correct'
);

SELECT throws_ok(
    $$ SELECT 'zzzzzzzzzzzzzz'::salesforce_id; $$,
    '22026',
    'Invalid salesforce_id, should be 18 or 15 characters long, got 14 instead',
    'We should get an error when length is not correct'
);

SELECT throws_ok(
    $$ SELECT 'zzzzzzzzzzzzzzzAAB'::salesforce_id; $$,
    '22P02',
    'Salesforce id suffix is used for case sensitive check, it should be "AAA" got "AAB" instead',
    'We should get an error when length is not correct'
);

