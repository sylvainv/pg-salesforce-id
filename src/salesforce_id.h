
#include "postgres.h"

#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "fmgr.h"
#include "utils/builtins.h"

Datum		salesforce_id_in(PG_FUNCTION_ARGS);
Datum		salesforce_id_out(PG_FUNCTION_ARGS);
Datum		salesforce_id_in_text(PG_FUNCTION_ARGS);
Datum		salesforce_id_out_text(PG_FUNCTION_ARGS);
Datum		salesforce_id_send(PG_FUNCTION_ARGS);
Datum		salesforce_id_recv(PG_FUNCTION_ARGS);
Datum       gen_random_salesforce_id(PG_FUNCTION_ARGS);

/*
 http://www.fishofprey.com/2011/09/obscure-salesforce-object-key-prefixes.html
 https://salesforce.stackexchange.com/questions/27686/how-can-i-convert-a-15-char-id-value-into-an-18-char-id-value
*/
typedef uint32 SalesforceId[3];
// NNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes    
// PNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes
// IIIPP 6 bits * 5 = 30 bits can fit on 32 bits or 4 bytes    
