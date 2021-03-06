/*
 * Copyright (c) 2009, 2010 Axel Gembe
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

#include "netscape_win32.h"

netscape_win32_patch::netscape_win32_patch()
{
  init();
}

bool netscape_win32_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor < 1) || ver.major < 10);
}

void netscape_win32_patch::init()
{
  /*
   * This is the test inside the Netscape Flash player plugin that starts
   * the fullscreen window destruction routine when the player loses focus.
   */
  unsigned char pattern[] = {
    0x39, 0x9E, 0x14, 0x04, 0x00, 0x00, /* cmp [esi+addr], ebx */
    0x74, 0x47,                         /* jz short */
    0x53,                               /* push ebx */
    0x8D, 0x45,                         /* mov ecx, esi */
    0xF0, 0x50,
    0x8B, 0xCE,
    0xE8, 0x0A, 0xC3, 0xFF, 0xFF,
  };

  unsigned char pattern_mask[] = {
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0x00,
    0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0xFF, 0xFF,
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { JMP };
  unsigned char patch_mask[] = { 0xFF };

  set_patch(
    6,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("Netscape 10.0 Win32 fullscreen patch");
}

netscape_10_1_win32_patch::netscape_10_1_win32_patch()
{
  init();
}

bool netscape_10_1_win32_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor >= 1) || ver.major > 10);
}

void netscape_10_1_win32_patch::init()
{
  /*
   * This is the test inside the Netscape Flash player that checks for WM_KILLFOCUS
   * in fullscreen.
   */
  unsigned char pattern[] = {
    0x48,             /* dec eax */
    0x74, 0x39,       /* jz short loc_10181650 */
    0x83, 0xE8, 0x07, /* sub eax, 7 */
    0x74, 0x11,       /* jz short loc_1018162D */
    0x83, 0xE8, 0x05, /* sub eax, 5 */
    0x75, 0x13,       /* jnz short loc_10181634 */
  };

  unsigned char pattern_mask[] = {
    0xFF,
    0xFF, 0x00,
    0xFF, 0xFF, 0xFF,
    0xFF, 0x00,
    0xFF, 0xFF, 0xFF,
    0xFF, 0x00,
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { NOP, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF };

  set_patch(
    1,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("Netscape 10.1 Win32 fullscreen patch");
}

netscape_win32_topmost_patch::netscape_win32_topmost_patch()
{
  init();
}

bool netscape_win32_topmost_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor < 4));
}

void netscape_win32_topmost_patch::init()
{
  /*
   * This is the call to SetWindowPos that makes the Flash Window topmost.
   * We don't want to set it to topmost but bring it to the front of the Z-Order instead.
   */
  unsigned char pattern[] = {
    0x52,                               /* push edx */
    0x50,                               /* push eax (eax is zero here) */
    0x51,                               /* push ecx */
    0x6A, 0xFF,                         /* push -1 (this is HWND_TOPMOST) */
    0x53,                               /* push ebx */
    0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, /* call SetWindowPos */
    0x3B, 0x9E, 0x28, 0x10, 0x00, 0x00  /* cmp ebx, [esi+1028h] */
  };

  unsigned char pattern_mask[] = {
    0xFF,
    0xFF,
    0xFF,
    0xFF, 0xFF,
    0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { PUSH_EAX, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF };

  set_patch(
    3,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("Netscape 10.0 Win32 topmost patch");
}

netscape_10_1_win32_topmost_patch::netscape_10_1_win32_topmost_patch()
{
  init();
}

bool netscape_10_1_win32_topmost_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor >= 1) || ver.major > 10);
}

void netscape_10_1_win32_topmost_patch::init()
{
  /*
   * This is the call to SetWindowPos that makes the Flash Window topmost.
   * We don't want to set it to topmost but bring it to the front of the Z-Order instead.
   */
  unsigned char pattern[] = {
    0x50,                               /* push eax */
    0x57,                               /* push edi */
    0x52,                               /* push edx */
    0x6A, 0xFF,                         /* push -1 (this is HWND_TOPMOST) */
    0xFF, 0x75, 0xFC,                   /* push [ebp+hWnd] */
    0x89, 0x46, 0x30,                   /* mov [esi+30h], eax */
    0x89, 0x4E, 0x34,                   /* mov [esi+34h], ecx */
    0xFF, 0x15, 0x5C, 0xD5, 0x42, 0x10, /* call SetWindowPos */
    0x39, 0x5D, 0xFC,                   /* cmp [ebp+hWnd], ebx */
  };

  unsigned char pattern_mask[] = {
    0xFF,                               /* push eax */
    0xFF,                               /* push edi */
    0xFF,                               /* push edx */
    0xFF, 0xFF,                         /* push -1 (this is HWND_TOPMOST) */
    0xFF, 0xFF, 0x00,                   /* push [ebp+hWnd] */
    0xFF, 0xF0, 0x00,                   /* mov [esi+30h], eax */
    0xFF, 0xF0, 0x00,                   /* mov [esi+34h], ecx */
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, /* call SetWindowPos */
    0xFF, 0xFF, 0x00,                   /* cmp [ebp+hWnd], ebx */
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { PUSH_EBX, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF };

  set_patch(
    3,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("Netscape 10.1 Win32 topmost patch");
}
