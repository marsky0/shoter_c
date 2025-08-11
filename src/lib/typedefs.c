#include "typedefs.h"

void string_free(string* str) {
	if (str->str) {
		free(str->str);
	}
    str->str = NULL;
	str->length = 0;
	str->capacity = 0;
}

void string_set(string* str, const char* chars) {
    string_free(str);
    str->length = strlen(chars);
    str->capacity = str->length + 1;    
    str->str = calloc(str->capacity, sizeof(char));
	if (str->str != NULL) {
		strcpy(str->str, chars);
	}
}

string string_new(char* c) {
	string s = {NULL, 0, 0};
	string_set(&s, c);
	return s;
}

string* string_split(string* str, char delim, u64* count) {
	string* vec = NULL;
	*count = 0;

	u64 icount = 0;
	u64 ichar = 0;

	char* tmp_str = calloc(str->capacity, sizeof(char));
	if (tmp_str == NULL) return NULL;

	for (u64 i=0; i <= str->length; i++) {
		if ((str->str[i] == delim && ichar > 0) || (str->str[i] == '\0' && ichar > 0)) {
			tmp_str[ichar] = '\0';	
			
			vec = realloc(vec, sizeof(string)*(icount+1));
			if (vec == NULL) return NULL;
			vec[icount].str = NULL;
			vec[icount].length = 0;
			vec[icount].capacity = 0;
			
			string_set(&vec[icount], tmp_str);

			memset(tmp_str, 0, str->length);
			ichar = 0;
			icount++;
		} else if (str->str[i] != delim) {
			tmp_str[ichar] = str->str[i];
			ichar++;
		}
	}
	if (icount == 0) return NULL;

	*count = icount;
	return vec;
}

i32 compare_strings(const void* a, const void* b) {
    const string* str1 = (const string*)a;
    const string* str2 = (const string*)b;
    return strcmp(str1->str, str2->str);
}

void strings_sort(string* arr, u64 size) {
    qsort(arr, size, sizeof(string), compare_strings);
}

