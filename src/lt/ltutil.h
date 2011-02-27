/* Copyright (C) 2010-2011 Ian MacLarty */
#ifndef LTUTIL_H
#define LTUTIL_H

void ltAbort(const char *fmt, ...);
void ltLog(const char *fmt, ...);
bool ltFileExists(const char *file);

// Returns an array of null separated matched paths.  The last entry is two
// null characters.  The returned array should be freed by the caller with
// delete[]. The last entry in patterns should be NULL.
char* ltGlob(const char **patterns);

#endif
