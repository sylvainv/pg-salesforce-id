# PostgreSQL SalesforceId plugin

PostgreSQL plugin for representing Salesforce Id

## PostgreSQL `salesforce_id` type

The article http://www.fishofprey.com/2011/09/obscure-salesforce-object-key-prefixes.html describe the detail of a Salesforce ID.
It is made of 15 case sensitive characters and an extra optional 3 case insensitive characters that makes the whole string case insensitive.

The characters to be used are 0-9A-Za-z. This makes it possible to encode one character on only 6 bits. We can then divide the whole string in 3 parts of 5 characters. Each part can fit in a 32 bit unsigned integer using the ascii code of each character and using its index in a predefined dictionnary string. 

I follow the algorithm written in here https://salesforce.stackexchange.com/questions/27686/how-can-i-convert-a-15-char-id-value-into-an-18-char-id-value
to convert a 15 characters string to 18 and vice versa.

## Usage

```
make && make install
```

```SQL
create extension salesforce_id;

SELECT '0012800000CXn0kAAD'::salesforce_id;
SELECT '0012800000CXn0kAAD'::salesforce_id = '0012800000CXn0k'; // should be TRUE
```

## Disclaimer

It turns out the difference in size between a text and salesforce_id is not extremely significant in terms of space. About 10MB lower for salesforce_id compare to text, for about a million records. The speed seems to also be slightly faster when comparing salesforce_id vs text, but it will need benchmarking to establish a consistent improvement benefit. 

This plugin has not been tested on production.