\echo Use "CREATE EXTENSION salesforce_id" to load this file. \quit

-----------------------------------------------------------------------------
--                            salesforce_id                                --
-----------------------------------------------------------------------------

--
--	Input and output functions and the type itself:

CREATE TYPE salesforce_id;

CREATE OR REPLACE FUNCTION salesforce_id_in(cstring)
	RETURNS salesforce_id
	AS 'salesforce_id', 'salesforce_id_in'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_out(salesforce_id)
	RETURNS cstring
	AS 'salesforce_id', 'salesforce_id_out'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_send(salesforce_id)
	RETURNS bytea
	AS 'salesforce_id', 'salesforce_id_send'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_recv(internal)
	RETURNS salesforce_id
	AS 'salesforce_id', 'salesforce_id_recv'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_in_text(text)
	RETURNS salesforce_id
	AS 'salesforce_id', 'salesforce_id_in_text'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_out_text(salesforce_id)
	RETURNS text
	AS 'salesforce_id', 'salesforce_id_out_text'
	LANGUAGE C STRICT IMMUTABLE;

CREATE TYPE salesforce_id (
	INPUT = salesforce_id_in,
	OUTPUT = salesforce_id_out,
-- values of internallength, passedbyvalue, alignment, and storage are copied from the named type.
    INTERNALLENGTH = 12,
	--SEND = salesforce_id_send,
	--RECEIVE = salesforce_id_recv,
-- string category, to automatically try string conversion etc
	CATEGORY = 'S',
	PREFERRED = false,
	ALIGNMENT = int4
);

--
--	Comparison functions
--
CREATE OR REPLACE FUNCTION salesforce_id_eq(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_eq'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_ne(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_ne'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_le(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_le'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_lt(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_lt'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_ge(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_ge'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION salesforce_id_gt(salesforce_id, salesforce_id)
	RETURNS bool
	AS 'salesforce_id', 'salesforce_id_gt'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION btcmp_salesforce_id(salesforce_id, salesforce_id)
	RETURNS int4
	AS 'btint8cmp'
	LANGUAGE internal STRICT IMMUTABLE;

-- this function seems to "hash" the int8 to a much bigger size; eg
-- 1 => -1905060026
-- CREATE OR REPLACE FUNCTION hash_salesforce_id(salesforce_id)
-- 	RETURNS int4
-- 	AS 'hashint8'
-- 	LANGUAGE internal STRICT IMMUTABLE;
--
--	Now the operators.
--
CREATE OPERATOR = (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = <>,
	procedure = salesforce_id_eq,
	restrict = eqsel,
	commutator = =,
	join = eqjoinsel,
	hashes, merges
);

CREATE OPERATOR <> (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = =,
	procedure = salesforce_id_ne,
	restrict = neqsel,
	join = neqjoinsel
);

CREATE OPERATOR #<# (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = #>=#,
	procedure = salesforce_id_lt
);

CREATE OPERATOR #<=# (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = #>#,
	procedure = salesforce_id_le
);

CREATE OPERATOR #># (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = #<=#,
	procedure = salesforce_id_gt
);

CREATE OPERATOR #>=# (
	leftarg = salesforce_id,
	rightarg = salesforce_id,
	negator = #<#,
	procedure = salesforce_id_ge
);

--
--  Implicit and assignment type casts.
--

-- ...not sure yet
CREATE CAST (salesforce_id AS text)    WITH FUNCTION salesforce_id_out_text(salesforce_id) AS IMPLICIT;
CREATE CAST (text AS salesforce_id)    WITH FUNCTION salesforce_id_in_text(text) AS IMPLICIT;

--
-- The btree indexing operator class.
--

CREATE OPERATOR CLASS salesforce_id_ops
DEFAULT FOR TYPE salesforce_id USING btree AS
    OPERATOR    1   #<#  (salesforce_id, salesforce_id),
    OPERATOR    2   #<=# (salesforce_id, salesforce_id),
    OPERATOR    3   =    (salesforce_id, salesforce_id),
    OPERATOR    4   #>=# (salesforce_id, salesforce_id),
    OPERATOR    5   #>#  (salesforce_id, salesforce_id),
    FUNCTION    1   btcmp_salesforce_id(salesforce_id, salesforce_id);

    --
    -- The hash indexing operator class.
    --

-- CREATE OPERATOR CLASS salesforce_id_ops
-- DEFAULT FOR TYPE salesforce_id USING hash AS
--	OPERATOR    1   =  (salesforce_id, salesforce_id),	FUNCTION    1   hash_salesforce_id(salesforce_id);
	