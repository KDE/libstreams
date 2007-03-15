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
#ifndef PNGENDANALYZER
#define PNGENDANALYZER

#include "streamendanalyzer.h"

class PngEndAnalyzerFactory;
class PngEndAnalyzer : public Strigi::StreamEndAnalyzer {
private:
    const PngEndAnalyzerFactory* factory;
    bool checkHeader(const char* header, int32_t headersize) const;
    char analyze(Strigi::AnalysisResult& idx, jstreams::InputStream* in);
    const char* getName() const { return "PngEndAnalyzer"; }
public:
    PngEndAnalyzer(const PngEndAnalyzerFactory* f) :factory(f) {}
};

class PngEndAnalyzerFactory : public Strigi::StreamEndAnalyzerFactory {
friend class PngEndAnalyzer;
private:
    static const cnstr widthFieldName;
    static const cnstr heightFieldName;
    const Strigi::RegisteredField* widthField;
    const Strigi::RegisteredField* heightField;
    const char* getName() const {
        return "PngEndAnalyzer";
    }
    Strigi::StreamEndAnalyzer* newInstance() const {
        return new PngEndAnalyzer(this);
    }
    void registerFields(Strigi::FieldRegister&);
};

#endif