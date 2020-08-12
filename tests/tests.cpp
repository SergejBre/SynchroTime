#include <QString>
#include <QtTest>

class Tests : public QObject
{
    Q_OBJECT

public:
    Tests();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void Case1_data();
    void Case1();
};

Tests::Tests()
{
}

void Tests::initTestCase()
{
}

void Tests::cleanupTestCase()
{
}

void Tests::Case1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void Tests::Case1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(Tests)

#include "tests.moc"
