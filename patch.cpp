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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")

#endif /* WIN32 */

#include <QFile>

#include "patch.h"

namespace patch {
  QString get_status_string(status stat)
  {
    switch (stat) {
      case ok:
        return QObject::tr("Ok");
      case error_open:
        return QObject::tr("Failed to open the input file for read/write access");
      case error_mapping:
        return QObject::tr("Failed to map the input file to memory");
      case error_patchloc:
        return QObject::tr("Failed to find the patch location");
      case error_checksum:
        return QObject::tr("Failed to update the PE header checksum");
      default:
        return QString();
    }
  }

  base::base()
  {
    patch_offset = 0;
  }

  bool base::probe(const utils::fileversion &ver) const
  {
    return true;
  }

  status base::patch(const QString &file, const utils::fileversion &ver)
  {
    QFile f(file);

    if (!f.open(QIODevice::ReadWrite))
      return error_open;

    unsigned char *mem = f.map(0, f.size());
    if (!mem)
      return error_mapping;

    status ret = patch(mem, f.size());

#ifdef WIN32
    if (ret == ok && (ver.target == utils::fileversion::Win32 || ver.target == utils::fileversion::Win64)) {
      /* Update the PE checksum on Win32 */
      DWORD sum, sum_new;
      PIMAGE_NT_HEADERS pe_hdr;
      pe_hdr = ::CheckSumMappedFile(mem, f.size(), &sum, &sum_new);
      if (!pe_hdr)
        return error_checksum;
      pe_hdr->OptionalHeader.CheckSum = sum_new;
    }
#endif /* WIN32 */

    f.unmap(mem);

    return ret;
  }

  status base::patch(unsigned char *data, size_t len)
  {
    size_t start;

    /* Search for the start location using the pattern */
    if (!search(data, len, start))
      return error_patchloc;

    size_t idx, patch_len = patch_data.size();
    unsigned char mask;
    
    /* Apply the patch */
    unsigned char *loc = data + start + patch_offset;
    for (idx = 0; idx < patch_len; idx++, loc++) {
      mask = patch_mask[idx];
      *loc = (patch_data[idx] & mask) | (*loc & ~mask);
    }

    return ok;
  }
  
  QString base::get_name() const
  {
    return name;
  }

  void base::set_pattern(utils::buffer data, utils::buffer mask)
  {
    pattern_data = data;
    pattern_mask = mask;
  }

  void base::set_patch(size_t offset, utils::buffer data, utils::buffer mask)
  {
    patch_offset = offset;
    patch_data = data;
    patch_mask = mask;
  }

  void base::set_name(const QString& name)
  {
    this->name = name;
  }

  bool base::search(unsigned char *data, size_t len, size_t &start)
  {
    size_t pattern_len = pattern_data.size();
    size_t end = len - pattern_len;
    size_t pat_idx;
    unsigned char mask;

    for (start = 0; start < end; start++) {
      for (pat_idx = 0; pat_idx < pattern_len; pat_idx++) {
        mask = pattern_mask[pat_idx];

        if ((data[start + pat_idx] & mask) != (pattern_data[pat_idx] & mask))
          break;
      }

      if (pat_idx == pattern_len)
        return true;
    }

    return false;
  }
};
