/* cJSON */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <math.h>    //-lm,libm.so
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "fota_typedef.h"
#include "fota_stdlib.h"
#include "fota_cjson.h"


static const char *ep;
#define LN10 (2.302585092994046)

#define MAX_PRECISION   (13)
#if (0)
static const double rounders[MAX_PRECISION + 1] =
{
    0.5,                // 0
    0.05,               // 1
    0.005,              // 2
    0.0005,             // 3
    0.00005,            // 4
    0.000005,           // 5
    0.0000005,          // 6
    0.00000005,         // 7
    0.000000005,        // 8
    0.0000000005,       // 9
    0.00000000005,      // 10
    0.000000000005,     //11
    0.0000000000005,    //12
    0.00000000000005    //13
    
};
#endif

char * fotadm_ftoa(double f, char * buf, int precision)
{
    char * ptr = buf;
    char * p = ptr;
    char * p1;
    char c;
    long intPart;
    char* temp_str;

    // check precision bounds
    if (precision > MAX_PRECISION)
        precision = MAX_PRECISION;

    // sign stuff
    if (f < 0)
    {
        f = -f;
        *ptr++ = '-';
    }

    if (precision < 0)  // negative precision == automatic precision guess
    {
        if (f < (double)1.0) precision = 6;
        else if (f < (double)10.0) precision = 5;
        else if (f < (double)100.0) precision = 4;
        else if (f < (double)1000.0) precision = 3;
        else if (f < (double)10000.0) precision = 2;
        else if (f < (double)100000.0) precision = 1;
        else precision = 0;
    }

    // round value according the precision
    //if (precision)
    //    f += rounders[precision];

    // integer part...
    intPart = f;
    f -= intPart;

    if (!intPart)
        *ptr++ = '0';
    else
    {
        // save start pointer
        p = ptr;

        // convert (reverse order)
        while (intPart)
        {
            *p++ = '0' + intPart % 10;
            intPart /= 10;
        }

        // save end pos
        p1 = p;

        // reverse result
        while (p > ptr)
        {
            c = *--p;
            *p = *ptr;
            *ptr++ = c;
        }

        // restore end pos
        ptr = p1;
    }

    // decimal part
    if (precision)
    {
        // place decimal point
        *ptr++ = '.';

        // convert
        while (precision--)
        {
            f *= (double)10.0;
            c = f;
            *ptr++ = '0' + c;
            f -= c;
        }
    }

    // terminating zero
    *ptr = 0;

    temp_str = --ptr;
    while(*temp_str != '.')
    { 
        if(*temp_str == '0')
        {
            *temp_str = '\0';
        }
        else
        {
            break;
        }
        temp_str--;
    }   

    if((*(temp_str+1) == '\0') && (*temp_str == '.'))
    {
        *(temp_str+1) = '0';
    }

    return buf;
}


#ifdef CJSON_MEMORY_ANALYZE
int gslim_json_obj_new_count = 0;
int gslim_json_obj_free_count = 0;
int gslim_json_obj_size = 0;
#endif

//#define CJSON_MEMORY_ANALYZE

#ifdef CJSON_MEMORY_ANALYZE
int gslim_json_obj_new_count = 0;
int gslim_json_obj_free_count = 0;
int gslim_json_obj_size = 0;
#endif

const char *fota_cJSON_GetErrorPtr(void) {return ep;}

//extern void fprintf(stderr,const char *content, ...);

static int fota_cJSON_strcasecmp(const char *s1,const char *s2)
{
	if (!s1) return (s1==s2)?0:1;if (!s2) return 1;
	for(; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2)	if(*s1 == 0)	return 0;
	return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

/*
static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;
*/

/*
 * MTK,cjson malloc/free
 */
void *fota_cJSON_malloc(unsigned int sz)
{   
    //return malloc(sz);
#ifdef CJSON_MEMORY_ANALYZE    
    gslim_json_obj_new_count++;
#endif
    return (void *)fota_memalloc(sz);
}
void fota_cJSON_free(void *ptr)
{
    //free(ptr);
    fota_memfree(ptr);
#ifdef CJSON_MEMORY_ANALYZE
    gslim_json_obj_free_count ++;
#endif
}

static char* fota_cJSON_strdup(const char* str)
{
	size_t len;
	char* copy = NULL;

	len = fota_strlen(str) + 1;
	copy = (char*)fota_cJSON_malloc(len);
	if (copy == NULL)
	{
		return 0;
	}
	fota_memcpy(copy,str,len);
	return copy;
}

/*
void cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (!hooks) { 
        cJSON_malloc = malloc;
        cJSON_free = free;
        return;
    }

	cJSON_malloc = (hooks->malloc_fn)?hooks->malloc_fn:malloc;
	cJSON_free	 = (hooks->free_fn)?hooks->free_fn:free;
}
*/

/* Internal constructor. */
static cJSON *fota_cJSON_New_Item(void)
{
	cJSON* node = (cJSON*)fota_cJSON_malloc(sizeof(cJSON));
	if (node) fota_memset(node,0,sizeof(cJSON));
	return node;
}

/* Delete a cJSON structure. */
void fota_cJSON_Delete(cJSON *c)
{
	cJSON *next;
	while (c)
	{
		next=c->next;
		if (!(c->type&cJSON_IsReference) && c->child) fota_cJSON_Delete(c->child);
		if (!(c->type&cJSON_IsReference) && c->valuestring) fota_cJSON_free(c->valuestring);
		if (c->string) fota_cJSON_free(c->string);
		fota_cJSON_free(c);
		c=next;
	}
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(cJSON *item,const char *num)
{
	double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;

	if (*num=='-') sign=-1,num++;	/* Has sign? */
	if (*num=='0') num++;			/* is zero */
	if (*num>='1' && *num<='9')	do	n=(n*(double)10.0)+(*num++ -'0');	while (*num>='0' && *num<='9');	/* Number? */
	if (*num=='.' && num[1]>='0' && num[1]<='9') {num++;		do	n=(n*(double)10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');}	/* Fractional part? */
	if (*num=='e' || *num=='E')		/* Exponent? */
	{	num++;if (*num=='+') num++;	else if (*num=='-') signsubscale=-1,num++;		/* With sign? */
		while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');	/* Number? */
	}

	n=sign*n*exp((double)LN10*(scale+subscale*signsubscale));	/* number = +/- number.fraction * 10^+/- exponent */
	
	item->valuedouble=n;
	item->valueint=(int)n;
	item->type=cJSON_Number;
	return num;
}

/* Render the number nicely from the given item into a string. */
static char *fota_print_number(cJSON *item)
{
	char *str;
	double d=item->valuedouble;
	if (fabs(((double)item->valueint)-d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN)
	{
		str=(char*)fota_cJSON_malloc(21);	/* 2^64+1 can be represented in 21 chars. */
		if (str) fota_sprintf(str,"%d",item->valueint);
	}
	else
	{
		str=(char*)fota_cJSON_malloc(64);	/* This is a nice tradeoff. */
		if (str)
		{
                        /* MTK modify */
                        /*
                        if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<DBL_MAX)sprintf(str,"%.0f",d);
			else if (fabs(d)<(double)1.0e-6 || fabs(d)>(double)1.0e9)			sprintf(str,"%e",d);
			else												sprintf(str,"%f",d);
                        */
                        fotadm_ftoa(d, str, 13);
		}
	}
	return str;
}

static unsigned parse_hex4(const char *str)
{
	unsigned h=0;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	return h;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(cJSON *item,const char *str)
{
	const char *ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
	if (*str!='\"') {ep=str;return 0;}	/* not a string! */
	
	while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. */
	
	out=(char*)fota_cJSON_malloc(len+1);	/* This is how long we need for the string, roughly. */
	if (!out) return 0;
	
	ptr=str+1;ptr2=out;
	while (*ptr!='\"' && *ptr)
	{
		if (*ptr!='\\') *ptr2++=*ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
				case 'b': *ptr2++='\b';	break;
				case 'f': *ptr2++='\f';	break;
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				case 'u':	 /* transcode utf16 to utf8. */
					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */

					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/

					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
					{
						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
						uc2=parse_hex4(ptr+3);ptr+=6;
						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
					}

					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
					
					switch (len) {
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
					}
					ptr2+=len;
					break;
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	item->valuestring=out;
	item->type=cJSON_String;
	return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr(const char *str)
{
	const char *ptr;char *ptr2,*out;int len=0;unsigned char token;
	
	if (!str)
	{
		return fota_cJSON_strdup("");
	}
	
	ptr=str;
	while ((0 !=(token=*ptr)) && ++len) 
	{
		if (strchr("\"\\\b\f\n\r\t",token)) 
		{
			len++; 
		}
		else if (token<32) 
		{
			len+=5;
		}

		ptr++;
	}
	
	out=(char*)fota_cJSON_malloc(len+3);
	if (!out) return 0;

	ptr2=out;ptr=str;
	*ptr2++='\"';
	while (*ptr)
	{
		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
		else
		{
			*ptr2++='\\';
			switch (token=*ptr++)
			{
				case '\\':	*ptr2++='\\';	break;
				case '\"':	*ptr2++='\"';	break;
				case '\b':	*ptr2++='b';	break;
				case '\f':	*ptr2++='f';	break;
				case '\n':	*ptr2++='n';	break;
				case '\r':	*ptr2++='r';	break;
				case '\t':	*ptr2++='t';	break;
				default: sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
			}
		}
	}
	*ptr2++='\"';*ptr2++=0;
	return out;
}
/* Invote print_string_ptr (which is useful) on an item. */
static char *fota_print_string(cJSON *item)	{return print_string_ptr(item->valuestring);}

/* Predeclare these prototypes. */
static const char *fota_parse_value(cJSON *item,const char *value);
static char *fota_print_value(cJSON *item,int depth,int fmt);
static const char *fota_parse_array(cJSON *item,const char *value);
static char *fota_print_array(cJSON *item,int depth,int fmt);
static const char *fota_parse_object(cJSON *item,const char *value);
static char *fota_print_object(cJSON *item,int depth,int fmt);

/* Utility to jump whitespace and cr/lf */
static const char *fota_cjson_skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}

/* Parse an object - create a new root, and populate. */
cJSON *fota_cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated)
{
	const char *end=0;
	cJSON *c=fota_cJSON_New_Item();
	ep=0;
	if (!c) return 0;       /* memory fail */

	end=fota_parse_value(c,fota_cjson_skip(value));
	if (!end)	{fota_cJSON_Delete(c);return 0;}	/* parse failure. ep is set. */

	/* if we require null-terminated JSON without appended garbage, fota_cjson_skip and then check for a null terminator */
	if (require_null_terminated) {end=fota_cjson_skip(end);if (*end) {fota_cJSON_Delete(c);ep=end;return 0;}}
	if (return_parse_end) *return_parse_end=end;
	return c;
}
/* Default options for cJSON_Parse */
cJSON *fota_cJSON_Parse(const char *value) {return fota_cJSON_ParseWithOpts(value,0,0);}

/* Render a cJSON item/entity/structure to text. */
char *fota_cJSON_Print(cJSON *item)				{return fota_print_value(item,0,1);}
char *fota_cJSON_PrintUnformatted(cJSON *item)	{return fota_print_value(item,0,0);}

/* Parser core - when encountering text, process appropriately. */
static const char *fota_parse_value(cJSON *item,const char *value)
{
	if (!value)						return 0;	/* Fail on null. */
	if (!strncmp(value,"null",4))	{ item->type=cJSON_NULL;  return value+4; }
	if (!strncmp(value,"false",5))	{ item->type=cJSON_False; return value+5; }
	if (!strncmp(value,"true",4))	{ item->type=cJSON_True; item->valueint=1;	return value+4; }
	if (*value=='\"')				{ return parse_string(item,value); }
	if (*value=='-' || (*value>='0' && *value<='9'))	{ return parse_number(item,value); }
	if (*value=='[')				{ return fota_parse_array(item,value); }
	if (*value=='{')				{ return fota_parse_object(item,value); }

	ep=value;return 0;	/* failure. */
}

/* Render a value to text. */
static char *fota_print_value(cJSON *item,int depth,int fmt)
{
	char *out=0;
	if (!item) return 0;
	switch ((item->type)&255)
	{
		case cJSON_NULL:	out=fota_cJSON_strdup("null");	break;
		case cJSON_False:	out=fota_cJSON_strdup("false");break;
		case cJSON_True:	out=fota_cJSON_strdup("true"); break;
		case cJSON_Number:	out=fota_print_number(item);break;
		case cJSON_String:	out=fota_print_string(item);break;
		case cJSON_Array:	out=fota_print_array(item,depth,fmt);break;
		case cJSON_Object:	out=fota_print_object(item,depth,fmt);break;
	}
	return out;
}

/* Build an array from input text. */
static const char *fota_parse_array(cJSON *item,const char *value)
{
	cJSON *child;
	if (*value!='[')	{ep=value;return 0;}	/* not an array! */

	item->type=cJSON_Array;
	value=fota_cjson_skip(value+1);
	if (*value==']') return value+1;	/* empty array. */

	item->child=child=fota_cJSON_New_Item();
	if (!item->child) return 0;		 /* memory fail */
	value=fota_cjson_skip(fota_parse_value(child,fota_cjson_skip(value)));	/* fota_cjson_skip any spacing, get the value. */
	if (!value) return 0;

	while (*value==',')
	{
		cJSON *new_item = fota_cJSON_New_Item();
		if (NULL == new_item) return 0; 	/* memory fail */
		child->next=new_item;new_item->prev=child;child=new_item;
		value=fota_cjson_skip(fota_parse_value(child,fota_cjson_skip(value+1)));
		if (!value) return 0;	/* memory fail */
	}

	if (*value==']') return value+1;	/* end of array */
	ep=value;return 0;	/* malformed. */
}

/* Render an array to text */
static char *fota_print_array(cJSON *item,int depth,int fmt)
{
	char **entries;
	char *out=0,*ptr,*ret;int len=5;
	cJSON *child=item->child;
	int numentries=0,i=0,fail=0;
	
	/* How many entries in the array? */
	while (child) numentries++,child=child->next;
	/* Explicitly handle numentries==0 */
	if (!numentries)
	{
		out=(char*)fota_cJSON_malloc(3);
		if (out) fota_strcpy(out,"[]");
		return out;
	}
	/* Allocate an array to hold the values for each */
	entries=(char**)fota_cJSON_malloc(numentries*sizeof(char*));
	if (!entries) return 0;
	fota_memset(entries,0,numentries*sizeof(char*));
	/* Retrieve all the results: */
	child=item->child;
	while (child && !fail)
	{
		ret=fota_print_value(child,depth+1,fmt);
		entries[i++]=ret;
		if (ret) len+=strlen(ret)+2+(fmt?1:0); else fail=1;
		child=child->next;
	}
	
	/* If we didn't fail, try to malloc the output string */
	if (!fail) out=(char*)fota_cJSON_malloc(len);
	/* If that fails, we fail. */
	if (!out) fail=1;

	/* Handle failure. */
	if (fail)
	{
		for (i=0;i<numentries;i++) if (entries[i]) fota_cJSON_free(entries[i]);
		fota_cJSON_free(entries);
		return 0;
	}
	
	/* Compose the output array. */
	*out='[';
	ptr=out+1;*ptr=0;
	for (i=0;i<numentries;i++)
	{
		fota_strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
		if (i!=numentries-1) {*ptr++=',';if(fmt)*ptr++=' ';*ptr=0;}
		fota_cJSON_free(entries[i]);
	}
	fota_cJSON_free(entries);
	*ptr++=']';*ptr++=0;
	return out;	
}

/* Build an object from the text. */
static const char *fota_parse_object(cJSON *item,const char *value)
{
	cJSON *child;
	if (*value!='{')	{ep=value;return 0;}	/* not an object! */
	
	item->type=cJSON_Object;
	value=fota_cjson_skip(value+1);
	if (*value=='}') return value+1;	/* empty array. */
	
	item->child=child=fota_cJSON_New_Item();
	if (!item->child) return 0;
	value=fota_cjson_skip(parse_string(child,fota_cjson_skip(value)));
	if (!value) return 0;
	child->string=child->valuestring;child->valuestring=0;
	if (*value!=':') {ep=value;return 0;}	/* fail! */
	value=fota_cjson_skip(fota_parse_value(child,fota_cjson_skip(value+1)));	/* fota_cjson_skip any spacing, get the value. */
	if (!value) return 0;
	
	while (*value==',')
	{
		cJSON *new_item=fota_cJSON_New_Item();
		if (NULL == new_item)	return 0; /* memory fail */
		child->next=new_item;new_item->prev=child;child=new_item;
		value=fota_cjson_skip(parse_string(child,fota_cjson_skip(value+1)));
		if (!value) return 0;
		child->string=child->valuestring;child->valuestring=0;
		if (*value!=':') {ep=value;return 0;}	/* fail! */
		value=fota_cjson_skip(fota_parse_value(child,fota_cjson_skip(value+1)));	/* fota_cjson_skip any spacing, get the value. */
		if (!value) return 0;
	}
	
	if (*value=='}') return value+1;	/* end of array */
	ep=value;return 0;	/* malformed. */
}

/* Render an object to text. */
static char *fota_print_object(cJSON *item,int depth,int fmt)
{
	char **entries=0,**names=0;
	char *out=0,*ptr,*ret,*str;int len=7,i=0;
	cJSON *child=item->child;
	int numentries=0,fail=0;
	/* Count the number of entries. */
	while (child) numentries++,child=child->next;
	/* Explicitly handle empty object case */
	if (!numentries)
	{
		out=(char*)fota_cJSON_malloc(fmt?depth+4:3);
		if (!out)	return 0;
		ptr=out;*ptr++='{';
		if (fmt) {*ptr++='\n';for (i=0;i<depth-1;i++) *ptr++='\t';}
		*ptr++='}';*ptr++=0;
		return out;
	}
	/* Allocate space for the names and the objects */
	entries=(char**)fota_cJSON_malloc(numentries*sizeof(char*));
	if (!entries) return 0;
	names=(char**)fota_cJSON_malloc(numentries*sizeof(char*));
	if (!names) {fota_cJSON_free(entries);return 0;}
	fota_memset(entries,0,sizeof(char*)*numentries);
	fota_memset(names,0,sizeof(char*)*numentries);

	/* Collect all the results into our arrays: */
	child=item->child;depth++;if (fmt) len+=depth;
	while (child&&!fail)
	{
		names[i]=str=print_string_ptr(child->string);
		entries[i++]=ret=fota_print_value(child,depth,fmt);
		if (str && ret) len+=strlen(ret)+strlen(str)+2+(fmt?2+depth:0); else fail=1;
		child=child->next;
	}
	
	/* Try to allocate the output string */
	if (!fail) out=(char*)fota_cJSON_malloc(len);
	if (!out) fail=1;

	/* Handle failure */
	if (fail)
	{
		for (i=0;i<numentries;i++) {if (names[i]) fota_cJSON_free(names[i]);if (entries[i]) fota_cJSON_free(entries[i]);}
		fota_cJSON_free(names);fota_cJSON_free(entries);
		return 0;
	}
	
	/* Compose the output: */
	*out='{';ptr=out+1;/*if (fmt)*ptr++='\n';*/*ptr=0;
	for (i=0;i<numentries;i++)
	{
		/*if (fmt) for (j=0;j<depth;j++) *ptr++='\t';*/
		strcpy(ptr,names[i]);ptr+=fota_strlen(names[i]);
		*ptr++=':';/*if (fmt) *ptr++='\t';*/
		strcpy(ptr,entries[i]);ptr+=fota_strlen(entries[i]);
		if (i!=numentries-1) *ptr++=',';
		/*if (fmt) *ptr++='\n';*/*ptr=0;
		fota_cJSON_free(names[i]);fota_cJSON_free(entries[i]);
	}
	
	fota_cJSON_free(names);fota_cJSON_free(entries);
	/*if (fmt) for (i=0;i<depth-1;i++) *ptr++='\t';*/
	*ptr++='}';*ptr++=0;
	return out;	
}

/* Get Array size/item / object item. */
int    fota_cJSON_GetArraySize(cJSON *array)							{cJSON *c=array->child;int i=0;while(c)i++,c=c->next;return i;}
cJSON *fota_cJSON_GetArrayItem(cJSON *array,int item)				{cJSON *c=array->child;  while (c && item>0) item--,c=c->next; return c;}
cJSON *fota_cJSON_GetObjectItem(cJSON *object,const char *string)	{cJSON *c=object->child; while (c && fota_cJSON_strcasecmp(c->string,string)) c=c->next; return c;}

/* Utility for array list handling. */
static void fota_suffix_object(cJSON *prev,cJSON *item) {prev->next=item;item->prev=prev;}
/* Utility for handling references. */
static cJSON *fota_create_reference(cJSON *item) {cJSON *ref=fota_cJSON_New_Item();if (!ref) return 0;fota_memcpy(ref,item,sizeof(cJSON));ref->string=0;ref->type|=cJSON_IsReference;ref->next=ref->prev=0;return ref;}

/* Add item to array/object. */
void   fota_cJSON_AddItemToArray(cJSON *array, cJSON *item)						{cJSON *c=array->child;if (!item) return; if (!c) {array->child=item;} else {while (c && c->next) c=c->next; fota_suffix_object(c,item);}}
void   fota_cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)	{if (!item) return; if (item->string) fota_cJSON_free(item->string);item->string=fota_cJSON_strdup(string);fota_cJSON_AddItemToArray(object,item);}
void	fota_cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)						{fota_cJSON_AddItemToArray(array,fota_create_reference(item));}
void	fota_cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item)	{fota_cJSON_AddItemToObject(object,string,fota_create_reference(item));}

cJSON *fota_cJSON_DetachItemFromArray(cJSON *array,int which)			{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return 0;
	if (c->prev) c->prev->next=c->next;if (c->next) c->next->prev=c->prev;if (c==array->child) array->child=c->next;c->prev=c->next=0;return c;}
void   fota_cJSON_DeleteItemFromArray(cJSON *array,int which)			{fota_cJSON_Delete(fota_cJSON_DetachItemFromArray(array,which));}
cJSON *fota_cJSON_DetachItemFromObject(cJSON *object,const char *string) {int i=0;cJSON *c=object->child;while (c && fota_cJSON_strcasecmp(c->string,string)) i++,c=c->next;if (c) return fota_cJSON_DetachItemFromArray(object,i);return 0;}
void   fota_cJSON_DeleteItemFromObject(cJSON *object,const char *string) {fota_cJSON_Delete(fota_cJSON_DetachItemFromObject(object,string));}

/* Replace array/object items with new ones. */
void   fota_cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem)		{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return;
	newitem->next=c->next;newitem->prev=c->prev;if (newitem->next) newitem->next->prev=newitem;
	if (c==array->child) array->child=newitem; else newitem->prev->next=newitem;c->next=c->prev=0;fota_cJSON_Delete(c);}
void   fota_cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem){int i=0;cJSON *c=object->child;while(c && fota_cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c){newitem->string=fota_cJSON_strdup(string);fota_cJSON_ReplaceItemInArray(object,i,newitem);}}

/* Create basic types: */
cJSON *fota_cJSON_CreateNull(void)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=cJSON_NULL;return item;}
cJSON *fota_cJSON_CreateTrue(void)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=cJSON_True;return item;}
cJSON *fota_cJSON_CreateFalse(void)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=cJSON_False;return item;}
cJSON *fota_cJSON_CreateBool(int b)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=b?cJSON_True:cJSON_False;return item;}
cJSON *fota_cJSON_CreateNumber(double num)			{cJSON *item=fota_cJSON_New_Item();if(item){item->type=cJSON_Number;item->valuedouble=num;item->valueint=(int)num;}return item;}
cJSON *fota_cJSON_CreateString(const char *string)	{cJSON *item=fota_cJSON_New_Item();if(item){item->type=cJSON_String;item->valuestring=fota_cJSON_strdup(string);}return item;}
cJSON *fota_cJSON_CreateArray(void)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=cJSON_Array;return item;}
cJSON *fota_cJSON_CreateObject(void)					{cJSON *item=fota_cJSON_New_Item();if(item)item->type=cJSON_Object;return item;}

/* Create Arrays: */
cJSON *fota_cJSON_CreateIntArray(const int *numbers,int count)		{int i;cJSON *n=0,*p=0,*a=fota_cJSON_CreateArray();for(i=0;a && i<count;i++){n=fota_cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else fota_suffix_object(p,n);p=n;}return a;}
cJSON *fota_cJSON_CreateFloatArray(const float *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=fota_cJSON_CreateArray();for(i=0;a && i<count;i++){n=fota_cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else fota_suffix_object(p,n);p=n;}return a;}
cJSON *fota_cJSON_CreateDoubleArray(const double *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=fota_cJSON_CreateArray();for(i=0;a && i<count;i++){n=fota_cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else fota_suffix_object(p,n);p=n;}return a;}
cJSON *fota_cJSON_CreateStringArray(const char **strings,int count)	{int i;cJSON *n=0,*p=0,*a=fota_cJSON_CreateArray();for(i=0;a && i<count;i++){n=fota_cJSON_CreateString(strings[i]);if(!i)a->child=n;else fota_suffix_object(p,n);p=n;}return a;}

/* Duplication */
cJSON *fota_cJSON_Duplicate(cJSON *item,int recurse)
{
	cJSON *newitem,*cptr,*nptr=0,*newchild;
	/* Bail on bad ptr */
	if (!item) return 0;
	/* Create new item */
	newitem=fota_cJSON_New_Item();
	if (!newitem) return 0;
	/* Copy over all vars */
	newitem->type=item->type&(~cJSON_IsReference),newitem->valueint=item->valueint,newitem->valuedouble=item->valuedouble;
	if (item->valuestring)	{newitem->valuestring=fota_cJSON_strdup(item->valuestring);	if (!newitem->valuestring)	{fota_cJSON_Delete(newitem);return 0;}}
	if (item->string)		{newitem->string=fota_cJSON_strdup(item->string);			if (!newitem->string)		{fota_cJSON_Delete(newitem);return 0;}}
	/* If non-recursive, then we're done! */
	if (!recurse) return newitem;
	/* Walk the ->next chain for the child. */
	cptr=item->child;
	while (cptr)
	{
		newchild=fota_cJSON_Duplicate(cptr,1);		/* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild) {fota_cJSON_Delete(newitem);return 0;}
		if (nptr)	{nptr->next=newchild,newchild->prev=nptr;nptr=newchild;}	/* If newitem->child already set, then crosswire ->prev and ->next and move on */
		else		{newitem->child=newchild;nptr=newchild;}					/* Set newitem->child and move to it */
		cptr=cptr->next;
	}
	return newitem;
}

void fota_cJSON_Minify(char *json)
{
	char *into=json;
	while (*json)
	{
		if (*json==' ') json++;
		else if (*json=='\t') json++;	/* Whitespace characters. */
		else if (*json=='\r') json++;
		else if (*json=='\n') json++;
		else if (*json=='/' && json[1]=='/')  while (*json && *json!='\n') json++;	/* double-slash comments, to end of line. */
		else if (*json=='/' && json[1]=='*') {while (*json && !(*json=='*' && json[1]=='/')) json++;json+=2;}	/* multiline comments. */
		else if (*json=='\"'){*into++=*json++;while (*json && *json!='\"'){if (*json=='\\') *into++=*json++;*into++=*json++;}*into++=*json++;} /* string literals, which are \" sensitive. */
		else *into++=*json++;			/* All other characters. */
	}
	*into=0;	/* and null-terminate. */
}

int fota_json_get_value(char *text, char *key, char *value, int limit, unsigned char type)
{
    //char *out;
    cJSON *jroot, *jarray, *jcurritem;
	short idx=0,count=0;
	char found=0;
	int len=0;

    jroot = fota_cJSON_Parse(text);
    if(!jroot) 
	{
        fprintf(stderr,"Error before: [%s]\n", fota_cJSON_GetErrorPtr());
        return 0;
    } 
	else 
	{
        //out = cJSON_Print(jroot);
        //fprintf(stderr,"%s\n", out);

		//
        count = fota_cJSON_GetArraySize(jroot);
		for(idx=-1; idx<count; ++idx) 
		{
			if(idx==-1)
				jarray = jroot;
			else
		    	jarray = fota_cJSON_GetArrayItem(jroot, idx);
			
		    if(jarray != NULL)
			{
				switch(type)
				{
					case 0://string
						if((jcurritem = fota_cJSON_GetObjectItem(jarray, key)) != NULL)
						{
							//fprintf(stderr,"item[str][%s] vale:%s\r\n", key, jcurritem->valuestring);
							len = fota_strlen(jcurritem->valuestring);
							if(len <= limit)
								fota_memcpy(value, jcurritem->valuestring, len);
							found = 1;
						}
						break;
					case 1://int
						if((jcurritem = fota_cJSON_GetObjectItem(jarray, key)) != NULL)
						{
							//fprintf(stderr,"item[int][%s] vale:%d\r\n", key, jcurritem->valueint);
							len = sizeof(jcurritem->valueint);
							if(len <= limit)
								fota_memcpy(value, &(jcurritem->valueint), len);
							
							found = 1;
						}
						break;
					case 2://double
						if((jcurritem = fota_cJSON_GetObjectItem(jarray, key)) != NULL)
						{
							//fprintf(stderr,"item[double][%s] vale:%d\r\n", key, jcurritem->valuedouble);
							len = sizeof(jcurritem->valuedouble);
							if(len <= limit)
								fota_memcpy(value, &(jcurritem->valuedouble), len);
							found = 1;							
						}
						break;
					default:
						fprintf(stderr,"fota_json_get_value: type error!!!: %d\n", type);
						break;
				}

				if(found==1)
					break;
			}
			else 
			{
	            fprintf(stderr,"fota_json_get_value: error!!! idx/count:%d / %d\n", idx, count);
		    }
    	}

		if(found==0)
		{
			fprintf(stderr,"fota_json_get_value: NOT found!!! key:%s, type:%d", key, type);
		}
			
		fota_cJSON_Delete(jroot);
        //cJSON_free(out);
        return 0;
    }
}


