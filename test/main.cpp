#include <QCoreApplication>
#include <QtTest/QtTest>

#include "testsuit.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestSuit test_suit;

    return QTest::qExec(&test_suit, argc, argv);
}


