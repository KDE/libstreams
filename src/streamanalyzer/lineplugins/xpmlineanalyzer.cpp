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

#include "xpmlineanalyzer.h"
#include <strigi/strigiconfig.h>
#include "analysisresult.h"
#include "fieldtypes.h"
#include <cstring>

using namespace std;
using namespace Strigi;

// AnalyzerFactory
void
XpmLineAnalyzerFactory::registerFields(FieldRegister& reg) {
    widthField = reg.registerField("image.width", FieldRegister::integerType, 1, 0);
    heightField = reg.registerField("image.height", FieldRegister::integerType, 1, 0);
    numberOfColorsField = reg.registerField("image.color_depth",
        FieldRegister::integerType, 1, 0);
}

// Analyzer
void
XpmLineAnalyzer::startAnalysis(AnalysisResult* i) {
    analysisResult = i;
    ready = false;
    line = 0;
}
void
XpmLineAnalyzer::handleLine(const char* data, uint32_t length) {
    if (ready) return;
    ++line;
    if (line == 1 && (length < 9 || strncmp(data, "/* XPM */", 9))) {
        // this is not an xpm file
        ready = true;
        return;
    } else if (length == 0 || data[0] != '"') {
        return;
    }
    uint32_t i = 0;
    // we have found the line which should contain the information we want
    ready = true;
    // read the height
    uint32_t propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->addValue(factory->heightField, propertyValue);

    // read the width
    propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->addValue(factory->widthField, propertyValue);

    // read the number of colors
    propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->addValue(factory->numberOfColorsField, propertyValue);
}
bool
XpmLineAnalyzer::isReadyWithStream() {
    return ready;
}

//Factory
class Factory : public AnalyzerFactoryFactory {
public:
    list<StreamLineAnalyzerFactory*>
    streamLineAnalyzerFactories() const {
        list<StreamLineAnalyzerFactory*> af;
        af.push_back(new XpmLineAnalyzerFactory());
        return af;
    }
};

STRIGI_ANALYZER_FACTORY(Factory);
