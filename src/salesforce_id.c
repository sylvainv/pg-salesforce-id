/*
 * PostgreSQL type definitions for salesforce_id type
 *
 * Written by Sylvain Verly
 * Inspired
 *
 * sylvain.verly@gmail.com
 *
 * salesforce_id/salesforce_id.c
 */

#include "postgres.h"

#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#include "fmgr.h"
#include "salesforce_id.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "utils/array.h"
#include "catalog/pg_type.h"

// BEGIN ** BIT MASK DIRECTIVE ** //
// Directives from http://www.coranac.com/documents/working-with-bits-and-bitfields/
#define BIT(n)                  (1ul<<(n) )

#define BIT_SET(y, mask)        ( y |=  (mask) )
#define BIT_CLEAR(y, mask)      ( y &= ~(mask) )
#define BIT_FLIP(y, mask)       ( y ^=  (mask) )

/* Create a bitmask of length len */
#define BIT_MASK(len) (BIT(len)-1ul)

/* Create a bitfield mask of length len starting at bit start */
#define BF_MASK(start, len) ( BIT_MASK(len)<<(start) )

/* Prepare a bitmask for insertion or combining */
#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) )

/* Extract a bitfield of length len starting at bit start from y */
#define BF_GET(y, start, len) ( ((y)>>(start)) & BIT_MASK(len) )

/* Insert a new bitfield value x into y */
#define BF_SET(y, x, start, len) \
              (y=((y) & (~BF_MASK(start, len))) | BF_PREP(x, start, len))

#define LOW 0
#define HIGH 1
#define PREFIX 2
// END ** BIT MASK DIRECTIVE ** //

PG_MODULE_MAGIC;

// In ascii order
static char salesforce_id_alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char salesforce_id_extra_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";

/*
 * Parses a character passed as integer, and set its bit into the salesforce id
 */
bool parse_character(uint32* id, uint8 str_x, char* str, uint8 start) {  
  bool is_upper_case = false;

  if ( str_x >= '0' && str_x <= '9' ) {
    *id = BF_SET(*id , str_x - 48, start, 6);
  }
  else if ( str_x >= 'A' && str_x <= 'Z' ) {
    *id = BF_SET(*id , str_x - 55, start, 6);
    is_upper_case = true;
  }
  else if ( str_x >= 'a' && str_x <= 'z' ) {
    *id = BF_SET(*id , str_x - 61, start, 6);
  }
  else {
	  ereport(ERROR,
		  (errcode(ERRCODE_SYNTAX_ERROR),
       errmsg("invalid input character \"%c\" for salesforce_id %s", str_x, str))
    );
  }
  return is_upper_case;
}

/*
 * Takes a PostgreSQL Salesforce Id amd parses it into a string
 */
void parse_salesforce_id(SalesforceId* result, char* str)
{
  int i;
  
  int length = strlen(str);
  if (length != 15) {
    if (length != 18) {
      ereport(ERROR,
        (errcode(ERRCODE_STRING_DATA_LENGTH_MISMATCH),
          errmsg("Invalid salesforce_id, should be 18 or 15 characters long, got %d instead", length))
      );
    }  
  }

  (*result)[LOW] = 0ul;  
  (*result)[HIGH] = 0ul;  
  (*result)[PREFIX] = 0ul;

  // split salesforce id in 3 of 5 characters each
  char case_sensitive_check[4];
  case_sensitive_check[3] = '\0';
  
  uint8 case_sensitive_mask = 0;
  int bit = 0;
  for (i=14; i>9; i--) {
    if (parse_character(&(*result)[LOW], str[i], str, bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i-10, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[2] = salesforce_id_extra_alphabet[case_sensitive_mask];

  bit = 0;
  case_sensitive_mask = 0;
  for (i=9; i>4; i--) {
    if (parse_character(&(*result)[HIGH], str[i], str, bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i-5, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[1] = salesforce_id_extra_alphabet[case_sensitive_mask];
  
  bit = 0;
  case_sensitive_mask = 0;  
  for (i=4;i>=0; i--) {
    if (parse_character(&(*result)[PREFIX], str[i], str, bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[0] = salesforce_id_extra_alphabet[case_sensitive_mask];

  // if length is 18, we check the case sensitive suffix
  if (length == 18) {
    char suffix[4];
    memcpy(suffix, &str[15], 4);  
    
    if (strcmp(case_sensitive_check, suffix) != 0) {
      ereport(ERROR,
        (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
          errmsg("Salesforce id suffix is used for case sensitive check, it should be \"%s\" got \"%s\" instead", case_sensitive_check, suffix))
      );
    }
  }
}

/*
 * Takes a PostgreSQL Salesforce Id amd parses it into a string
 */
char get_case_sensitive_check_char(char* str)
{
   int i;
   int result = 0;
   char str_x;
   for(i=0;i<5;i++) {
     str_x = str[i];
     if ( str_x >= 'A' && str_x <= 'Z' ) {
       BF_SET(result, 1, i, 1);
     }
     else {
       BF_SET(result, 0, i, 1);      
     }
   }
   return salesforce_id_extra_alphabet[result];
}

void emit_salesforce_id_buf(char* result, SalesforceId* salesforce_id)
{
  // Build the last 3 characters of the string
  // https://astadiaemea.wordpress.com/2010/06/21/15-or-18-character-ids-in-salesforce-com-%E2%80%93-do-you-know-how-useful-unique-ids-are-to-your-development-effort/

  int bit = 0;
	int i;
  char split[6];
  split[5] = '\0';

	for (i=14; i>9; i--) {
    split[i - 10] = result[i] = salesforce_id_alphabet[BF_GET((*salesforce_id)[LOW], bit, 6)];
    bit = bit + 6;
  }
  result[17] = get_case_sensitive_check_char(split);

  bit = 0;
  for (; i>4; i--) {
    split[i - 5] = result[i] = salesforce_id_alphabet[BF_GET((*salesforce_id)[HIGH], bit, 6)];
    bit = bit + 6;
  }
  result[16] = get_case_sensitive_check_char(split); 

  bit = 0;
  for (; i>=0; i--) {
    split[i] = result[i] = salesforce_id_alphabet[BF_GET((*salesforce_id)[PREFIX], bit, 6)];
    bit = bit + 6;
  }
  result[15] = get_case_sensitive_check_char(split); 
  
  result[18] = '\0';
}

PG_FUNCTION_INFO_V1(salesforce_id_send);
Datum
salesforce_id_send(PG_FUNCTION_ARGS)
{
    SalesforceId    *result = (SalesforceId *) PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint(&buf, (*result)[LOW], 4);
    pq_sendint(&buf, (*result)[HIGH], 4);
    pq_sendint(&buf, (*result)[PREFIX], 4);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(salesforce_id_recv);
Datum
salesforce_id_recv(PG_FUNCTION_ARGS)
{
    StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
    SalesforceId    *result;

    result = (SalesforceId *) palloc(sizeof(SalesforceId));
    (*result)[LOW] = pq_getmsgint(buf,4);
    (*result)[HIGH] = pq_getmsgint(buf,4);
    (*result)[PREFIX] = pq_getmsgint(buf,4);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(gen_random_salesforce_id);
Datum
gen_random_salesforce_id(PG_FUNCTION_ARGS)
{
  SalesforceId* result = (SalesforceId *) palloc(sizeof(SalesforceId));
  char * salesforce_id = (char*) palloc(16);
  for (int i = 0; i < 15; ++i) {
    salesforce_id[i] = salesforce_id_alphabet[rand() % 61];
  }
  salesforce_id[15] = '\0';

  parse_salesforce_id(result, salesforce_id);
	pfree(salesforce_id);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(check_salesforce_id_internal);
Datum
check_salesforce_id_internal(PG_FUNCTION_ARGS)
{
	SalesforceId* sfid = (SalesforceId *) PG_GETARG_POINTER(0);
  ArrayType* result_array;
  
  Datum inter_array[3];
  inter_array[PREFIX] = Int32GetDatum((int32) (*sfid)[PREFIX]);
  inter_array[HIGH] = Int32GetDatum((int32) (*sfid)[HIGH]);
  inter_array[LOW] = Int32GetDatum((int32) (*sfid)[LOW]);
  
  result_array = construct_array(inter_array, 3, INT4OID, sizeof(uint32), true, 'i');
  
	PG_RETURN_ARRAYTYPE_P(result_array);
}

/*
 * salesforce_id reader.
 */
PG_FUNCTION_INFO_V1(salesforce_id_in);
Datum
salesforce_id_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	SalesforceId* result = (SalesforceId *) palloc(sizeof(SalesforceId));
	
	parse_salesforce_id(result, str);
	PG_RETURN_POINTER(result);
}

/*
 * salesforce_id output function.
 */
PG_FUNCTION_INFO_V1(salesforce_id_out);
Datum
salesforce_id_out(PG_FUNCTION_ARGS)
{
	SalesforceId* salesforce_id = (SalesforceId *) PG_GETARG_POINTER(0);
  char* result = (char *) palloc(19);
  
	emit_salesforce_id_buf(result, salesforce_id);
	PG_RETURN_CSTRING(result);
}

/*
 * salesforce_id casting : in from TEXT
 */
PG_FUNCTION_INFO_V1(salesforce_id_in_text);
Datum
salesforce_id_in_text(PG_FUNCTION_ARGS)
{
	text* salesforce_id = PG_GETARG_TEXT_PP(0);
  SalesforceId* result = (SalesforceId *) palloc(sizeof(SalesforceId));
  
	parse_salesforce_id(result, text_to_cstring(salesforce_id));
	PG_FREE_IF_COPY(salesforce_id, 0);
	PG_RETURN_POINTER(result);
}

/*
 * salesforce_id casting : out to TEXT
 */
PG_FUNCTION_INFO_V1(salesforce_id_out_text);
Datum
salesforce_id_out_text(PG_FUNCTION_ARGS)
{
	SalesforceId* salesforce_id = (SalesforceId *) PG_GETARG_POINTER(0);
  char* xxx = (char *) palloc(19);
  emit_salesforce_id_buf(xxx, salesforce_id);
	text* res = cstring_to_text(xxx);
	pfree(xxx);
	
	PG_RETURN_TEXT_P(res);
}

PG_FUNCTION_INFO_V1(salesforce_id_eq);
Datum
salesforce_id_eq(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);

  PG_RETURN_BOOL(0 == memcmp(*sfid1, *sfid2, sizeof(SalesforceId)));
}

PG_FUNCTION_INFO_V1(salesforce_id_ne);
Datum
salesforce_id_ne(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
	SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);
  PG_RETURN_BOOL(0 != memcmp(*sfid1, *sfid2, sizeof(SalesforceId)));
}

PG_FUNCTION_INFO_V1(salesforce_id_le);
Datum
salesforce_id_le(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if ((*sfid1)[PREFIX] < (*sfid2)[PREFIX]) {
    PG_RETURN_BOOL(true);
  }
  else if ((*sfid1)[HIGH] < (*sfid2)[HIGH]) {
    PG_RETURN_BOOL(true);
  }
  else {
    PG_RETURN_BOOL((*sfid1)[LOW] <= (*sfid2)[LOW]);
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_lt);
Datum
salesforce_id_lt(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if ((*sfid1)[PREFIX] < (*sfid2)[PREFIX]) {
    PG_RETURN_BOOL(true);
  }
  else if ((*sfid1)[HIGH] < (*sfid2)[HIGH]) {
    PG_RETURN_BOOL(true);
  }
  else {
    PG_RETURN_BOOL((*sfid1)[LOW] < (*sfid2)[LOW]);
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_ge);
Datum
salesforce_id_ge(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	

  if ((*sfid1)[PREFIX] > (*sfid2)[PREFIX]) {
    PG_RETURN_BOOL(true);
  }
  else if ((*sfid1)[HIGH] > (*sfid2)[HIGH]) {
    PG_RETURN_BOOL(true);
  }
  else {
    PG_RETURN_BOOL((*sfid1)[LOW] >= (*sfid2)[LOW]);
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_gt);
Datum
salesforce_id_gt(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if ((*sfid1)[PREFIX] > (*sfid2)[PREFIX]) {
    PG_RETURN_BOOL(true);
  }
  else if ((*sfid1)[HIGH] > (*sfid2)[HIGH]) {
    PG_RETURN_BOOL(true);
  }
  else {
    PG_RETURN_BOOL((*sfid1)[LOW] > (*sfid2)[LOW]);
  }
}

PG_FUNCTION_INFO_V1(btcmp_salesforce_id);
Datum
btcmp_salesforce_id (PG_FUNCTION_ARGS){
  ereport(DEBUG1,
    (errmsg("-> btcmp_salesforce_id"))
  );
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
	SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);

  // Compares prefixes and HIGH vs LOW integer
	if ((*sfid1)[PREFIX] > (*sfid2)[PREFIX]) {
    PG_RETURN_INT32(1);
  }
	else if ((*sfid1)[PREFIX] < (*sfid2)[PREFIX]) {
    PG_RETURN_INT32(-1);
	}
	else {
    if ((*sfid1)[HIGH] > (*sfid2)[HIGH]) {
			PG_RETURN_INT32(1);
		}
		else if ((*sfid1)[HIGH] < (*sfid2)[HIGH]) {
			PG_RETURN_INT32(-1);		
		}
		else {
      if ((*sfid1)[LOW] > (*sfid2)[LOW]) {
        PG_RETURN_INT32(1);
      }
      else if ((*sfid1)[LOW] < (*sfid2)[LOW]) {
        PG_RETURN_INT32(-1);		
      }
      else {
        PG_RETURN_INT32(0);
      }	
		}				
	}
}

PG_FUNCTION_INFO_V1(hash_salesforce_id);
Datum
hash_salesforce_id (PG_FUNCTION_ARGS){
  // from https://github.com/postgres/postgres/commit/0052a0243d9c979a06ef273af965508103c456e0#diff-2305bf0a2a0383ad6887e5e4a637460b
  SalesforceId    *sfid = (SalesforceId *) PG_GETARG_POINTER(0);
  uint32 a = (*sfid)[LOW];
  uint32 b = (*sfid)[HIGH];
  a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
  return a;
}