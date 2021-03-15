#ifndef TREEPAGE_H
#define TREEPAGE_H

#include <QWidget>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>
#include "Logger.h"

class TreePage : public QWidget
{
    Q_OBJECT

private:
    int x, y;
    int needsSpace;

    int n; //порядок дерева
    int elementsCount; //количество элементов на странице
    int descendantsCount; //количество потомков
    int *elements; //массив элементов страницы
    TreePage *pParentPage; //указатель на страницу-родитель
    TreePage **arrPDescendants; //массив указателей на потомков
    void sort();

    QString formElementsToString(); //возвращает строку с элементами страницы, разделёнными пробелами
public:

    explicit TreePage(QWidget *parent = nullptr, int order = 0, TreePage *pParentPage = nullptr);

    friend class Tree;

public slots:

signals:



};

#endif // TREEPAGE_H
