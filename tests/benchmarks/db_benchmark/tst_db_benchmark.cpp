/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <functional>
#include <random>

#include <QtTest/QtTest>

#include "tst_common.h"
#include "TimeLogCategoryTreeNode.h"

QTemporaryDir *dataDir = Q_NULLPTR;
TimeLogHistory *history = Q_NULLPTR;

const int maxTimeout = 300000;

class tst_DB_Benchmark : public QObject
{
    Q_OBJECT

public:
    tst_DB_Benchmark();
    virtual ~tst_DB_Benchmark();

private slots:
    void init();
    void cleanup();
    void initTestCase();

    void create();
    void import();
    void import_data();
    void getHistory();
    void getHistory_data();
    void insert();
    void insert_data();
    void remove();
    void remove_data();
    void edit();
    void edit_data();
    void renameCategory();
    void renameCategory_data();
};

tst_DB_Benchmark::tst_DB_Benchmark()
{
}

tst_DB_Benchmark::~tst_DB_Benchmark()
{
}

void tst_DB_Benchmark::init()
{
    dataDir = new QTemporaryDir();
    Q_CHECK_PTR(dataDir);
    QVERIFY(dataDir->isValid());
    history = new TimeLogHistory;
    Q_CHECK_PTR(history);
    QVERIFY(history->init(dataDir->path()));
}

void tst_DB_Benchmark::cleanup()
{
    if (QTest::currentTestFailed()) {
//        dataDir->setAutoRemove(false);
    }
    history->deinit();
    delete history;
    history = Q_NULLPTR;
    delete dataDir;
    dataDir = Q_NULLPTR;
}

void tst_DB_Benchmark::initTestCase()
{
    qRegisterMetaType<QSet<QString> >();
    qRegisterMetaType<QVector<TimeLogEntry> >();
    qRegisterMetaType<TimeLogHistory::Fields>();
    qRegisterMetaType<QVector<TimeLogHistory::Fields> >();
    qRegisterMetaType<QSharedPointer<TimeLogCategoryTreeNode> >();

    qSetMessagePattern("[%{time}] <%{category}> %{type} (%{file}:%{line}, %{function}) %{message}");
}

void tst_DB_Benchmark::create()
{
    QBENCHMARK {
        TimeLogHistory history;
        QTemporaryDir dataDir;
        QVERIFY(dataDir.isValid());
        QVERIFY(history.init(dataDir.path()));
    }
}

void tst_DB_Benchmark::import()
{
    QFETCH(int, entriesCount);

    QVector<TimeLogEntry> origData(genData(entriesCount));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    QBENCHMARK_ONCE {
        history->import(origData);
        QVERIFY(importSpy.wait(maxTimeout));
    }
    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
}

void tst_DB_Benchmark::import_data()
{
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("empty db") << 0;
    QTest::newRow("1 entry") << 1;
    QTest::newRow("2 entries") << 2;
    QTest::newRow("10 entries") << 10;
    QTest::newRow("100 entries") << 100;
    QTest::newRow("1 000 entries") << 1000;
    QTest::newRow("10 000 entries") << 10000;
    QTest::newRow("50 000 entries") << 50000;
}

void tst_DB_Benchmark::getHistory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy dataSpy(history, SIGNAL(historyRequestCompleted(QVector<TimeLogEntry>,qlonglong)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    QFETCH(int, entriesCount);

    QBENCHMARK_ONCE {
        dataSpy.clear();
        qlonglong id = QDateTime::currentMSecsSinceEpoch();
        history->getHistoryAfter(id, entriesCount, QDateTime::fromTime_t(0, Qt::UTC));
        QVERIFY(dataSpy.wait(maxTimeout));
        QCOMPARE(dataSpy.constFirst().at(1).toLongLong(), id);
    }

    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
}

void tst_DB_Benchmark::getHistory_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_DB_Benchmark::insert()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy insertSpy(history, SIGNAL(dataInserted(TimeLogEntry)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    QFETCH(int, entriesCount);

    QBENCHMARK_ONCE {
        for (int i = 0; i < entriesCount; i++) {
            TimeLogEntry entry;
            entry.startTime = origData.constLast().startTime.addSecs(100);
            entry.category = "CategoryNew";
            entry.comment = "Test comment";
            entry.uuid = QUuid::createUuid();
            history->insert(entry);
            origData.append(entry);
        }
        while (insertSpy.size() < entriesCount) {
            QVERIFY(insertSpy.wait(maxTimeout));
        }
    }

    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
}

void tst_DB_Benchmark::insert_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;
    QTest::newRow("1 entry, 10") << 1 << 10;
    QTest::newRow("1 entry, 100") << 1 << 100;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;
    QTest::newRow("10 entries, 100") << 10 << 100;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_DB_Benchmark::remove()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy removeSpy(history, SIGNAL(dataRemoved(TimeLogEntry)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    QFETCH(int, entriesCount);

    std::uniform_int_distribution<> indexDistribution(0, origData.size() - 1);
    std::function<int()> randomIndex = std::bind(indexDistribution, std::default_random_engine());

    QVector<int> indices;

    while (indices.size() < entriesCount) {
        int index = randomIndex();
        if (!indices.contains(index)) {
            indices.append(index);
        }
    }

    QBENCHMARK_ONCE {
        for (int i = 0; i < indices.size(); i++) {
            history->remove(origData.at(indices.at(i)));
        }
        while (removeSpy.size() < entriesCount) {
            QVERIFY(removeSpy.wait(maxTimeout));
        }
    }

    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
}

void tst_DB_Benchmark::remove_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_DB_Benchmark::edit()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));
    QSignalSpy updateSpy(history, SIGNAL(dataUpdated(QVector<TimeLogEntry>,QVector<TimeLogHistory::Fields>)));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    QFETCH(int, entriesCount);

    std::uniform_int_distribution<> indexDistribution(0, origData.size() - 1);
    std::function<int()> randomIndex = std::bind(indexDistribution, std::default_random_engine());

    QVector<int> indices;

    while (indices.size() < entriesCount) {
        int index = randomIndex();
        if (!indices.contains(index)) {
            indices.append(index);
        }
    }

    QBENCHMARK_ONCE {
        for (int i = 0; i < indices.size(); i++) {
            TimeLogEntry entry = origData.at(indices.at(i));
            entry.comment = "New Test comment";
            history->edit(entry, TimeLogHistory::Comment);
        }
        while (updateSpy.size() < entriesCount) {
            QVERIFY(updateSpy.wait(maxTimeout));
        }
    }

    QVERIFY(errorSpy.isEmpty());
    QVERIFY(outdateSpy.isEmpty());
}

void tst_DB_Benchmark::edit_data()
{
    QTest::addColumn<int>("initialEntries");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("1 entry, 1") << 1 << 1;

    QTest::newRow("10 entries, 1") << 10 << 1;
    QTest::newRow("10 entries, 10") << 10 << 10;

    QTest::newRow("100 entries, 1") << 100 << 1;
    QTest::newRow("100 entries, 10") << 100 << 10;
    QTest::newRow("100 entries, 100") << 100 << 100;

    QTest::newRow("1000 entries, 1") << 1000 << 1;
    QTest::newRow("1000 entries, 10") << 1000 << 10;
    QTest::newRow("1000 entries, 100") << 1000 << 100;

    QTest::newRow("10000 entries, 1") << 10000 << 1;
    QTest::newRow("10000 entries, 10") << 10000 << 10;
    QTest::newRow("10000 entries, 100") << 10000 << 100;

    QTest::newRow("50000 entries, 1") << 50000 << 1;
    QTest::newRow("50000 entries, 10") << 50000 << 10;
    QTest::newRow("50000 entries, 100") << 50000 << 100;
}

void tst_DB_Benchmark::renameCategory()
{
    QFETCH(int, initialEntries);

    QVector<TimeLogEntry> origData(genData(initialEntries));

    QSignalSpy errorSpy(history, SIGNAL(error(QString)));
    QSignalSpy outdateSpy(history, SIGNAL(dataOutdated()));

    QSignalSpy importSpy(history, SIGNAL(dataImported(QVector<TimeLogEntry>)));
    history->import(origData);
    QVERIFY(importSpy.wait(maxTimeout));

    QBENCHMARK_ONCE {
        int index = std::ceil((origData.size() - 1) / 2.0);
        history->editCategory(origData.at(index).category, TimeLogCategory(QUuid::createUuid(),
                                                                           TimeLogCategoryData("CategoryNew")));
        QVERIFY(outdateSpy.wait(maxTimeout));
    }

    QVERIFY(errorSpy.isEmpty());
}

void tst_DB_Benchmark::renameCategory_data()
{
    QTest::addColumn<int>("initialEntries");

    QTest::newRow("1 entry") << 1;
    QTest::newRow("10 entries") << 10;
    QTest::newRow("100 entries") << 100;
    QTest::newRow("1000 entries") << 1000;
    QTest::newRow("10000 entries") << 10000;
    QTest::newRow("50000 entries") << 50000;
}

QTEST_MAIN(tst_DB_Benchmark)
#include "tst_db_benchmark.moc"
