CREATE EXTENSION IF NOT EXISTS salesforce_id;
CREATE EXTENSION IF NOT EXISTS pgtap;

SELECT '0012800000CXn0kAAD'::salesforce_id;
SELECT '0012800000CXmbXAAT'::salesforce_id;
SELECT '001p000000OoULiAAN'::salesforce_id;
SELECT '000000000000000AAA'::salesforce_id;
SELECT 'AAAAAJJJJJPPPPP555'::salesforce_id;
SELECT 'zzzzzzzzzzzzzzzAAA'::salesforce_id;

SELECT plan(22);
SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id = 'zzzzzzzzzzzzzzzAAA'::salesforce_id, TRUE, 'Comparison equality should work');
SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id = 'zzzzzzzzzzzzzzz'::salesforce_id, TRUE, 'Comparison equality should work');
SELECT is('0012800000CXmbXAAT'::salesforce_id = '0012800000CXmbX'::salesforce_id, TRUE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = '000000000000000AAA'::salesforce_id, TRUE, 'Comparison equality should work');
SELECT is('0012800000CXmbXAAT'::salesforce_id = '0012800000CXmbXAAT'::salesforce_id, TRUE, 'Comparison equality should work');
SELECT is('0012800000CXmbXAAT'::salesforce_id = 'zzzzzzzzzzzzzzzAAA'::salesforce_id, FALSE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = 'zzzzzzzzzzzzzzzAAA'::salesforce_id, FALSE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = '0012800000CXmbXAAT'::salesforce_id, FALSE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = '0012800000CXmbXAAT'::salesforce_id, '0012800000CXmbXAAT'::salesforce_id = '000000000000000AAA'::salesforce_id, 'Comparison equality should work and is commutative');

SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id > '000000000000000AAA'::salesforce_id, TRUE, 'gt comparison should work');
SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id >= '000000000000000AAA'::salesforce_id, TRUE, 'ge comparison should work');
SELECT is('000000000000000AAA'::salesforce_id < 'zzzzzzzzzzzzzzzAAA'::salesforce_id, TRUE, 'lt comparison should work');
SELECT is('000000000000000AAA'::salesforce_id <= 'zzzzzzzzzzzzzzzAAA'::salesforce_id, TRUE, 'le comparison should work');

-- Auto casting works
SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id = 'zzzzzzzzzzzzzzzAAA', TRUE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = '000000000000000AAA', TRUE, 'Comparison equality should work');
SELECT is('0012800000CXmbXAAT'::salesforce_id = '0012800000CXmbXAAT', TRUE, 'Comparison equality should work');
SELECT is('0012800000CXmbXAAT'::salesforce_id = 'zzzzzzzzzzzzzzzAAA', FALSE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = 'zzzzzzzzzzzzzzzAAA', FALSE, 'Comparison equality should work');
SELECT is('000000000000000AAA'::salesforce_id = '0012800000CXmbXAAT', FALSE, 'Comparison equality should work');

SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id > '000000000000000AAA', TRUE, 'gt comparison should work');
SELECT is('zzzzzzzzzzzzzzzAAA'::salesforce_id >= '000000000000000AAA', TRUE, 'ge comparison should work');
SELECT is('000000000000000AAA'::salesforce_id < 'zzzzzzzzzzzzzzzAAA', TRUE, 'lt comparison should work');
SELECT is('000000000000000AAA'::salesforce_id <= 'zzzzzzzzzzzzzzzAAA', TRUE, 'le comparison should work');

