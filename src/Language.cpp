/*
  Language.cpp

  Copyright (C) 2008  SDML (www.sdml.info)

  This file is part of the srcML translator.

  The srcML translator is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  The srcML translator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Language.h"
#include <regex.h>

pair Language::lang2int[] = {
    { LanguageName::LANGUAGE_C, LANGUAGE_C },
    { LanguageName::LANGUAGE_CXX, LANGUAGE_CXX },
    { LanguageName::LANGUAGE_JAVA, LANGUAGE_JAVA },
    { LanguageName::LANGUAGE_CXX_0X, LANGUAGE_CXX_0X },
    { LanguageName::LANGUAGE_ASPECTJ, LANGUAGE_ASPECTJ },
    { 0, 0 }
};

static int usercount = 0;

pair Language::userext2int[47] = { { 0, 0 } };

const char * const regex = "(zg\\.|2zb\\.)*([^\\.]*)";

bool Language::registerUserExt(const char* ext, int language) {

  userext2int[usercount].s = ext;
  userext2int[usercount].n = language;
  ++usercount;

  return true;
}

bool Language::registerUserExt(const char* ext, const char* language) {

  int nlanguage = Language::getLanguage(language);
  if (!nlanguage)
    return false;

  return registerUserExt(ext, nlanguage);
}

void reverseString(char * const string, int start, int end)
{
  for(int i = start; i < (start + end) / 2; ++i)
    {
      char temp = string[i];
      string[i] = string[end - ((i + 1) - start)];
      string[end - ((i + 1) - start)] = temp;
    }
}

const char* getLanguageExtension(const char * const inpath)
{
  static char extension[256];

  char path[256];
  strcpy(path, inpath);

  int length = strlen(path);
  reverseString(path, 0, length);

  regex_t preg;
  int errorcode = regcomp(&preg, regex, REG_EXTENDED);

  regmatch_t pmatch[3];
  errorcode = errorcode || regexec(&preg, path, 3, pmatch, 0);

  extension[0] = '\0';
  int matchlength = pmatch[2].rm_eo - pmatch[2].rm_so;
  for(int i = 0; i < matchlength; ++i)
  {
    extension[i] = path[pmatch[2].rm_eo - (i + 1)];
  }
  extension[matchlength] = '\0';

  regfree(&preg);

  return extension;
}

// gets the current language based on the extenstion           
int Language::getLanguageFromFilename(const char* const path) {

  // extract the (pure) extension
  const char* extension = getLanguageExtension(path);

  // custom extensions
  for (const pair * pos = userext2int + usercount - 1; pos->s != 0; --pos) {

    if (strcmp(pos->s, extension) == 0)
      return pos->n;
  }

  return 0;
}
