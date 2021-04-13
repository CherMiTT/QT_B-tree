/*!
* \file
* \brief Header file for MainWindow class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QIntValidator>

#include "tree.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class testMainWindow;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Tree* tree;

private slots:
    void addElementClicked();
    void searchElementClicked();
    void deleteElement();
};
#endif // MAINWINDOW_H
