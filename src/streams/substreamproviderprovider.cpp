/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "jstreamsconfig.h"
#include "substreamproviderprovider.h"
#include "mailinputstream.h"
#include "tarinputstream.h"
#include "zipinputstream.h"
using namespace jstreams;

SubStreamProviderProvider::SubStreamProviderProvider() {
}
SubStreamProvider*
SubStreamProviderProvider::getSubStreamProvider(StreamBase<char>* input) {
    // read the header
    const char* header;
    int64_t pos = input->getPosition();
    int32_t headersize = input->read(header, 1024, 0);
    input->reset(pos);
    if (headersize <= 0) {
        return 0;
    }
    printf("%x%x%x%x\n", header[0], header[1], header[2], header[3]);
    if (MailInputStream::checkHeader(header, headersize)) {
        return new MailInputStream(input);
    }
    if (ZipInputStream::checkHeader(header, headersize)) {
        printf("zip!\n");
        return new ZipInputStream(input);
    }
    printf("no match %i\n", headersize);
    return 0;
}
