/*!
* \file
* \brief Header file for tree class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#ifndef TREE_H
#define TREE_H

#include <QWidget>
#include "treepage.h"

/*!
 * \brief Класс B-дерева
 */
class Tree : public QWidget
{
    Q_OBJECT
public:
    QGraphicsScene* scene; //! Графическая сцена, на которой рисуем дерево

    explicit Tree(QWidget *parent = nullptr, size_t n = 0); //! Конструктор

signals:
    void rebuildTree(); //! Сигнал о том, что нужно перестраивать всё дерево с нуля
    void treeStructureChanged(); //! Сигнал о том, что изменилась структура дерева (добавили/удалили элемент/страницу)

public slots:
    void orderChanged(int newOrder);
    void addElement(int e);
    TreePage* searchForElement(int e);

private:
    size_t n; //! Порядок дерева
    TreePage* root; //! Корень дерева
    size_t elementCount; //! Количество элементов в дереве


    int addElementToPage(TreePage*, int);
    TreePage* findPageWhereAdd(int e);
    void restoreTree(TreePage*);
    int splitPage(TreePage&, TreePage&);
    void recountNeededSpace(TreePage*);
    void repaintTree(TreePage*, int, int);
};

#endif // TREE_H
