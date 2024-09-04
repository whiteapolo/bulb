#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define lambda(return_type, function_body)  \
({                                          \
    return_type __fn__ function_body        \
    __fn__;                                 \
 })

typedef struct {
	char *data;
	int len;
	int alocatedLen;
} string;

typedef string strView;
typedef string strStack;

#define EMPTY_STR ((strView){ .len = 0 })

#define newStrStack(fmt, ...)                                \
({                                                           \
    strStack s##__COUNTER__;                                 \
    s##__COUNTER__.len = getFmtSize(fmt, ##__VA_ARGS__);     \
    s##__COUNTER__.data =                                    \
			alloca((s##__COUNTER__.len + 1) * sizeof(char)); \
    snprintf(s##__COUNTER__.data,                            \
			s##__COUNTER__.len + 1, fmt, ##__VA_ARGS__);     \
    s##__COUNTER__;                                          \
})

#define newStrStackVa(fmt, ap)                               \
({                                                           \
    strStack s##__COUNTER__;                                 \
    va_list __apCopy;                                        \
    va_copy(__apCopy, ap);                                   \
    s##__COUNTER__.len = getFmtSizeVa(fmt, ap);              \
    s##__COUNTER__.data =                                    \
			alloca((s##__COUNTER__.len + 1) * sizeof(char)); \
    vsnprintf(s##__COUNTER__.data,                           \
			s##__COUNTER__.len + 1, fmt, apCopy);            \
    s##__COUNTER__;                                          \
})


string newStr(const char *fmt, ...);

strView newStrView(const char *s);

string newStrVa(const char *fmt, va_list ap);

string newStrFromExisting(string s);

int getFmtSize(const char *fmt, ...);

int getFmtSizeVa(const char *fmt, va_list ap);

void strPushc(string *s, char c);

char strTopc(string *s);

char strPopc(string *s);

void strPushs(string *s, const char *fmt, ...);

void strPush(string *s, string src);

bool strIsEmpty(string s);

int strCmp(string s1, string s2);

int strnCmp(string s1, string s2, int n);

bool strIsEqual(string s1, string s2);

bool strnIsEqual(string s1, string s2, int n);

bool strIsEqualC(string s1, const char *s2);

strView strTokStart(string s, const char *delim);

strView strTok(string s, strView previuosView, const char *delim);

void strForEachTok(string s, const char *delim, void (*action)(strView));

void strForEach(string s, void (*action)(char));

int strCountc(string haystack, char needle);

int strCounts(string haystack, string needle);

int strStr(string haystack, string needle, int startOffset);

void strReplace(string *s, string sub, string by, int maxOccurrences);
void strReplaceC(string *s, const char *sub, const char *by, int maxOccurrences);

void strReplaceAll(string *s, string sub, string by);
void strReplaceAllC(string *s, const char *sub, const char *by);

void strReverse(string *s);

void strToLower(string *s);

void strToUpper(string *s);

int strToNumeric(string s);

bool strIsNumeric(string s);

void strTrim(string *s);

int strDisplayedLength(const string s);

void strFree(string *s);

void strPrint(string s);

int getFileSize(FILE *fp);

#ifdef STRING_IMPL

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
const float GROW_RATE      =  1.5f;
const float SHRINK_RATE    =  0.66f;

const float GROW_TRIGGER   =  0.66f;
const float SHRINK_TRIGGER =  0.33f;

static void *exitMalloc(size_t size);
static void *exitRealloc(void *ptr, size_t size);

string newStr(const char *fmt, ...)
{
	// this is a comment
	va_list ap;
	va_start(ap, fmt);
	string s = newStrVa(fmt, ap);
	va_end(ap);
	return s;
}

string newStrVa(const char *fmt, va_list ap)
{
	string s;
	va_list ap1;
	va_copy(ap1, ap);
	
	s.len = getFmtSizeVa(fmt, ap1);
	va_end(ap1);

	s.alocatedLen = (s.len + 1) * 2;
	s.data = exitMalloc(s.alocatedLen * sizeof(char));
	va_copy(ap1, ap);
	vsnprintf(s.data, s.len + 1, fmt, ap1);
	va_end(ap1);
    return s;
}

string newStrFromExisting(string s)
{
	return newStr("%s", s.data);
}

strView newStrView(const char *s)
{
	strView view;
	view.data = (char *)s;
	view.len = strlen(s);
	return view;
}

strView newStrViewRange(string s, int start, int end)
{
	strView view;
	view.data = s.data + start;
	view.len = end - start;
	return view;
}

int getFmtSize(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	return size;
}

int getFmtSizeVa(const char *fmt, va_list ap)
{
	return vsnprintf(NULL, 0, fmt, ap);
}

static void growByOne(string *s)
{
	s->len++;
	if (s->alocatedLen == 0) {
		s->alocatedLen = 2;
		s->data = exitRealloc(s->data, s->alocatedLen);
	} else if ((float) s->len / s->alocatedLen > GROW_TRIGGER)	{
		s->alocatedLen *= GROW_RATE;
		s->data = exitRealloc(s->data, s->alocatedLen);
	}
	s->data[s->len] = '\0';
}

static void shrinkByOne(string *s)
{
	s->len--;
	if ((float) s->len / s->alocatedLen < SHRINK_TRIGGER) {
		s->alocatedLen /= GROW_RATE;
		s->data = exitRealloc(s->data, s->alocatedLen);
	}
	s->data[s->len] = '\0';
}

void strPushc(string *s, char c)
{
	growByOne(s);
	s->data[s->len - 1] = c;
}

char strTopc(string *s)
{
	return s->data[s->len - 1];
}

char strPopc(string *s)
{
	char c = strTopc(s);
	shrinkByOne(s);
	return c;
}

void strPushs(string *s, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	string tmp = newStrVa(fmt, ap);
	va_end(ap);

	for (int i = 0; i < tmp.len; i++)
		strPushc(s, tmp.data[i]);

	strFree(&tmp);
}

void strPush(string *s, string src)
{
	strPushs(s, "%s", src.data);
}

bool strIsEmpty(string s)
{
	return s.len == 0;
}

int strCmp(string s1, string s2)
{
	return strncmp(s1.data, s2.data, s1.len);
}

int strnCmp(string s1, string s2, int n)
{
	if (s1.len < n)
		return -1; 

	if (s2.len < n)
		return 1; 

	return strncmp(s1.data, s2.data, n);
}

bool strIsEqual(string s1, string s2)
{
	if (s1.len != s2.len)
		return false;
	return strCmp(s1, s2) == 0;
}

bool strIsEqualC(string s1, const char *s2)
{
	return strCmp(s1, newStrView(s2)) == 0;
}

bool strnIsEqual(string s1, string s2, int n)
{
	return strnCmp(s1, s2, n) == 0;
}

bool isStrViewOverflow(string s, strView view)
{
	return view.data + view.len > s.data + s.len;
}

strView strTokStart(string s, const char *delim)
{
	strView view;
	view.data = s.data;
	view.len = 0;
	return strTok(s, view, delim);
}

strView strTok(string s, strView previuosView, const char *delim)
{
	strView view;
	view.data = previuosView.data + previuosView.len;
	view.len = 0;

	while (!isStrViewOverflow(s, view) && 
			strchr(delim, view.data[view.len]))
	{
		view.data++;
	}

	while (!isStrViewOverflow(s, view) && 
			!strchr(delim, view.data[view.len]))
	{
		view.len++;
	}

	return view;
}

void strForEachTok(string s, const char *delim, void (*action)(strView))
{
	strView view = strTokStart(s, delim);
	while (!strIsEmpty(view)) {
		action(view);
		view = strTok(s, view, delim);
	}
}

void strForEach(string s, void (*action)(char))
{
    for (int i = 0; i < s.len; i++)
        action(s.data[i]);
}

int strCountc(string haystack, char needle)
{
	int count = 0;
	for (int i = 0; i < haystack.len; ++i)
		if (haystack.data[i] == needle)
			count++;
	return count;
}

int strCounts(string haystack, string needle)
{
	int count = 0;
	int i = -1;

	while ((i = strStr(haystack, needle, i + 1)) != -1)
		count++;

	return count;
}

int strStr(string haystack, string needle, int startOffset)
{
	for (int i = startOffset; i < haystack.len - needle.len + 1; ++i) {

		strView view;
		view.data = haystack.data + i;
		view.len = needle.len;

		if (strIsEqual(view, needle))
			return i;
	}

	return -1;
}

void strReplaceC(string *s, const char *sub, const char *by, int maxOccurrences)
{
	strReplace(s, newStrView(sub), newStrView(by), maxOccurrences);	
}

void strReplace(string *s, string sub, string by, int maxOccurrences)
{
	string result = newStr("");
	int i = 0;

	while (i < s->len - sub.len + 1 && maxOccurrences > 0) {
		strView tmp = newStrViewRange(*s, i, i + sub.len);
		if (strIsEqual(tmp, sub)) {
			strPush(&result, by);
			i += sub.len;
			maxOccurrences--;
		} else {
			strPushc(&result, s->data[i++]);
		}
	}

	// push the rest of the string
	strPushs(&result, "%.*s", s->len - i, s->data + i);

	strFree(&*s);
	*s = result;
}

void strReplaceAllC(string *s, const char *sub, const char *by)
{
	strReplaceC(s, sub, by, s->len);
}

void strReplaceAll(string *s, string sub, string by)
{
	strReplace(s, sub, by, s->len);
}

void strReverse(string *s)
{
	string tmp = newStr("");

	for (int i = s->len - 1; i >= 0; --i)
		strPushc(&tmp, s->data[i]);

	strFree(&*s);
	*s = tmp;
}

void strToLower(string *s)
{
	for (int i = 0; i < s->len; ++i)
		s->data[i] = tolower(s->data[i]);
}

void strToUpper(string *s)
{
	for (int i = 0; i < s->len; ++i)
		s->data[i] = toupper(s->data[i]);
}

int strToNumeric(string s)
{
	return atoi(s.data);
}

bool strIsNumeric(string s)
{
	for (int i = 0; i < s.len; ++i)
		if (!isdigit(s.data[i]))
			return false;
	return true;
}

void strTrim(string *s)
{
	int start = 0;
	int end = s->len - 1;

	while (start < s->len && isspace(s->data[start]))
		start++;

	while (end > 0 && isspace(s->data[end]))
		end--;

	strView view = newStrViewRange(*s, start, end + 1);
	string tmp = newStrFromExisting(view);
	strFree(&*s);
	*s = tmp;
}

int strDisplayedLength(const string s)
{
    bool escapeSeq = false;
    int len = 0;
    strForEach(s, lambda(void, (char c) {

        if (c == '\e') {
            escapeSeq = true;
            return;
        }
        
        if (escapeSeq && c == 'm') {
            escapeSeq = false;
            return;
        }

        if (escapeSeq)
            return;

        if (c == '\t')
            len += 7;

        len++;
    }));
    return len;
}

void strFree(string *s)
{
    if (s->len > 0) {
	    free(s->data);
        s->len = 0;
    }
}

void strPrint(string s)
{
    printf("%.*s", s.len, s.data);
}

static void strDir(const char *msg)
{
	perror(msg);
	exit(1);
}

static void *exitMalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL)
		strDir("exitMalloc");
	return ptr;
}

static void *exitRealloc(void *ptr, size_t size)
{
	void *newPtr = realloc(ptr, size);
	if (newPtr == NULL)
		strDir("exitRealloc");
	return newPtr;
}

#endif

#endif
