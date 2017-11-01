
#include "postgres.h"

#include <time.h>
#include <unistd.h>

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
typedef struct {
    unsigned long prefix; // PP 6 bits * 5 = 12 bits can fit in int16
    unsigned long long id; // PNNNNNNNNN 6 bits * 10 = 60 can fit in int64
} SalesforceId;

unsigned long long parse_character(unsigned long long id, char str_x, int start);
char get_case_sensitive_check_char(char* str);

void parse_salesforce_id(SalesforceId *result, char* str);
void emit_salesforce_id_buf(char* result, SalesforceId* salesforce_id);
