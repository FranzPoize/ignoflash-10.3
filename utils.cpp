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

#include <accctrl.h>
#include <aclapi.h>
#pragma comment(lib, "advapi32.lib")

#endif /* WIN32 */

#include <QDir>
#include <QCryptographicHash>
#include <QByteArray>
#include <QHash>

#include "utils.h"

namespace utils {
  fileversion::fileversion() :
    major(0), minor(0), build(0), revision(0), target(Unknown)
  {
  }

  fileversion::fileversion(int ma, int mi, int b, int r, filetarget t) :
    major(ma), minor(mi), build(b), revision(r), target(t)
  {
  }

  QString fileversion::str() const
  {
    QString ret;
    
    ret.sprintf("%d.%d.%d.%d", major, minor, build, revision);

    switch (target) {
      default:
      case Unknown:
        ret.append(QObject::tr(" Unknown"));
        break;
      case Win32:
        ret.append(QObject::tr(" Windows 32-bit"));
        break;
      case Win64:
        ret.append(QObject::tr(" Windows 64-bit"));
        break;
      case Linux32:
        ret.append(QObject::tr(" Linux 32-bit"));
        break;
      case Linux64:
        ret.append(QObject::tr(" Linux 64-bit"));
        break;
      case OSX32:
        ret.append(QObject::tr(" MacOS X 32-bit"));
        break;
      case OSX64:
        ret.append(QObject::tr(" MacOS X 64-bit"));
        break;
    }

    return ret;
  }

  wchar_t* new_qtowc(const QString& buf)
  {
    /* Allocate the buffer for the wide char copy */
    wchar_t *ret = new wchar_t[buf.length() + 1];

    /* Zero the buffer */
    memset(ret, 0, (buf.length() + 1) * sizeof(wchar_t));

    /* Convert the QString to wchar */
    buf.toWCharArray(ret);

    return ret;
  }

  void free_qtowc(wchar_t* buf)
  {
    /* Free the buffer */
    delete [] buf;
  }

  unsigned long long get_file_mod_time(const QString& path)
  {
#ifdef WIN32
    FILETIME ft;
    HANDLE f;

    /* Create a non-const copy of the path as wchar */
    wchar_t *wpath = new_qtowc(QDir::toNativeSeparators(path));

    f = ::CreateFile(wpath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (f == INVALID_HANDLE_VALUE)
      return 0ULL;

    if (::GetFileTime(f, NULL, NULL, &ft) == FALSE)
      return 0ULL;

    ::CloseHandle(f);

    /* Release the buffer */
    free_qtowc(wpath);

    /* Return Filetime as 64 bit */
    return ((unsigned long long)ft.dwHighDateTime << 32) |
      (unsigned long long)ft.dwLowDateTime;
#endif /* WIN32 */

    return 0ULL;
  }

  bool get_file_version(const QString& path, fileversion &fv)
  {
#ifdef WIN32
    bool ret = false;
    wchar_t *wpath = new_qtowc(QDir::toNativeSeparators(path));
    QFile f(path);

    DWORD dlen = ::GetFileVersionInfoSize(wpath, NULL);
    if (!dlen)
      goto gfv_exit;

    char *buf = new char[dlen];
    if (!buf)
      goto gfv_exit;

    if (!::GetFileVersionInfo(wpath, 0, dlen, buf))
      goto gfv_exit_malloc;

    VS_FIXEDFILEINFO *fi;
    UINT fi_len;
    if (!::VerQueryValue(buf, L"\\", (LPVOID*)&fi, &fi_len))
      goto gfv_exit_malloc;

    fv.major = HIWORD(fi->dwFileVersionMS);
    fv.minor = LOWORD(fi->dwFileVersionMS);
    fv.build = HIWORD(fi->dwFileVersionLS);
    fv.revision = LOWORD(fi->dwFileVersionLS);

    /*
     * Now all thats left is trying to determine whether the file is 32 or
     * 64 bit. For this we have to get the IMAGE_NT_HEADERS which contains
     * the information.
     */
    if (!f.open(QIODevice::ReadOnly))
      goto gfv_exit_malloc;

    size_t len = f.size();
    unsigned char *mem = f.map(0, len);
    if (!mem)
      goto gfv_exit_malloc;

    PIMAGE_DOS_HEADER hdos = (PIMAGE_DOS_HEADER)mem;
    if (hdos->e_magic != IMAGE_DOS_SIGNATURE)
      goto gfv_exit_malloc;

    PIMAGE_NT_HEADERS hnt = (PIMAGE_NT_HEADERS)(mem + hdos->e_lfanew);
    if (hnt->Signature != IMAGE_NT_SIGNATURE)
      goto gfv_exit_malloc;

    switch (hnt->FileHeader.Machine) {
      default:
      case IMAGE_FILE_MACHINE_I386:
        fv.target = fileversion::Win32;
        break;
      case IMAGE_FILE_MACHINE_AMD64:
      case IMAGE_FILE_MACHINE_IA64:
        fv.target = fileversion::Win64;
        break;
    }
    
    f.unmap(mem);

    ret = true;

gfv_exit_malloc:
    delete [] buf;

gfv_exit:
    free_qtowc(wpath);
    
    return ret;

#else /* !WIN32 */
    return false;
#endif /* WIN32 */
  }

  bool get_flash_file_version(const QString& path, fileversion &fv)
  {
    /* Try to get the version from the file headers */
    if (get_file_version(path, fv))
      return true;

    /*
     * Failed to get info from header, compute the
     * checksum and look up the file version.
     */
    QFile f(path);

    if (!f.open(QIODevice::ReadOnly))
      return false;

    size_t len = f.size();
    unsigned char *mem = f.map(0, len);
    if (!mem)
      return false;

    QByteArray qba((char*)mem, len);

    QByteArray hash = QCryptographicHash::hash(qba, QCryptographicHash::Md5);

    f.unmap(mem);

    {
      static QHash<QByteArray, fileversion> vertable;
      static bool vertable_inited = false;

      if (!vertable_inited) {
        /*
         * Method to get the linux flash player version without running it:
         * strings libflashplayer.so | grep LNX
         */
        vertable.insert(QByteArray("\x8a\x10\xf0\x63\x21\x52\x19\xa9\x8f\x7d\xbe\x6d\xd3\x36\x67\x1e", 16), fileversion( 10, 1, 102, 65, fileversion::Linux32 ));
        vertable.insert(QByteArray("\xb9\x95\xbf\x05\xfc\x10\x56\x8c\x06\xd3\x0b\x0d\x5d\x59\xab\xa1", 16), fileversion( 10, 2, 151, 49, fileversion::Linux32 ));
        vertable.insert(QByteArray("\x3d\x13\xa0\x70\x9b\x9a\x93\x7a\xa0\xd2\x88\x83\xb4\x6d\xeb\xb9", 16), fileversion( 10, 3, 181, 26, fileversion::Linux32 ));
        vertable.insert(QByteArray("\x26\x7b\xfd\xb3\x8d\x14\xc9\xd9\x6d\x0d\x04\xe2\x73\xc3\xd9\x61", 16), fileversion( 10, 3, 162, 29, fileversion::Linux64 ));
        vertable_inited = true;
      }

      if (vertable.contains(hash)) {
        fv = vertable.value(hash);
        return true;
      }
    }

    return false;
  }

  bool remove_acls(const QString& path)
  {
#ifdef WIN32
    /*
     * This fixes the file permissions on the ActiveX plugin.
     * Damn you, Flash! Do I REALLY have to do this ?
     */

    PACL acl = NULL;
    PSECURITY_DESCRIPTOR sd = NULL;

    /* Create a non-const copy of the path as wchar */
    wchar_t *wpath = new_qtowc(path);

    /* Get the current ACL */
    DWORD res = ::GetNamedSecurityInfo(wpath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &acl, NULL, &sd);
    if (res != ERROR_SUCCESS)
      return false;

    /* Delete all ACEs, object will keep inherited ACEs. */
    while (::DeleteAce(acl, 0) != 0);

    /* Set the new ACL */
    if (::SetNamedSecurityInfo(wpath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, acl, NULL) != ERROR_SUCCESS)
      return false;

    /* Free the security descriptor */
    ::LocalFree((HLOCAL)sd);

    /* Reset Read-Only attribute */
    ::SetFileAttributes(wpath, FILE_ATTRIBUTE_NORMAL);

    /* Release the buffer */
    free_qtowc(wpath);
#endif /* WIN32 */

    /* Success! */
    return true;
  }
};
