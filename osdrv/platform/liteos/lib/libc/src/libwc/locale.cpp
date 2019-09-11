/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <errno.h>
#include "locale.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"

// We currently support a single locale, the "C" locale (also known as "POSIX").

static bool __bionic_current_locale_is_utf8 = true;

struct __locale_t {
  size_t mb_cur_max;
};

void setLocaleInit(struct __locale_t *other)
{
    if (other == LC_GLOBAL_LOCALE) {
      other->mb_cur_max = __bionic_current_locale_is_utf8 ? 4 : 1;
    }
}

static bool __is_supported_locale(const char* locale) {
  return (bool)(strcmp(locale, "") == 0 ||
          strcmp(locale, "C") == 0 ||
          strcmp(locale, "C.UTF-8") == 0 ||
          strcmp(locale, "en_US.UTF-8") == 0 ||
          strcmp(locale, "POSIX") == 0);
}

char* setlocale(int category, const char* locale_name) {
  // Is 'category' valid?
  if (category < LC_CTYPE || category > LC_IDENTIFICATION) {
    errno = EINVAL;
    return (char*)NULL;
  }

  // Caller wants to set the locale rather than just query?
  if (locale_name != NULL) {
    if (!__is_supported_locale(locale_name)) {
      // We don't support this locale.
      errno = ENOENT;
      return (char*)NULL;
    }
    __bionic_current_locale_is_utf8 = (strstr(locale_name, "UTF-8") != NULL);
  }

  return (char *)(__bionic_current_locale_is_utf8 ? "C.UTF-8" : "C");
}

size_t __ctype_get_mb_cur_max(void) {
    return 4;
}
