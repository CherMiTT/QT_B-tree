#ifndef TREEDESCRIPTOR_H
#define TREEDESCRIPTOR_H

#include <QObject>
#include <QWidget>
#include "TreePage.h"

class Tree : public QWidget //синглтон
{
    Q_OBJECT

private:

    int elementCount; //число элементов

    int n; //порядок дерева
    static Tree *pTree; //указатель на дескриптор
    TreePage *root; //указатель на корневой элемент

    explicit Tree(QWidget *parent = nullptr, int order = 1);
    ~Tree();

    void repaintTree(TreePage*, int, int);

    void deletePage(TreePage*); //используется в деструкторе
    int addElementToPage(int, TreePage*); //добавление элемента на страницу, возвращает индекс элемента на странице
    void restoreTree(TreePage*); //восстанавливает свойство дерева, если на какой-то странице больше 2n элементов

    void recountNeededSpace(TreePage* ptr);
public:
    QGraphicsScene *scene;

    static Tree *getPTree(int order); //обновляет порядок дерева при его первоначальном создании
    static Tree *getPTree();
    friend class TreePage;

signals:



public slots:
    void orderChanged(int newOrder);
    void addElement(int e);
    void searchForElement(int e);
    void deleteElement(int e);
};

#endif // TREEDESCRIPTOR_H
