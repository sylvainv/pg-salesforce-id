
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

/*
 http://www.fishofprey.com/2011/09/obscure-salesforce-object-key-prefixes.html
 https://salesforce.stackexchange.com/questions/27686/how-can-i-convert-a-15-char-id-value-into-an-18-char-id-value
*/
typedef struct SalesforceId {
    uint32 prefix; // IIIPP 6 bits * 5 = 30 bits can fit on 32 bits or 4 bytes
    uint32 high; // PNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes
    uint32 low; // NNNNN 6 bits * 5 = 30 can fit in 32 bits or 4 bytes
} SalesforceId;

bool parse_character(uint32* id, uint8 str_x, uint8 start);
char get_case_sensitive_check_char(char* str);

void parse_salesforce_id(SalesforceId *result, char* str);
void emit_salesforce_id_buf(char* result, SalesforceId* salesforce_id);
