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

#include <shlobj.h>

#endif /* WIN32 */

#include <QFile>

#include "detect.h"

#ifdef WIN32

bool read_reg_string(HKEY root, const QString &path, const QString &name, QString &buf)
{
  /* Open registry */
  HKEY hdl;
  if (RegOpenKey(root, path.toStdWString().c_str(), &hdl) != ERROR_SUCCESS)
    return false;

  DWORD blen = 0, rret;
  PBYTE bbuf = NULL;
  do {
    if (bbuf)
      delete [] bbuf;
    blen += 1024;
    bbuf = new BYTE[blen];
    rret = RegQueryValueEx(hdl, name.toStdWString().c_str(), NULL, NULL, bbuf, &blen);
  } while (rret == ERROR_MORE_DATA);

  RegCloseKey(hdl);

  if (rret != ERROR_SUCCESS) {
    delete [] bbuf;
    return false;
  }

  buf = QString::fromWCharArray((wchar_t*)bbuf);
  delete [] bbuf;

  return true;
}

QStringList get_opera_plugins()
{
  QStringList ret;
  QString buf;

  if (read_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\Opera Software", "Plugin Path", buf)) {
    buf.append("\\NPSWF32.dll");
    if (QFile::exists(buf))
      ret.append(buf);
  }

#if 0
  // TODO: Convert to QString, for now select by hand for Opera USB
  if (read_reg_string(HKEY_CURRENT_USER, "Software\\Opera Software", "Last CommandLine v2", buf)) {
    char *tmp = strrchr(buf, '\\');
    tmp[0] = 0x00;
    strcat(buf, "\\program\\plugins");
    return true;
  }
#endif
  
  return ret;
}

QStringList get_chrome_plugins() {
  QStringList ret;

  wchar_t wappdata[MAX_PATH];
  if (::SHGetSpecialFolderPath(NULL, wappdata, CSIDL_LOCAL_APPDATA, FALSE) == TRUE) {
    QString appdata = QString::fromWCharArray(wappdata);

    QString gcapath = appdata;
    gcapath.append("\\Google\\Chrome\\Application");

    QString findpath = gcapath;
    findpath.append("\\*");

    WIN32_FIND_DATA data;
    HANDLE handle;

    if ((handle = FindFirstFile(findpath.toStdWString().c_str(), &data)) == INVALID_HANDLE_VALUE)
      return ret;
  
    do {
      if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
        QString found = gcapath;
        found.append('\\');
        found.append(QString::fromWCharArray(data.cFileName));
        found.append("\\gcswf32.dll");

        if (QFile::exists(found))
          ret.append(found);
      }
    } while (FindNextFile(handle, &data) != 0);

    FindClose(handle);
  }

  return ret;
}

QStringList get_activex_plugins()
{
  QStringList ret;
  QString buf;

  if (read_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\Macromedia\\FlashPlayerActiveX", "PlayerPath", buf)) {
    if (QFile::exists(buf))
      ret.append(buf);
  }

  return ret;
}

QStringList get_netscape_plugins()
{
  QStringList ret;
  QString buf;

  if (read_reg_string(HKEY_LOCAL_MACHINE, "SOFTWARE\\Macromedia\\FlashPlayerPlugin", "PlayerPath", buf)) {
    if (QFile::exists(buf))
      ret.append(buf);
  }

  return ret;
}

QStringList detect_plugins()
{
  QStringList ret;

  ret.append(get_opera_plugins());
  ret.append(get_chrome_plugins());
  ret.append(get_activex_plugins());
  ret.append(get_netscape_plugins());

  /* Return! :) */
  return ret;
}

#else /* !WIN32 */

QStringList detect_plugins()
{
  QStringList ret;
  return ret;
}

#endif /* WIN32 */
