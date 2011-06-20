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

#include "utils.h"

/* x86 NOP Opcode (0x90) - one of my best friends :) */
#define NOP 0x90

/* x86 JMP Opcode (0xEB) */
#define JMP 0xEB

/* x86 PUSH EAX Opcode (0x50) */
#define PUSH_EAX 0x50

/* x86 PUSH EBX Opcode (0x53) */
#define PUSH_EBX 0x53

namespace patch {
  enum status {
    ok = 0,
    error_open,
    error_mapping,
    error_patchloc,
    error_checksum
  };

  QString get_status_string(status stat);

  class base
  {
  public:
    base();

    virtual bool probe(const utils::fileversion &ver) const;

    status patch(const QString &file, const utils::fileversion &ver);
    status patch(unsigned char *data, size_t len);

    QString get_name() const;

  protected:
    virtual void init() = 0;

    void set_pattern(utils::buffer data, utils::buffer mask);
    void set_patch(size_t offset, utils::buffer data, utils::buffer mask);
    void set_name(const QString& name);

  private:
    base(const base&);
    base& operator=(const base&);

    bool search(unsigned char *data, size_t len, size_t &start);

    utils::buffer pattern_data;
    utils::buffer pattern_mask;

    size_t patch_offset;
    utils::buffer patch_data;
    utils::buffer patch_mask;

    QString name;
  };
};
