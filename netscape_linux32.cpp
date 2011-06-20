/*
 * Copyright (c) 2010 Kevin Crossan
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

#include "netscape_linux32.h"

netscape_10_1_linux32_patch::netscape_10_1_linux32_patch()
{
  init();
}

bool netscape_10_1_linux32_patch::probe(const utils::fileversion &ver) const
{
  if (ver.target != utils::fileversion::Linux32)
    return false;

  return ((ver.major == 10 && ver.minor >= 1) || ver.major > 10);
}

void netscape_10_1_linux32_patch::init()
{
  /*
   * This is the function call inside the Netscape Flash player that sets the
   * GDK event mask to include GDK_PROPERTY_CHANGE_MASK, which causes X window
   * property changes to be reported.
   */
  unsigned char pattern[] = {
    0x89, 0x34, 0x24,                /* mov [esp], esi */
    0xE8, 0xC6, 0x1E, 0xFF, 0xFF,    /* call _gdk_window_get_events */
    0x89, 0x34, 0x24,                /* mov [esp], esi */
    0x0D, 0x00, 0x00, 0x01, 0x00,    /* or eax, 10000h */
    0x89, 0x44, 0x24, 0x04,          /* mov [esp+4], eax */
    0xE8, 0x25, 0x21, 0xFF, 0xFF,    /* call _gdk_window_set_events */
  };

  unsigned char pattern_mask[] = {
    0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0xFF,
  };

  set_pattern(
    utils::buffer(pattern, pattern + sizeof(pattern)),
    utils::buffer(pattern_mask, pattern_mask + sizeof(pattern_mask))
    );

  unsigned char patch[] = { NOP, NOP, NOP, NOP, NOP };
  unsigned char patch_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  set_patch(
    20,
    utils::buffer(patch, patch + sizeof(patch)),
    utils::buffer(patch_mask, patch_mask + sizeof(patch_mask))
    );

  set_name("Netscape 10.1.102/10.2 Linux32 fullscreen patch");
}
