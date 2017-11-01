
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>


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
      
              typedef struct {
                unsigned long prefix; // PP 6 bits * 5 = 12 bits can fit in int16
                unsigned long long id; // PNNNNNNNNN 6 bits * 10 = 60 can fit in int64
            } SalesforceId;
// In ascii order
static char salesforce_id_alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char salesforce_id_extra_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";

unsigned long long parse_character(unsigned long long id, char str_x, int start) {  
  if ( str_x >= '0' && str_x <= '9' ) {
    BF_SET(id , (unsigned char) str_x - 48, start, 6);
  }
  else if ( str_x >= 'A' && str_x <= 'Z' ) {
    BF_SET(id , (unsigned char) str_x - 55, start, 6);
  }
  else if ( str_x >= 'a' && str_x <= 'z' ) {
    BF_SET(id , (unsigned char) str_x - 61, start, 6);
  }
  else {

  }
  return id;
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

void parse_salesforce_id(SalesforceId* result, char* str)
{
  int i;
    
  int bit = 0;
  result->id = 0ull;
  result->prefix = 0ul;

  for (i=14; i>=5; i--) {
    result->id = parse_character(result->id, str[i], bit);
    bit = bit + 6;
  }

  bit = 0;
  for (i=4; i>0; i--) {
    result->prefix = parse_character(result->prefix, str[i], bit);
    bit = bit + 6;
  }

  int start = 0;
  char split[6];
  char case_sensitive_check[4];
  char suffix[4];
  memcpy(suffix, &str[15], 4);
  for(i=0;i<3;i++, start+=5) {
    memcpy(split, &str[start], 5);
    split[5] = '\0';
    case_sensitive_check[i] = get_case_sensitive_check_char(split);
  }
  case_sensitive_check[3] = '\0';

  printf("%s %s\n", case_sensitive_check, suffix);
}

char* emit_salesforce_id(SalesforceId* salesforce_id) {
  char* result = (char *) malloc(19);

  int bit = 0;
	int i;

	for (i=14; i>4; i--) {
    result[i] = salesforce_id_alphabet[BF_GET(salesforce_id->id, bit, 6)];
    bit = bit + 6;    
  }

  bit = 0;
  for (i=4; i>=0; i--) {
    result[i] = salesforce_id_alphabet[BF_GET(salesforce_id->prefix, bit, 6)];
    bit = bit + 6;
  }
  
  // Build the last 3 characters of the string
  // https://astadiaemea.wordpress.com/2010/06/21/15-or-18-character-ids-in-salesforce-com-%E2%80%93-do-you-know-how-useful-unique-ids-are-to-your-development-effort/
  char split[5];
  
  int start = 0;
  for(i=0;i<3;i++, start+=5) {
    memcpy(split, &result[start], 5);
    result[15 + i] = get_case_sensitive_check_char(split);  
  }
  result[18] = '\0';

  return result;
}


int main()
{
  SalesforceId* result = malloc(sizeof(SalesforceId));
  char* newstring;
  char* string;

  printf("GOOO\n");
  string = "00528000001H8n1AAB";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));  
  string = "00528000001EqrcAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EqsQAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F5IcAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F7o7AAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9isAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9jHAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9jRAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9jbAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EiamAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EiaiAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001G59CAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001G96JAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H7sLAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F1DzAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F1EJAA0";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F7nJAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F7nsAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F7nxAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8YXAA0";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8ZaAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8bqAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8ncAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HA2FAAW";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001Ek75AAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EqvFAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001FrDgAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F0FjAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EqvAAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001FhYHAA0";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001GBXsAAO";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9koAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9ktAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9l3AAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EiajAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EiakAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EjSnAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9jvAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9kAAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9kPAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9kUAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ErwuAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EianAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F1CcAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001FiCLAA0";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000000U6G0AAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EhkZAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F1EOAA0";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001F7o2AAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001G5RuAAK";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8cjAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9j2AAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H9jWAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HAXcAAO";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HAaRAAW";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HEXQAA4";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EialAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001G09yAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8cWAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8ctAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8enAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8mrAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8msAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001H8mtAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HykJAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HykgAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HykkAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ICH3AAO";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ICH5AAO";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ICIAAA4";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HyjSAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP98AAE";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HyjvAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HyksAAC";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ICGvAAO";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9BAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001HykNAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEobAAE";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNVAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNYAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNlAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNsAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pHg8AAE";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEoWAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9DAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9EAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9HAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9MAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eaqhAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNNAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNaAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNdAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNnAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pJTpAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNQAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNTAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNOAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNPAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001g1sDAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEoHAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEovAAE";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNfAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNqAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNmAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001gkhkAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNRAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNcAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNiAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pVZKAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9LAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEpFAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNLAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9FAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9IAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEpAAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNhAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNoAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9GAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNKAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGMEAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9NAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ggCAAAY";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNkAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hGNpAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001eP9KAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001egZtAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001egbaAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001hEogAAE";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024039AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xSsAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xSmAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xSvAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xT6AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTGAAY";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTrAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTsAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pay2AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x6eaAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022pbnAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xT3AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pzVPAAY";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x6euAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTCAAY";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xU3AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xU4AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000023uyKAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002402wAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024038AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x6cZAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x7f5AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001xNoLAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001xQScAAM";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002402tAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024034AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pWvAAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pc4sAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTuAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xT5AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pv5yAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pXYFAA2";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xTEAAY";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pWvFAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001ptahAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x7gcAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xT2AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xT4AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xU2AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002402vAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002402yAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024037AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pkclAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pnViAAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000022xU1AAI";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000023A1MAAU";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001x86kAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002402zAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024036AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pivTAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024032AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001pmiAAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "005280000024044AAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002404BAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002404IAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403GAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403FAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403IAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403SAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403WAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403YAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002403bAAA";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002404CAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000002404HAAQ";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  string = "00528000001EqvFAAS";parse_salesforce_id(result, string); newstring = emit_salesforce_id(result); printf("%s %s %d\n", string, newstring, strcmp(string, newstring));
  
  return 0;
}