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
#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include "clientinterface.h"

class SocketClient : public ClientInterface {
private:
    std::string socketpath;
    std::string error;
    std::vector<std::string> request;
    std::vector<std::string> response;

    int open();
    bool readResponse(int sd);
    bool sendRequest(int sd);
public:
    const std::string& getError() const { return error; }
    void setSocketName(const std::string& socketname);
    int countHits(const std::string &query);
    Hits getHits(const std::string &query, int max, int off);
    std::map<std::string, std::string> getStatus();
    std::string stopDaemon();
    std::string startIndexing();
    std::string stopIndexing();
    std::set<std::string> getIndexedDirectories();
    std::string setIndexedDirectories(std::set<std::string>);
    std::multimap<int,std::string> getFilteringRules();
    void setFilteringRules(const std::multimap<int,std::string>&);
    void setFilters(const std::vector<std::pair<bool,std::string> >& rules);
    std::set<std::string> getIndexedFiles();
    void indexFile(const std::string &path, uint64_t mtime,
        const std::vector<char>&);
    std::vector<std::string> getBackEnds();
};

#endif
