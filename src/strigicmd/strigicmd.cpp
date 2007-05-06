/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Jos van den Oever <jos@vandenoever.info>
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
#include "analyzerconfiguration.h"
#include "diranalyzer.h"
#include "strigiconfig.h"
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <dirent.h>
using namespace std;
using namespace Strigi;

map<char, string> options;
vector<string> dirs;

void
parseArguments(int argc, char** argv) {
    // parse arguments
    int i = 1;
    while (++i < argc) {
        const char* arg = argv[i];
        size_t len = strlen(arg);
        if (len == 0) continue;
        if (*arg == '-' && len > 1) {
            char option = arg[1];
            if (len > 2) {
                arg += 2;
            } else if (i+1 < argc) {
                arg = argv[++i]; 
            } else {
                arg = "";
            }
            options[option].assign(arg);
        } else {
            dirs.push_back(arg);
        }
    }
}
/*
 * Help function for printing to stderr: fprintf(stderr, 
 */
int
pe(const char *format, ...) {
    va_list arg;
    int done;
    va_start(arg, format);
    done = vfprintf(stderr, format, arg);
    va_end(arg);
    return done;
}
/**
 * This is the main for implementing a command line program that can create
 * and query indexes.
 **/
int
usage(int argc, char** argv) {
    pe("%s:\n", argv[0]);
    pe("    Program for creating and querying indices.\n");
    pe("    This program is not meant for talking to the strigi daemon.\n\n");
    pe("usage:\n");
    pe("  %s create [-j num] -t backend -d indexdir files/dirs\n");
    pe("  %s update [-j num] -t backend -d indexdir files/dirs\n");
    return 1; 
}
void
checkIndexdirIsEmpty(const char* dir) {
    DIR* d = opendir(dir);
    if (!d) return;
    struct dirent* de = readdir(d);
    while (de) {
        if (strcmp(de->d_name, "..") && strcmp(de->d_name, ".")) {
            fprintf(stderr, "Directory %s is not empty.\n", dir);
            exit(1);
        }
        de = readdir(d);
    }
    closedir(d);
}
void
printBackends(const string& msg, const vector<string> backends) {
    pe(msg.c_str());
    pe(" Choose one from ");
    for (uint j=0; j<backends.size()-1; ++j) {
        pe("'%s', ", backends[j].c_str());
    }
    pe("'%s'.\n", backends[backends.size()-1].c_str());
}
int
create(int argc, char** argv) {
    // parse arguments
    parseArguments(argc, argv);
    string backend = options['t'];
    string indexdir = options['d'];
    int nthreads = atoi(options['j'].c_str());
    if (nthreads < 1) nthreads = 2;

    // check arguments: backend
    const vector<string>& backends = CombinedIndexManager::backEnds();
    // if there is only one backend, the backend does not have to be specified
    if (backend.size() == 0) {
        if (backends.size() == 1) {
            backend = backends[0];
        } else {
            printBackends("Specify a backend.", backends);
            return usage(argc, argv);
        }
    }
    vector<string>::const_iterator b
        = find(backends.begin(), backends.end(), backend);
    if (b == backends.end()) {
        printBackends("Invalid index type.", backends);
        return usage(argc, argv);
    }
    // check arguments: indexdir
    if (indexdir.length() == 0) {
        pe("Provide a dir to write the index to with -d.\n");
        return usage(argc, argv);
    }
    // check that the dir does not yet exist
    checkIndexdirIsEmpty(indexdir.c_str());

    // check arguments: dirs
    if (dirs.size() == 0) {
        pe("'%s' '%s'\n", backend.c_str(), indexdir.c_str());
        pe("Provide directories to index.\n");
        return usage(argc, argv);
    }

    IndexManager* manager
        = CombinedIndexManager::factories()[backend](indexdir.c_str());
    IndexWriter* writer = manager->indexWriter();
    AnalyzerConfiguration config;
    DirAnalyzer* analyzer = new DirAnalyzer(*writer, &config);
    vector<string>::const_iterator j;
    for (j = dirs.begin(); j != dirs.end(); ++j) {
        analyzer->analyzeDir(j->c_str(), nthreads);
    }
    delete analyzer;
    delete manager;

    return 0;
}
int
update(int argc, char** argv) {
    return 0;
}
int
main(int argc, char** argv) {
    if (argc < 2) { 
        return usage(argc, argv);
    }
    const char* cmd = argv[1];
    if (!strcmp(cmd,"create")) {
        return create(argc, argv);
    } else if (!strcmp(cmd,"update")) {
        return create(argc, argv);
    } else {
        return usage(argc, argv);
    }
    return 0;
}
