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

#include "backup.h"
#include "utils.h"

backup::backup(const QString &file, const QString &suffix)
{
  source.setFileName(file);
  copy.setFileName(file + "." + suffix);
}

bool backup::create(bool overwrite)
{
  if (!source.exists() || (exists() && !overwrite))
    return false;

  if (!source.open(QIODevice::ReadWrite))
    return false;
  source.close();

  if (copy.exists())
    copy.remove();

  return source.copy(copy.fileName());
}

bool backup::restore()
{
  if (!exists())
    return false;

  if (source.exists()) {
    if (!source.remove())
      return false;
  }

  return copy.rename(source.fileName());
}

bool backup::exists() const
{
  unsigned long long smod = utils::get_file_mod_time(source.fileName());
  unsigned long long cmod = utils::get_file_mod_time(copy.fileName());

  if (smod != cmod)
    return false;

  return copy.exists();
}

QString backup::name() const
{
  return copy.fileName();
}
