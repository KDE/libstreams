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

#ifndef EVENTTHROUGHANALYZER
#define EVENTTHROUGHANALYZER

#include "jstreamsconfig.h"
#include "dataeventinputstream.h"
#include "streamthroughanalyzer.h"
#include <vector>

namespace Strigi {

class StreamEventAnalyzerFactory;
class StreamSaxAnalyzerFactory;
class StreamLineAnalyzerFactory;
class StreamEventAnalyzer;
class StreamSaxAnalyzer;
class StreamLineAnalyzer;

class EventThroughAnalyzer : public StreamThroughAnalyzer,
        public jstreams::DataEventHandler {
private:
    std::vector<StreamEventAnalyzer*> event;
    jstreams::DataEventInputStream* datastream;
    AnalysisResult* result;
    bool ready;

    void setIndexable(AnalysisResult*);
    jstreams::InputStream* connectInputStream(jstreams::InputStream* in);
    bool isReadyWithStream();
    bool handleData(const char* data, uint32_t size);
    void handleEnd();
public:
    EventThroughAnalyzer(std::vector<StreamEventAnalyzer*>& e)
            : event(e), datastream(0), result(0), ready(true){}
    ~EventThroughAnalyzer();
};
class EventThroughAnalyzerFactory : public StreamThroughAnalyzerFactory {
    std::vector<StreamSaxAnalyzerFactory*>& saxfactories;
    std::vector<StreamLineAnalyzerFactory*>& linefactories;
    std::vector<StreamEventAnalyzerFactory*>& eventfactories;

    const char* getName() const { return "EventThroughAnalyzerFactory"; }
    void registerFields(Strigi::FieldRegister&);
public:
    EventThroughAnalyzerFactory(std::vector<StreamSaxAnalyzerFactory*>& s,
                std::vector<StreamLineAnalyzerFactory*>& l,
                std::vector<StreamEventAnalyzerFactory*>& e)
            : saxfactories(s), linefactories(l), eventfactories(e) {}
    StreamThroughAnalyzer* newInstance() const;
};

}

#endif