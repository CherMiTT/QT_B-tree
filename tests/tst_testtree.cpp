#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class TestTree : public QObject
{
    Q_OBJECT

public:
    TestTree();
    ~TestTree();

private slots:
    void test_case1();

};

TestTree::TestTree()
{

}

TestTree::~TestTree()
{

}


void TestTree::test_case1()
{

}

QTEST_MAIN(TestTree)

#include "tst_testtree.moc"
