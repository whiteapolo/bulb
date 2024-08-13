#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

typedef enum {
	OK,
	FILE_NOT_FOUND,
	SCANF_ERROR,
	FILE_READ_ERROR,
	FILE_WRITE_ERROR,
	UNKNOWN,
} ERROR;

const char *errorToStr(ERROR e);
void printError(const char *label, ERROR e);

#ifdef ERROR_IMPL

const char *errorToStr(ERROR e)
{
	switch (e) {
	  case FILE_NOT_FOUND:   return "File not found";
	  case SCANF_ERROR:      return "Scanf error";
	  case FILE_READ_ERROR:  return "File read error";
	  case FILE_WRITE_ERROR: return "File write error";
	  case UNKNOWN:          return "Unknown error";
	  default:               return "Invalid error code";
	}
}

void printError(const char *label, ERROR e)
{
	printf("%s: %s\n", label, errorToStr(e));
}

#endif

#endif
