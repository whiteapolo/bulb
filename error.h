#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

typedef const char *ERROR;

const ERROR OK               = 0;

const ERROR FILE_NOT_FOUND   = "File not found";
const ERROR FILE_READ_ERROR  = "File read error";
const ERROR FILE_WRITE_ERROR = "File write error";
const ERROR SCANF_ERROR      = "Scanf input not as expected";

void printError(const char *label, ERROR e);

#ifdef ERROR_IMPL

void printError(const char *label, ERROR e)
{
    fprintf(stderr, "%s: %s\n", label, e);
}

#endif

#endif
