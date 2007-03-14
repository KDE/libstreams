#include "jstreamsconfig.h"
#include "cluceneindexmanager.h"
#include "indexmanagertests.h"
#include "indexwritertests.h"
#include "indexreadertests.h"
#include "analyzerconfiguration.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#endif

StrigiMutex errorlock;
int founderrors = 0;
int
CLuceneTest(int argc, char*argv[]) {
    const char* path = "testcluceneindex";

    // initialize a directory for writing and an indexmanager
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, S_IRUSR|S_IWUSR|S_IXUSR);
#endif
    jstreams::IndexManager* manager = createCLuceneIndexManager(path);

    jstreams::AnalyzerConfiguration ic;
    IndexManagerTests tests(manager, ic);
    tests.testAll();
    tests.testAllInThreads(20);

    jstreams::IndexWriter* writer = manager->getIndexWriter();
    IndexWriterTests wtests(*writer, ic);
    wtests.testAll();

    jstreams::IndexReader* reader = manager->getIndexReader();
    IndexReaderTests rtests(reader);
    rtests.testAll();

    // close and clean up the manager
    delete manager;

    // clean up data
    std::string cmd = "rm -r ";
    cmd += path;
    system(cmd.c_str());

    return founderrors;
}
