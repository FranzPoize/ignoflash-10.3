/*
 * Copyright (c) 2010 Axel Gembe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <QString>

#include <vector>

namespace utils {

  class fileversion
  {
  public:
    enum filetarget {
      Unknown = 0,
      Win32,
      Win64,
      Linux32,
      Linux64,
      OSX32,
      OSX64
    };

    fileversion();
    fileversion(int ma, int mi, int b, int r, filetarget t);

    QString str() const;

    int major;
    int minor;
    int build;
    int revision;
    filetarget target;
  };

  wchar_t* new_qtowc(const QString& buf);
  void free_qtowc(wchar_t* buf);
  unsigned long long get_file_mod_time(const QString& path);
  bool get_file_version(const QString& path, fileversion &fv);
  bool get_flash_file_version(const QString& path, fileversion &fv);
  bool remove_acls(const QString& path);
  typedef std::vector<unsigned char> buffer;
}
