#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class testTree : public QObject
{
    Q_OBJECT

public:
    testTree();
    ~testTree();

private slots:
    void test_case1();

};

testTree::testTree()
{

}

testTree::~testTree()
{

}

void testTree::test_case1()
{

}

QTEST_MAIN(testTree)

#include "tst_testtree.moc"
