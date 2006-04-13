#ifndef ZIPINPUTSTREAM_H
#define ZIPINPUTSTREAM_H

#include "substreamprovider.h"
#include "gzipinputstream.h"

/**
 * Partial implementation of the zip file format according to
 * http://www.pkware.com/business_and_developers/developer/popups/appnote.txt
 * 99% of zip files on my system can be read with this class.
 * Exceptions are files that are (at least)
 * - files generated by writing to stdout
 * - files using other compression as deflated
 * - encrypted files
 **/

namespace jstreams {

class ZipInputStream : public SubStreamProvider {
private:
    // information relating to the current entry
    SubInputStream *compressedEntryStream;
    GZipInputStream *uncompressionStream;
    SubInputStream *uncompressedEntryStream;
    int32_t entryCompressedSize;
    int32_t compressionMethod;

    void readFileName(int32_t len);
    void readHeader();
    static int32_t read2bytes(const unsigned char *b);
    static int32_t read4bytes(const unsigned char *b);
public:
    ZipInputStream(InputStream *input);
    ~ZipInputStream();
    SubInputStream* nextEntry();
    static bool checkHeader(const char* data, int32_t datasize);
};

} // end namespace jstreams

#endif
