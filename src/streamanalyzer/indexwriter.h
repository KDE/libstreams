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
#ifndef INDEXWRITER_H
#define INDEXWRITER_H

#include <string>
#include <vector>
#include "jstreamsconfig.h"
#include "streamanalyzer.h"

namespace Strigi {

template <class T> class StreamBase;
class FieldRegister;

/*
- create indexwriter
for all streams {
 - create an indexable
 - add the indexwriter to it
 - add a stream to the indexable (optional)
 - add fields to indexable (optional)
 - delete the indexable
}
- delete the indexwriter
*/
class AnalysisResult;
class RegisteredField;
/**
 * Abstract class that provides write access to a Strigi index.
 *
 * Instances of the class should be obtained by calling the function
 * IndexManager::indexWriter() and should not be used from threads other
 * than the thread that called IndexManager::indexWriter().
 **/
class STREAMANALYZER_EXPORT IndexWriter {
friend class AnalysisResult;
protected:
    virtual void startAnalysis(AnalysisResult*) = 0;
    virtual void addText(const AnalysisResult*, const char* text, int32_t length)=0;
    virtual void addField(const AnalysisResult*, const RegisteredField* field,
        const std::string& value) = 0;
    virtual void addField(const AnalysisResult*, const RegisteredField* field,
        const unsigned char* data, uint32_t size) = 0;
    virtual void addField(const AnalysisResult*, const RegisteredField* field,
        int32_t value) = 0;
    virtual void addField(const AnalysisResult*, const RegisteredField* field,
        uint32_t value) = 0;
    virtual void addField(const AnalysisResult*, const RegisteredField* field,
        double value) = 0;
    virtual void addValue(const AnalysisResult*, const RegisteredField* field,
        const std::string& name, const std::string& value) = 0;
    virtual void finishAnalysis(const AnalysisResult*) = 0;
    /**
     * Add a complete RDF triplet
     *
     * @param subject
     * @param predicate
     * @param object
     **/
    virtual void addTriplet(const std::string& subject,
        const std::string& predicate, const std::string& object) = 0;
public:
    virtual ~IndexWriter() {}
    /**
     * Flush the accumulated changes to disk.
     **/
    virtual void commit() { return; }
    /**
     * Delete the entries with the given paths from the index.
     *
     * @param entries the paths of the files that should be deleted
     **/
    virtual void deleteEntries(const std::vector<std::string>& entries) = 0;
    /**
     * Delete all indexed documents from the index.
     **/
    virtual void deleteAllEntries() = 0;
    /**
     * Return the number of objects that are currently in the cache.
     **/
    virtual int itemsInCache() { return 0; }
    /**
     * Optimize the index. This can be computationally intensive and can
     * will often cause the index to temporarily use the double amount of
     * diskspace.
     **/
    virtual void optimize() {}
    virtual void initWriterData(const Strigi::FieldRegister&) {}
    virtual void releaseWriterData(const Strigi::FieldRegister&) {}
};
}

#endif
