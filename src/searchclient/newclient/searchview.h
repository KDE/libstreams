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
#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <QtGui/QWidget>
#include "strigiasyncclient.h"

class QTextBrowser;
class QUrl;
class SearchViewHtmlHelper;
class StrigiHtmlGui;

class SearchView : public QWidget {
Q_OBJECT
private:
    QTextBrowser* view;
    QString query;
    StrigiAsyncClient asyncstrigi;
    SearchViewHtmlHelper* htmlguihelper;
    StrigiHtmlGui* htmlgui;

private Q_SLOTS:
    void openItem(const QUrl& url);
public:
    explicit SearchView();
    ~SearchView();
    void setHTML(const QString&html);
public Q_SLOTS:
    void handleHits(const QString& q, int offset, const QList<StrigiHit>& hits);
    void setQuery(const QString&);
Q_SIGNALS:
    void gotHits(const QString& query);
};

#endif
