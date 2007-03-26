/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Vincent Ricard <magic@magicninja.org>
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
#include "tgathroughanalyzer.h"
#include "analysisresult.h"
#include "fieldtypes.h"
#include "textutils.h"

using namespace std;
using namespace jstreams;
using namespace Strigi;

// AnalyzerFactory
const string TgaThroughAnalyzerFactory::compressionFieldName("compression");
const string TgaThroughAnalyzerFactory::colorDepthFieldName("colorDepth");
const string TgaThroughAnalyzerFactory::colorModeFieldName("colorMode");
const string TgaThroughAnalyzerFactory::widthFieldName("width");
const string TgaThroughAnalyzerFactory::heightFieldName("height");

void
TgaThroughAnalyzerFactory::registerFields(FieldRegister& reg) {
    compressionField = reg.registerField(compressionFieldName,
        FieldRegister::stringType, 1, 0);
    colorDepthField = reg.registerField(colorDepthFieldName,
        FieldRegister::integerType, 1, 0);
    colorModeField = reg.registerField(colorModeFieldName,
        FieldRegister::stringType, 1, 0);
    widthField = reg.registerField(widthFieldName,
        FieldRegister::integerType, 1, 0);
    heightField = reg.registerField(heightFieldName,
        FieldRegister::integerType, 1, 0);
}

// Analyzer
void
TgaThroughAnalyzer::setIndexable(AnalysisResult* i) {
    analysisResult = i;
}

InputStream*
TgaThroughAnalyzer::connectInputStream(InputStream* in) {
    const int32_t nreq = 18;
    const char* buf;
    int32_t nread = in->read(buf, nreq, -1);
    in->reset(0);

    if (nread < nreq) {
        return in;
    }

    uint8_t imagetype = *(buf+2);;
    switch (imagetype) {
    case 1 :
    case 9 :
    case 32 :
        analysisResult->setField(factory->colorModeField, "Color-Mapped");
        break;
    case 2 :
    case 10 :
    case 33 :
        analysisResult->setField(factory->colorModeField, "RGB");
        break;
    case 3 :
    case 11 :
        analysisResult->setField(factory->colorModeField, "Black and White");
        break;
    default :
        analysisResult->setField(factory->colorModeField, "Unknown");
    }

    switch (imagetype) {
    case 1 :
    case 2 :
    case 3 :
        analysisResult->setField(factory->compressionField, "None");
        break;
    case 9 :
    case 10 :
    case 11 :
        analysisResult->setField(factory->compressionField, "RLE");
        break;
    case 32 :
        analysisResult->setField(factory->compressionField, "Huffman, Delta & RLE");
        break;
    case 33 :
        analysisResult->setField(factory->compressionField, "Huffman, Delta, RLE (4-pass quadtree)");
        break;
    default :
        analysisResult->setField(factory->compressionField, "Unknown");
    };

    uint16_t width = readLittleEndianUInt16(buf+12);
    uint16_t height = readLittleEndianUInt16(buf+14);
    analysisResult->setField(factory->widthField, width);
    analysisResult->setField(factory->heightField, height);

    uint8_t colorDepth = *(buf+16);
    analysisResult->setField(factory->colorDepthField, colorDepth);

    return in;
}

bool
TgaThroughAnalyzer::isReadyWithStream() {
    return true;
}

//Factory
class Factory : public AnalyzerFactoryFactory {
public:
    list<StreamThroughAnalyzerFactory*>
    getStreamThroughAnalyzerFactories() const {
        list<StreamThroughAnalyzerFactory*> af;
        af.push_back(new TgaThroughAnalyzerFactory());
        return af;
    }
};

STRIGI_ANALYZER_FACTORY(Factory)