#ifndef PATH_H
#define PATH_H

#include "string.h"
#include "error.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define lambda(return_type, function_body)  \
({                                          \
    return_type __fn__ function_body        \
    __fn__;                                 \
 })

strView getPathTail(const char *path);

strView getEnvView(const char *name);

strView getHomePath();

void expandPath(string *path);

void compressPath(string *path);

int getFileSize(FILE *fp);

ERROR readWholeFile(string *s, const char *fileName);
ERROR readWholeFileB(string *s, const char *fileName, int maxBytes);

ERROR scanFile(const char *fileName, const char *fmt, ...);
ERROR echoToFile(const char *fileName, const char *fmt, ...);
ERROR echoAppendToFile(const char *fileName, const char *fmt, ...);

ERROR dirTraverse(const char *dir, void (*action)(const char *));
ERROR dirTraverseHiddenFiles(const char *dir, void (*action)(const char *));
ERROR dirTraverseVisibleFiles(const char *dir, void (*action)(const char *));

int getCommandOutput(const char *command, string *output, int maxBytes);

bool isdir(const char *fmt, ...);

bool isregfile(const char *fileName);

bool isfile(const char *fileName);

#ifdef PATH_IMPL

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

strView getPathTail(const char *path)
{
	(void)path;
	return EMPTY_STR;
	// strView s;
	// int pathLen = strlen(path);
	// int i = pathLen - 1;

	// if (path[i] == '/') {
	// 	s.len = 0;
	// 	return s;
	// }

	// while (i > 0 && path[i - 1] != '/')
	// 	i--;

	// return path + i;
}

strView getEnvView(const char *name)
{
	const char *env = getenv(name);
	if (env == NULL)
		return EMPTY_STR;
	return newStrView(env);
}

strView getHomePath()
{
	strView s = getEnvView("HOME");
	if (strIsEmpty(s))
		return newStrView(".");
	return s;
}

void expandPath(string *path)
{
	if (path->data[0] == '~')
		strReplace(path, newStrView("~"), getHomePath(), 1);
}

void compressPath(string *path)
{
	strView home = getHomePath();
	if (strnIsEqual(*path, home, home.len))
		strReplace(path, home, newStrView("~"), 1);
}

int getFileSize(FILE *fp)
{
	int curr = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, curr, SEEK_SET);
	return size;
}

ERROR readWholeFile(string *s, const char *fileName)
{
	return readWholeFileB(s, fileName, -1);	
}

ERROR readWholeFileB(string *s, const char *fileName, int maxBytes)
{
	string expandedPath = newStr(fileName);
	expandPath(&expandedPath);

	FILE *fp = fopen(expandedPath.data, "r");
	strFree(&expandedPath);

	if (fp == NULL)
		return FILE_NOT_FOUND;

	char c;

	while ((c = fgetc(fp)) != EOF && s->len != maxBytes)
		strPushc(s, c);

	fclose(fp);
	return OK;
}

ERROR scanFile(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return FILE_NOT_FOUND;

	if (vfscanf(fp, fmt, ap) == EOF) {
        fclose(fp);
		return SCANF_ERROR;
    }

	va_end(ap);
    fclose(fp);
	return OK;
}

ERROR echoToFile(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "w");

	if (fp == NULL)
		return FILE_NOT_FOUND;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return OK;
}

ERROR echoAppendToFile(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "a");

	if (fp == NULL)
		return FILE_NOT_FOUND;

	vfprintf(fp, fmt, ap);
	va_end(ap);
	return OK;
}

ERROR dirTraverse(const char *dir, void (*action)(const char *))
{
	struct dirent *de;
	DIR *dr = opendir(dir);

	if (dr == NULL)
		return FILE_NOT_FOUND;

	while ((de = readdir(dr)) != NULL)
		action(de->d_name);

	closedir(dr);
	return OK;
}

ERROR dirTraverseHiddenFiles(const char *dir, void (*action)(const char *))
{
	return dirTraverse(dir, lambda(void, (const char *fileName) {
		if (*fileName == '.')
			action(fileName);
	}));
}

ERROR dirTraverseVisibleFiles(const char *dir, void (*action)(const char *))
{
	return dirTraverse(dir, lambda(void, (const char *fileName) {
		if (*fileName != '.')
			action(fileName);
	}));
}

int getCommandOutput(const char *command, string *output, int maxBytes)
{
    FILE *fp = popen(command, "r");
        
    if (fp == NULL)
        return -1;

    char c;

    while ((c = fgetc(fp)) != EOF && output->len != maxBytes)
        strPushc(output, c);

    return pclose(fp);
}

bool isdir(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    strStack tmp = newStrVa(fmt, ap);
	struct stat sb;
	stat(tmp.data, &sb);
    va_end(ap);
	return S_ISDIR(sb.st_mode);
}

bool isregfile(const char *fileName)
{
	struct stat sb;
	stat(fileName, &sb);
	return S_ISREG(sb.st_mode);
}

bool isfile(const char *fileName)
{
	return !access(fileName, F_OK);
}

#endif

#endif
