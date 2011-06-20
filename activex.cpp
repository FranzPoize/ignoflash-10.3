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

#include "activex.h"

activex_patch::activex_patch()
{
  init();
}

bool activex_patch::probe(const utils::fileversion &ver) const
{
  return (ver.target == utils::fileversion::Win32);
}

void activex_patch::init()
{
  /*
   * This is the test inside the ActiveX Flash player plugin that starts
   * the fullscreen window destruction routine when the player loses focus.
   */
  unsigned char pattern[] = {
    0x0F, 0x84, 0xA5, 0x00, 0x00, 0x00, /* jz loc_10141E7E */
    0x48,                               /* dec eax */
    0x0F, 0x84, 0x85, 0x00, 0x00, 0x00, /* jz loc_10141E65 */
    0x83, 0xE8, 0x07,                   /* sub eax, 7 */
    0x74, 0x09,                         /* jz short loc_10141DEE */
    0x83, 0xE8, 0x05,                   /* sub eax, 5 */
  };

  unsigned char pattern_mask[] = {
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF,
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00,
    0xFF, 0x00,
    0xFF, 0xFF, 0x00,
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = {
    NOP, NOP, NOP, NOP, NOP, NOP
  };

  unsigned char patch_mask[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

  set_patch(
    7,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("ActiveX 10.0 / 10.1 fullscreen patch");
}

activex_topmost_patch::activex_topmost_patch()
{
  init();
}

bool activex_topmost_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor < 1) || ver.major < 10);
}

void activex_topmost_patch::init()
{
  /*
   * This is the call to SetWindowPos that makes the Flash Window topmost.
   * We don't want to set it to topmost but bring it to the front of the Z-Order instead.
   */
  unsigned char pattern[] = {
    0x53,                               /* push ebx */
    0xFF, 0xB6, 0xCC, 0x0F, 0x00, 0x00, /* push [esi+FCCh] */
    0xFF, 0xB6, 0xC8, 0x0F, 0x00, 0x00, /* push [esi+FC8h] */
    0xFF, 0xB6, 0xE0, 0x0F, 0x00, 0x00, /* push [esi+FE0h] */
    0xFF, 0xB6, 0xD8, 0x0F, 0x00, 0x00, /* push [esi+FD8h] */
    0x6A, 0xFF,                         /* push -1 (this is HWND_TOPMOST) */
    0x57,                               /* push edi */
    0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, /* call SetWindowPos */
    0x8B, 0xCE                          /* mov ecx, esi */
  };


  unsigned char pattern_mask[] = {
    0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF,
    0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { PUSH_EBX, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF };

  set_patch(
    25,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("ActiveX 10.0 topmost patch");
}

activex_10_1_topmost_patch::activex_10_1_topmost_patch()
{
  init();
}

bool activex_10_1_topmost_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Win32)
    return false;

  return ((ver.major == 10 && ver.minor >= 1) || ver.major > 10);
}

void activex_10_1_topmost_patch::init()
{
  /*
   * This is the call to SetWindowPos that makes the Flash Window topmost.
   * We don't want to set it to topmost but bring it to the front of the Z-Order instead.
   */
  unsigned char pattern[] = {
    0x51,                               /* push ecx */
    0x50,                               /* push eax */
    0x52,                               /* push edx */
    0xFF, 0x75, 0xF8,                   /* push [ebp+X] */
    0x89, 0x46, 0x2C,                   /* mov [esi+2Ch], eax */
    0x6A, 0xFF,                         /* push -1 */
    0x57,                               /* push edi */
    0x89, 0x4E, 0x30,                   /* mov [esi+30h], ecx */
    0xFF, 0x15, 0x08, 0xF6, 0x43, 0x10, /* call SetWindowPos */
    0x8B, 0x46, 0x08,                   /* mov eax, [esi+8] */
  };


  unsigned char pattern_mask[] = {
    0xFF,                               /* push ecx */
    0xFF,                               /* push eax */
    0xFF,                               /* push edx */
    0xFF, 0xFF, 0x00,                   /* push [ebp+X] */
    0xFF, 0xFF, 0x00,                   /* mov [esi+2Ch], eax */
    0xFF, 0xFF,                         /* push -1 */
    0xFF,                               /* push edi */
    0xFF, 0xFF, 0x00,                   /* mov [esi+30h], ecx */
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, /* call SetWindowPos */
    0xFF, 0xFF, 0x00,                   /* mov eax, [esi+8] */
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { PUSH_EBX, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF };

  set_patch(
    9,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("ActiveX 10.1 topmost patch");
}
