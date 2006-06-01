#include "estraierindexreader.h"
#include "estraierindexmanager.h"
#include <estraier.h>
#include <set>
#include <sstream>
#include <assert.h>
using namespace std;

EstraierIndexReader::EstraierIndexReader(EstraierIndexManager* m, ESTDB* d)
    : manager(m), db(d) {
}
EstraierIndexReader::~EstraierIndexReader() {
}
vector<string>
EstraierIndexReader::query(const std::string& query) {
    ESTCOND* cond = est_cond_new();
    est_cond_set_phrase(cond, query.c_str());
    est_cond_set_max(cond, 100);
    int n;
    int* ids;

    manager->ref();
    ids = est_db_search(db, cond, &n, NULL);
    
    std::vector<std::string> results;
    for (int i=0; i<n; ++i) {
        char* uri = est_db_get_doc_attr(db, ids[i], "@uri");
        if (uri) {
            results.push_back(uri);
            free(uri);
        }
    }
    manager->deref();

    // clean up
    est_cond_delete(cond);
    free(ids);
    return results;
}
map<string, time_t>
EstraierIndexReader::getFiles(char depth) {
    map<string, time_t> files;
    ESTCOND* cond = est_cond_new();
    string q = "depth NUMEQ 0";
    est_cond_add_attr(cond, q.c_str());
    int n;
    int* ids;

    manager->ref();
    ids = est_db_search(db, cond, &n, NULL);
    
    for (int i=0; i<n; ++i) {
        char* uri = est_db_get_doc_attr(db, ids[i], "@uri");
        char* mdate = est_db_get_doc_attr(db, ids[i], "@mdate");
        time_t md = atoi(mdate);
        assert(uri);
        files[uri] = md;
        free(uri);
    }
    manager->deref();

    // clean up
    est_cond_delete(cond);
    free(ids);
    return files;
}
int
EstraierIndexReader::countDocuments() {
    manager->ref();
    int count = est_db_doc_num(db);
    manager->deref();
    return count;
}
