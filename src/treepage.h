/*!
* \file
* \brief Header file for treePage class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#ifndef TREEPAGE_H
#define TREEPAGE_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>

#include "Logger.h"

//#define NOT_VALUE -100000
/*!
 * \brief Класс страницы B-дерева
 */
class TreePage : public QWidget
{
    Q_OBJECT

    friend class Tree;

public:
    explicit TreePage(QWidget *parent = nullptr, TreePage* parentPage = nullptr, size_t n = 2); //! Конструктор
    QString formElementsToString();
    void sort();
signals:

private:
    size_t elementsCount; //! Количество элементов на странице
    size_t descendantsCount; //! Количество потомков
    TreePage* parentPage; //! Родительская страница
    QList<int> elements; //! Список элементов страницы
    QList<TreePage*> descendands; //! Список дочерних страниц

};

#endif // TREEPAGE_H
