#ifndef PATH_H
#define PATH_H

#include "string.h"
#include "error.h"
#include <dirent.h>

#ifndef lambda
	#define lambda(return_type, function_body) \
	({ \
		  return_type __fn__ function_body \
			  __fn__; \
	})
#endif

strView getPathTail(const char *path);

strView getEnvView(const char *name);

strView getHomePath();

void expandPath(string *path);

void compressPath(string *path);

int getFileSize(FILE *fp);

ERROR readWholeFile(string *s, const char *fileName);
ERROR readWholeFileB(string *s, const char *fileName, int maxBytes);

ERROR scanfFileByName(const char *fileName, const char *fmt, ...);
ERROR writeFileByName(const char *fileName, const char *fmt, ...);
ERROR appendFileByName(const char *fileName, const char *fmt, ...);

ERROR forEveryFileInDir(const char *dir, void (*action)(const char *));
ERROR forEveryHiddenFileInDir(const char *dir, void (*action)(const char *));
ERROR forEveryRegFileInDir(const char *dir, void (*action)(const char *));

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
	strFree(expandedPath);

	*s = newStr("");

	if (fp == NULL)
		return FILE_NOT_FOUND;

	char c;

	while ((c = fgetc(fp)) != EOF && s->len != maxBytes)
		strPushc(s, c);

	fclose(fp);
	return OK;
}

ERROR scanfFileByName(const char *fileName, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		return FILE_NOT_FOUND;

	if (vfscanf(fp, fmt, ap) == EOF)
		return SCANF_ERROR;

	va_end(ap);
	return OK;
}

ERROR writeFileByName(const char *fileName, const char *fmt, ...)
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

ERROR appendFileByName(const char *fileName, const char *fmt, ...)
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

ERROR forEveryFileInDir(const char *dir, void (*action)(const char *))
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

ERROR forEveryHiddenFileInDir(const char *dir, void (*action)(const char *))
{
	return forEveryFileInDir(dir, lambda(void, (const char *fileName) {
		if (*fileName == '.')
			action(fileName);
	}));
}

ERROR forEveryRegFileInDir(const char *dir, void (*action)(const char *))
{
	return forEveryFileInDir(dir, lambda(void, (const char *fileName) {
		if (*fileName != '.')
			action(fileName);
	}));
}

#endif

#endif
