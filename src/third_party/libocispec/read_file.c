/*
  Copyright 2017 Giuseppe Scrivano

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "read_file.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#ifndef BUFSIZ
// default use BUFSIZ from stdio.h
#define BUFSIZ 8096
#endif

#ifndef JSON_MAX_SIZE
#define JSON_MAX_SIZE (10LL * 1024LL * 1024LL)
#endif

char *
fread_file (FILE *stream, size_t *length)
{
  char *buf = NULL, *tmpbuf = NULL;
  size_t off = 0;

  while (1)
    {
      size_t ret;
      tmpbuf = realloc (buf, off + BUFSIZ + 1);
      if (tmpbuf == NULL)
        {
          free (buf);
          return NULL;
        }
      buf = tmpbuf;
      ret = fread (buf + off, 1, BUFSIZ, stream);
      if (ret == 0 && ferror (stream))
        {
          free (buf);
          return NULL;
        }
      if (ret < BUFSIZ || feof (stream))
        {
          *length = off + ret + 1;
          buf[*length - 1] = '\0';
          return buf;
        }
      if (off > JSON_MAX_SIZE)
        {
          free (buf);
          return NULL;
        }
      off += BUFSIZ;
    }
}

char *
read_file (const char *path, size_t *length)
{
  FILE *f;
  char *buf = NULL;

  if (!path || !length)
    return NULL;

  f = fopen (path, "r");
  if (f == NULL)
    return NULL;

  buf = fread_file (f, length);
  fclose (f);
  return buf;
}
