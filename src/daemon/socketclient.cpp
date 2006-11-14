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
#include "socketclient.h"
#include "filters.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <assert.h>
#include <sstream>
using namespace std;

#if defined(__APPLE__)
#define SOCKET_NOSIGNAL SO_NOSIGPIPE
#elif defined( MSG_NOSIGNAL )
#define SOCKET_NOSIGNAL MSG_NOSIGNAL
#else
#define SOCKET_NOSIGNAL 0
#endif

void
SocketClient::setSocketName(const string& n) {
    socketpath = n;
}
int
SocketClient::open() {
    struct sockaddr_un  serv_addr;

    // create a socket
    int sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd < 0) {
        error = "Could not create socket: ";
        error += strerror(errno);
        return -1;
    }

    // set the address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    uint len = socketpath.length();
    len = (len > sizeof(serv_addr.sun_path)) ?sizeof(serv_addr.sun_path) :len;
    strncpy(serv_addr.sun_path, socketpath.c_str(), len);
    serv_addr.sun_path[len] = '\0';

    // connect to the server
    int r = connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (r < 0) {
        error = "Could not connect to server: ";
        error += strerror(errno);
        close(sd);
        return -1;
    }
    return sd;
}
bool
SocketClient::readResponse(int sd) {
    response.clear();
    string line;
    char c;
    while (true) {
        int r = recv(sd, &c, 1, 0);
        if (r < 0) {
            error = "Error reading from socket: ";
            error += strerror(errno);
            printf("%s\n", error.c_str());
            return false;
        } else if (r == 0 || c == 0) {
            if (line.size() > 0) {
                response.push_back(line);
            }
            return true;
        } else if (c == '\n') {
            if (line.size() == 0) {
                // finished reading the request
                return true;
            }
            response.push_back(line);
            line.clear();
        } else {
            line += c;
        }
    }
}
bool
SocketClient::sendRequest(int sd) {
    ssize_t r;
    for (uint i=0; i<request.size(); ++i) {
        string line = request[i];
        assert(line.find('\n') == string::npos);
        line += '\n';
        int p = 0;
        int len = line.length();
        do {
            r = send(sd, line.c_str()+p, len-p, SOCKET_NOSIGNAL);
            if (r < 0) {
                printf("error writing request\n");
                return false;
            }
            p += r;
        } while (p < len);
    }
    r = send(sd, "\n", 1, SOCKET_NOSIGNAL);
    return r > 0;
}
int
SocketClient::countHits(const string &query) {
    response.clear();
    request.clear();
    request.push_back("countHits");
    assert(query.find("\n") == string::npos);
    request.push_back(query);
    int sd = open();
    if (sd < 0) {
        fprintf(stderr, "   %s\n", error.c_str());
        return -1;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    if (response.size() == 0) return -1;
    int count = atoi(response[0].c_str());
    return count;
}
ClientInterface::Hits
SocketClient::getHits(const string &query, int max, int off) {
    response.clear();
    request.clear();
    request.push_back("query");
    assert(query.find("\n") == string::npos);
    request.push_back(query);
    ostringstream oss;
    oss << max;
    request.push_back(oss.str());
    oss.str("");
    oss << off;
    request.push_back(oss.str());
    int sd = open();
    Hits hits;
    if (sd < 0) {
        printf("   %s\n", error.c_str());
        hits.error = error;
        return hits;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    uint i = 0;
    while (i+6 < response.size()) {
        jstreams::IndexedDocument h;
        h.uri = response[i++];
        h.fragment = response[i++];
        h.mimetype = response[i++];
        h.score = atof(response[i++].c_str());
        h.size = atoi(response[i++].c_str());
        h.mtime = atoi(response[i++].c_str());
        while (i < response.size()) {
            const char* s = response[i].c_str();
            const char* v = strchr(s, ':');
            if (!v) break;
            const char* d = strchr(s, '/');
            if (d && d < v) {
                break;
            }
            string n(s, v-s);
            h.properties.insert(make_pair(n,v+1));
            ++i;
        }
        hits.hits.push_back(h);
    }
    response.clear();
    return hits;
}
map<string, string>
SocketClient::getStatus() {
    map<string, string> status;
    response.clear();
    request.clear();
    request.push_back("getStatus");
    int sd = open();
    if (sd < 0) {
        // no connection: return an empty map
        //printf("   %s\n", error.c_str());
        //status["error"] = error;
        return status;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    for (uint i=0; i<response.size(); ++i) {
        string s = response[i];
        uint p = s.find(":");
        if (p == string::npos) {
            printf("''%s''\n", s.c_str());
            status.clear();
            status["error"] = "Communication error.";
            return status;
        }
        status[s.substr(0,p)] = s.substr(p+1);
    }
    return status;
}
vector<string>
SocketClient::getBackEnds() {
    vector<string> v;
    return v;
}
string
SocketClient::stopDaemon() {
    request.clear();
    request.push_back("stopDaemon");
    int sd = open();
    if (sd < 0) {
        return "";
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    return "";
}
string
SocketClient::startIndexing() {
    request.clear();
    request.push_back("startIndexing");
    int sd = open();
    if (sd < 0) {
        return "";
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    return "";
}
string
SocketClient::stopIndexing() {
    request.clear();
    request.push_back("stopIndexing");
    int sd = open();
    if (sd < 0) {
        return "";
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    return "";
}
set<string>
SocketClient::getIndexedDirectories() {
    set<string> r;
    request.clear();
    response.clear();
    request.push_back("getIndexedDirectories");
    int sd = open();
    if (sd < 0) {
        return r;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    vector<string>::const_iterator i;
    for (i = response.begin(); i != response.end(); ++i) {
        r.insert(*i);
    }
    return r;
}
string
SocketClient::setIndexedDirectories(set<string> dirs) {
    request.clear();
    request.push_back("setIndexedDirectories");
    set<string>::const_iterator i;
    for (i = dirs.begin(); i != dirs.end(); ++i) {
        request.push_back(*i);
    }
    int sd = open();
    if (sd < 0) {
        return "";
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    return "";
}
multimap<int,string>
SocketClient::getFilteringRules() {
    multimap<int,string> r;
    request.clear();
    response.clear();
    request.push_back("getFilteringRules");
    int sd = open();
    if (sd < 0) {
        return r;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    
    // response structure:
    // LINE 0: FILTER_RTTI-TOT_RULES_NUMBER
    // ... rules ...
    // LINE $RULES_NUMBER: FILTER_RTTI-TOT_RULES_NUMBER
    // ... rules ...
    int filterRTTI = 0;
    unsigned int filterNum = 0;
    int unsigned i = 0;
    
    while ((i < response.size()) && (sscanf(response[i].c_str(), "%i-%u", &filterRTTI, &filterNum) != 0))
    {
        i++;
        for (unsigned int ub = i + filterNum; (i < ub) && (i < response.size()); i++)
            r.insert (make_pair(filterRTTI, response[i]));
    }
    
    return r;
}
void
SocketClient::setFilteringRules(const multimap<int,string>& rules) {
    request.clear();
    request.push_back("setFilteringRules");
    
    multimap<int,string>::const_iterator lb, ub, i;
    char buffer[500];
        
    lb = rules.lower_bound(PathFilter::RTTI);
    ub = rules.upper_bound(PathFilter::RTTI);
        
    // request structure:
    // LINE 0: FILTER_RTTI-TOT_RULES_NUMBER
    // ... rules ...
    // LINE $RULES_NUMBER: FILTER_RTTI-TOT_RULES_NUMBER
    // ... rules ...
        
    snprintf (buffer, 500*sizeof(char), "%i-%u",PathFilter::RTTI, rules.count(PathFilter::RTTI));
    request.push_back(string(buffer));
    for (i = lb; i != ub; ++i) {
        request.push_back(i->second);
    }
        
    lb = rules.lower_bound(PatternFilter::RTTI);
    ub = rules.upper_bound(PatternFilter::RTTI);
        
    snprintf (buffer, 500*sizeof(char), "%i-%u",PatternFilter::RTTI, rules.count(PatternFilter::RTTI));
    request.push_back(string(buffer));
    for (i = lb; i != ub; ++i) {
        request.push_back(i->second);
    }
    
    int sd = open();
    if (sd < 0) {
        return ;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
}
set<string>
SocketClient::getIndexedFiles() {
    set<string> r;
    request.clear();
    response.clear();
    request.push_back("getIndexedFiles");
    int sd = open();
    if (sd < 0) {
        return r;
    }
    sendRequest(sd);
    readResponse(sd);
    close(sd);
    vector<string>::const_iterator i;
    for (i = response.begin(); i != response.end(); ++i) {
        r.insert(*i);
    }
    return r;
}
void
SocketClient::indexFile(const std::string &path, uint64_t mtime,
        const std::vector<char>& content) {
    printf("so you want me to send a file to strigi?\n");
    request.clear();
    response.clear();
    request.push_back("indexFile");
    request.push_back(path);
    ostringstream out;
    out << mtime;
    request.push_back(out.str());
    request.push_back(&content[0]);
}

