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

#define BIT(n)                  (1ull<<(n) )

#define BIT_SET(y, mask)        ( y |=  (mask) )
#define BIT_CLEAR(y, mask)      ( y &= ~(mask) )
#define BIT_FLIP(y, mask)       ( y ^=  (mask) )

/* Create a bitmask of length len */
#define BIT_MASK(len) (BIT(len)-1ull)

/* Create a bitfield mask of length len starting at bit start */
#define BF_MASK(start, len) ( BIT_MASK(len)<<(start) )

/* Prepare a bitmask for insertion or combining */
#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) )

/* Extract a bitfield of length len starting at bit start from y */
#define BF_GET(y, start, len) ( ((y)>>(start)) & BIT_MASK(len) )

/* Insert a new bitfield value x into y */
#define BF_SET(y, x, start, len) \
              (y=((y) & (~BF_MASK(start, len))) | BF_PREP(x, start, len))
      
PG_MODULE_MAGIC;

// In ascii order
static char salesforce_id_alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char salesforce_id_extra_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";

bool parse_character(uint32* id, uint8 str_x, uint8 start) {  
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
		  (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
       errmsg("invalid input character \"%c\" for salesforce_id", str_x))
    );
  }
  return is_upper_case;
}

void parse_salesforce_id(SalesforceId* result, char* str)
{
  int i;
  
  int length = strlen(str);
  if (length != 18) {
    ereport(ERROR,
      (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("Invalid salesforce_id, should be 18 characters long, got %d instead", length))
    );
  }

  result->high = 0ul;
  result->low = 0ul;  
  result->prefix = 0ul;

  char case_sensitive_check[4];
  case_sensitive_check[3] = '\0';
  
  uint8 case_sensitive_mask = 0;
  int bit = 0;
  for (i=14; i>9; i--) {
    if (parse_character(&result->low, str[i], bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i-10, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[2] = salesforce_id_extra_alphabet[case_sensitive_mask];

  bit = 0;
  case_sensitive_mask = 0;
  for (i=9; i>4; i--) {
    if (parse_character(&result->high, str[i], bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i-5, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[1] = salesforce_id_extra_alphabet[case_sensitive_mask];
  
  bit = 0;
  case_sensitive_mask = 0;  
  for (i=4;i>0; i--) {
    if (parse_character(&result->prefix, str[i], bit)) {
      case_sensitive_mask = BF_SET(case_sensitive_mask, 1, i, 1);      
    }
    bit = bit + 6;
  }
  case_sensitive_check[0] = salesforce_id_extra_alphabet[case_sensitive_mask];

  char suffix[4];
  memcpy(suffix, &str[15], 4);

  if (strcmp(case_sensitive_check, suffix) != 0) {
    ereport(ERROR,
      (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("Salesforce id suffix is used for case sensitive check, it should be \"%s\" got \"%s\" instead", case_sensitive_check, suffix))
    );
  }
}

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
  ereport(DEBUG1,
    (errmsg("-> emit_salesforce_id_buf"))
  );

  // Build the last 3 characters of the string
  // https://astadiaemea.wordpress.com/2010/06/21/15-or-18-character-ids-in-salesforce-com-%E2%80%93-do-you-know-how-useful-unique-ids-are-to-your-development-effort/

  int bit = 0;
	int i;
  char split[6];
  split[5] = '\0';

	for (i=14; i>9; i--) {
    split[i - 10] = result[i] = salesforce_id_alphabet[BF_GET(salesforce_id->low, bit, 6)];
    bit = bit + 6;
  }
  result[17] = get_case_sensitive_check_char(split);

  bit = 0;
  for (; i>4; i--) {
    split[i - 5] = result[i] = salesforce_id_alphabet[BF_GET(salesforce_id->high, bit, 6)];
    bit = bit + 6;
  }
  result[16] = get_case_sensitive_check_char(split);  
  
  bit = 0;
  for (; i>=0; i--) {
    split[i] = result[i] = salesforce_id_alphabet[BF_GET(salesforce_id->prefix, bit, 6)];
    bit = bit + 6;
  }
  result[15] = get_case_sensitive_check_char(split);  
  
  result[18] = '\0';
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
  ereport(DEBUG1,
    (errmsg("-> salesforce_id_in_text"))
  );
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
  ereport(DEBUG1,
    (errmsg("-> salesforce_id_in_text"))
  );
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
  ereport(DEBUG1,
    (errmsg("-> salesforce_id_out_text"))
  );
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
  PG_RETURN_BOOL(sfid1->low == sfid2->low && sfid1->high == sfid2->high && sfid1->prefix == sfid2->prefix);
}

PG_FUNCTION_INFO_V1(salesforce_id_ne);
Datum
salesforce_id_ne(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
	SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  PG_RETURN_BOOL(sfid1->low != sfid2->low || sfid1->high != sfid2->high || sfid1->prefix != sfid2->prefix);
}

PG_FUNCTION_INFO_V1(salesforce_id_le);
Datum
salesforce_id_le(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if (sfid1->prefix <= sfid2->prefix) {
    PG_RETURN_BOOL(true);
  }
  else {
    if (sfid1->high <= sfid2->high) {
      PG_RETURN_BOOL(true);
    }
    else {
      PG_RETURN_BOOL(sfid1->low <= sfid2->low);
    }
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_lt);
Datum
salesforce_id_lt(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if (sfid1->prefix < sfid2->prefix) {
    PG_RETURN_BOOL(true);
  }
  else {
    if (sfid1->high < sfid2->high) {
      PG_RETURN_BOOL(true);
    }
    else {
      PG_RETURN_BOOL(sfid1->low < sfid2->low);
    }
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_ge);
Datum
salesforce_id_ge(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  if (sfid1->prefix >= sfid2->prefix) {
    PG_RETURN_BOOL(true);
  }
  else {
    if (sfid1->high >= sfid2->high) {
      PG_RETURN_BOOL(true);
    }
    else {
      PG_RETURN_BOOL(sfid1->low >= sfid2->low);
    }
  }
}

PG_FUNCTION_INFO_V1(salesforce_id_gt);
Datum
salesforce_id_gt(PG_FUNCTION_ARGS)
{
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
  SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);	
  
  if (sfid1->prefix > sfid2->prefix) {
    PG_RETURN_BOOL(true);
  }
  else {
    if (sfid1->high > sfid2->high) {
      PG_RETURN_BOOL(true);
    }
    else {
      PG_RETURN_BOOL(sfid1->low > sfid2->low);
    }
  }
}

PG_FUNCTION_INFO_V1(btcmp_salesforce_id);
Datum
btcmp_salesforce_id (PG_FUNCTION_ARGS){
	SalesforceId    *sfid1 = (SalesforceId *) PG_GETARG_POINTER(0);
	SalesforceId    *sfid2 = (SalesforceId *) PG_GETARG_POINTER(1);

	if (sfid1->prefix > sfid2->prefix) {
    PG_RETURN_INT32(1);
  }
	else if (sfid1->prefix < sfid2->prefix) {
    PG_RETURN_INT32(-1);
	}
	else {
    if (sfid1->high > sfid2->high) {
			PG_RETURN_INT32(1);
		}
		else if (sfid1->high < sfid2->high) {
			PG_RETURN_INT32(-1);		
		}
		else {
      if (sfid1->low > sfid2->low) {
        PG_RETURN_INT32(1);
      }
      else if (sfid1->low < sfid2->low) {
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
  uint32 a = sfid->low;
  uint32 b = sfid->high;
  a ^= b + 0x9e3779b9 + (a << 6) + (a >> 2);
  return a;
}