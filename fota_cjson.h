#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
	
#define cJSON_IsReference 256

/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	double valuedouble;			/* The item's number, if type==cJSON_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;

/*
typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;
*/

/* Supply malloc, realloc and free functions to cJSON */
//extern void cJSON_InitHooks(cJSON_Hooks* hooks);

extern char * fotadm_ftoa(double f, char * buf, int precision);

/* malloc/free functions */
extern void *fota_cJSON_malloc(unsigned int sz);
extern void fota_cJSON_free(void *ptr);

/* Supply a block of JSON, and this returns a cJSON object you can interrogate. Call cJSON_Delete when finished. */
extern cJSON *fota_cJSON_Parse(const char *value);
/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char  *fota_cJSON_Print(cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *fota_cJSON_PrintUnformatted(cJSON *item);
/* Delete a cJSON entity and all subentities. */
extern void   fota_cJSON_Delete(cJSON *c);

/* Returns the number of items in an array (or object). */
extern int	  fota_cJSON_GetArraySize(cJSON *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern cJSON *fota_cJSON_GetArrayItem(cJSON *array,int item);
/* Get item "string" from object. Case insensitive. */
extern cJSON *fota_cJSON_GetObjectItem(cJSON *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
extern const char *fota_cJSON_GetErrorPtr(void);
	
/* These calls create a cJSON item of the appropriate type. */
extern cJSON *fota_cJSON_CreateNull(void);
extern cJSON *fota_cJSON_CreateTrue(void);
extern cJSON *fota_cJSON_CreateFalse(void);
extern cJSON *fota_cJSON_CreateBool(int b);
extern cJSON *fota_cJSON_CreateNumber(double num);
extern cJSON *fota_cJSON_CreateString(const char *string);
extern cJSON *fota_cJSON_CreateArray(void);
extern cJSON *fota_cJSON_CreateObject(void);

/* These utilities create an Array of count items. */
extern cJSON *fota_cJSON_CreateIntArray(const int *numbers,int count);
extern cJSON *fota_cJSON_CreateFloatArray(const float *numbers,int count);
extern cJSON *fota_cJSON_CreateDoubleArray(const double *numbers,int count);
extern cJSON *fota_cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void fota_cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void	fota_cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
extern void fota_cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
extern void	fota_cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
extern cJSON *fota_cJSON_DetachItemFromArray(cJSON *array,int which);
extern void   fota_cJSON_DeleteItemFromArray(cJSON *array,int which);
extern cJSON *fota_cJSON_DetachItemFromObject(cJSON *object,const char *string);
extern void   fota_cJSON_DeleteItemFromObject(cJSON *object,const char *string);
	
/* Update array items. */
extern void fota_cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem);
extern void fota_cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);

/* Duplicate a cJSON item */
extern cJSON *fota_cJSON_Duplicate(cJSON *item,int recurse);
/* Duplicate will create a new, identical cJSON item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
extern cJSON *fota_cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated);

extern void fota_cJSON_Minify(char *json);

/* Macros for creating things quickly. */
#define cJSON_AddNullToObject(object,name)		fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)		fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b)	fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n)	fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	fota_cJSON_AddItemToObject(object, name, fota_cJSON_CreateString(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cJSON_SetIntValue(object,val)			((object)?(object)->valueint=(object)->valuedouble=(val):(val))

#ifdef __cplusplus
}
#endif

#endif
