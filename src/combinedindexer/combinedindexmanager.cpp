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
#include "combinedindexmanager.h"
#include "jstreamsconfig.h"

#ifdef HAVE_CLUCENE
#include "cluceneindexmanager.h"
#endif
#ifdef HAVE_XAPIAN
#include "xapianindexmanager.h"
#endif
#ifdef HAVE_ESTRAIER
#include "estraierindexmanager.h"
#endif
#ifdef HAVE_SQLITE
#include "sqliteindexmanager.h"
#endif

#include "query.h"
#include "indexeddocument.h"
#include "indexreader.h"
#include <string>
#include <map>
using namespace std;
using namespace jstreams;

map<string, IndexManager*(*)(const char*)>
CombinedIndexManager::getFactories() {
    map<string, IndexManager*(*)(const char*)> factories;
#ifdef HAVE_ESTRAIER
    factories["estraier"] = createEstraierIndexManager;
#endif
#ifdef HAVE_CLUCENE
    factories["clucene"] = createCLuceneIndexManager;
#endif
#ifdef HAVE_XAPIAN
    factories["xapian"] = createXapianIndexManager;
#endif
#ifdef HAVE_SQLITE
    factories["sqlite"] = createSqliteIndexManager;
#endif
    return factories;
}

vector<string>
CombinedIndexManager::getBackEnds() {
    vector<string> v;
    map<string, IndexManager*(*)(const char*)> f = getFactories();
    map<string, IndexManager*(*)(const char*)>::const_iterator i;
    for (i=f.begin(); i!=f.end(); ++i) {
        v.push_back(i->first);
    }
    return v;
}

// thread safe smart pointer
template <class T> 
class TSSPtr {
private:
    struct P {
        StrigiMutex lock;
        T* p;
        int count;
    };
    P* p;
public:
    TSSPtr(T* ptr = 0) {
        if (ptr) {
            p = new P();
            p->p = ptr;
            p->count = 1;
        } else {
            p = 0;
        }
    }
    TSSPtr(const TSSPtr& t) {
        set(t);
    }
    ~TSSPtr() {
        deref();
    }
    void operator=(const TSSPtr& t) {
        if (t.p == p) return;
        deref();
        set(t);
    }
    T* operator->() const {
        return (p) ?p->p :0;
    }
private:
    void deref() {
        if (p) {
            STRIGI_MUTEX_LOCK(&p->lock.lock);
            int c = --(p->count);
            STRIGI_MUTEX_UNLOCK(&p->lock.lock);
            if (c == 0) {
                delete p->p;
                delete p;
            }
            p = 0;
        }
    }
    void set(const TSSPtr& t) {
        if (t.p) {
            STRIGI_MUTEX_LOCK(&t.p->lock.lock);
            t.p->count++;
            STRIGI_MUTEX_UNLOCK(&t.p->lock.lock);
            p = t.p;
        } else {
            p = 0;
        }
    }
};

TSSPtr<int> a(0);

class CombinedIndexReader : public IndexReader {
private:
    CombinedIndexManager* m;
public:
    CombinedIndexReader(CombinedIndexManager* manager) :m(manager){}
    int32_t countHits(const Query& query);
    vector<IndexedDocument> query(const Query& q);
    map<string, time_t> getFiles(char depth);
    int32_t countDocuments();
    int32_t countWords();
    int64_t getIndexSize();
    int64_t getDocumentId(const string& uri);
    time_t getMTime(int64_t docid);
};

class CombinedIndexManager::Private {
public:
    StrigiMutex lock;
    CombinedIndexReader reader;
    string writermanagerdir;
    IndexManager* writermanager;
    map<string, TSSPtr<IndexManager> > readmanagers;

    Private(CombinedIndexManager* m);
    ~Private();
};
CombinedIndexManager::Private::Private(CombinedIndexManager* m) :reader(m) {
    writermanager = 0;
}
CombinedIndexManager::Private::~Private() {
}

CombinedIndexManager::CombinedIndexManager(const string& type,
        const string& indexdir) :p(new CombinedIndexManager::Private(this)) {
    // determine the right index manager
    map<string, IndexManager*(*)(const char*)> factories = getFactories();
    map<string, IndexManager*(*)(const char*)>::const_iterator f
        = factories.find(type);
    if (f == factories.end()) {
        f = factories.begin();
    }
    p->writermanager = f->second(indexdir.c_str());
}
CombinedIndexManager::~CombinedIndexManager() {
    delete p;
}
IndexReader*
CombinedIndexManager::getIndexReader() {
    return &(p->reader);
}
IndexWriter*
CombinedIndexManager::getIndexWriter() {
    return p->writermanager->getIndexWriter();
}
void
CombinedIndexManager::addReadonlyIndex(const string& indexdir,
        const string& type) {
}
void
CombinedIndexManager::removeReadonlyIndex(const string& indexdir) {
}
int32_t
CombinedIndexReader::countHits(const Query& query) {
    int32_t c = m->p->writermanager->getIndexReader()->countHits(query);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->getIndexReader()->countHits(query);
    }
    return c;
}
vector<IndexedDocument>
CombinedIndexReader::query(const Query& q) {
    vector<IndexedDocument> v = m->p->writermanager->getIndexReader()
        ->query(q);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        v = i->second->getIndexReader()->query(q);
    }
    return v;
}
map<string, time_t>
CombinedIndexReader::getFiles(char depth) {
    map<string, time_t> files = m->p->writermanager->getIndexReader()
        ->getFiles(depth);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        files = i->second->getIndexReader()->getFiles(depth);
    }
    return files;
}
int32_t
CombinedIndexReader::countDocuments() {
    int32_t c = m->p->writermanager->getIndexReader()->countDocuments();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->getIndexReader()->countDocuments();
    }
    return c;
}
int32_t
CombinedIndexReader::countWords() {
    int32_t c = m->p->writermanager->getIndexReader()->countWords();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->getIndexReader()->countWords();
    }
    return c;
}
int64_t
CombinedIndexReader::getIndexSize() {
    int64_t c = m->p->writermanager->getIndexReader()->getIndexSize();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->getIndexReader()->getIndexSize();
    }
    return c;
}
int64_t
CombinedIndexReader::getDocumentId(const string& uri) {
    return m->p->writermanager->getIndexReader()->getDocumentId(uri);
}
time_t
CombinedIndexReader::getMTime(int64_t docid) {
    return m->p->writermanager->getIndexReader()->getMTime(docid);
}