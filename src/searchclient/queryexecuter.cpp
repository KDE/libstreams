#include "queryexecuter.h"
#include "socketclient.h"
#include <QtCore/QDebug>
using namespace std;

void
QueryExecuter::run() {
    mutex.lock();
    QString q = querystring;
    mutex.unlock();

    SocketClient client;
    std::string socket = getenv("HOME");
    socket += "/.kitten/socket";
    client.setSocketName(socket.c_str());
    vector<string> r;

    QString oldq;
    do {
        qDebug() << "querying for " << q;
        r = client.query((const char*)q.toUtf8());
        oldq = q;
        mutex.lock();
        q = querystring;
        mutex.unlock();
    } while (q != oldq);
    mutex.lock();
    results = r;
    mutex.unlock();
    emit queryFinished(q);
}
QueryExecuter::QueryExecuter() {
}
QueryExecuter::~QueryExecuter() {
    // TODO add a timeout
}
std::vector<std::string>
QueryExecuter::getResults() {
    mutex.lock();
    std::vector<std::string> r = results;
    mutex.unlock();
    return r;
}
void
QueryExecuter::query(const QString& q) {
    mutex.lock();
    querystring = q;
    mutex.unlock();
    if (!isRunning()) start();
}
