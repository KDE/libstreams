#ifndef STREAMTHROUGHANALYZER_H
#define STREAMTHROUGHANALYZER_H

#include "inputstream.h"
#include <map>

class StreamIndexer;

/**
 * This class defines an interface for analyzing streams.
 * This interface is not yet complete, it lacks good functions for retrieving
 * the results in other ways than just printing them.
 */
class StreamThroughAnalyzer {
public:
    virtual ~StreamThroughAnalyzer() {};
    /**
     * This function sets up the analyzer for handling a stream.
     * The stream \p in is used in constructing a new internal
     * inputstream that is returned by this function. Every time
     * a read call is performed on this class, the data read
     * is used in the analysis before being passed on to the
     * caller.
     **/
    virtual jstreams::InputStream *connectInputStream(jstreams::InputStream *in) = 0;
    virtual const std::multimap<std::wstring, std::wstring> &getResults() = 0;
};

#endif
