
#include "postgres.h"

#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "fmgr.h"
#include "utils/builtins.h"

Datum salesforce_id_in(PG_FUNCTION_ARGS);
Datum salesforce_id_out(PG_FUNCTION_ARGS);
Datum salesforce_id_in_text(PG_FUNCTION_ARGS);
Datum salesforce_id_out_text(PG_FUNCTION_ARGS);
Datum salesforce_id_send(PG_FUNCTION_ARGS);
Datum salesforce_id_recv(PG_FUNCTION_ARGS);
Datum gen_random_salesforce_id(PG_FUNCTION_ARGS);
Datum check_salesforce_id_internal(PG_FUNCTION_ARGS);

/*
 http://www.fishofprey.com/2011/09/obscure-salesforce-object-key-prefixes.html
 https://salesforce.stackexchange.com/questions/27686/how-can-i-convert-a-15-char-id-value-into-an-18-char-id-value
 
 Creates a SalesforceId type, which internally is basically an array of 3 unsigned 32 integers. This has the advantage of keeping the type compact,
  avoids padding and all sorts of nightmare that can result with the use of struct.

 Salesforce id characters can be encoded using 6 bits. This way we can fit 5 characters on a single 32 bit integers. A salesforce id is a 15 characters case sensitive string.
*/

typedef uint32 SalesforceId[3];
// NNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes    
// PNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes
// IIIPP 6 bits * 5 = 30 bits can fit on 32 bits or 4 bytes    

bool parse_character(uint32* id, uint8 str_x, char* str, uint8 start);
void parse_salesforce_id(SalesforceId* result, char* str);
char get_case_sensitive_check_char(char* str);
void emit_salesforce_id_buf(char* result, SalesforceId* salesforce_id);
