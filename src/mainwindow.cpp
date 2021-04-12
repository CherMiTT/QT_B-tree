/*!
* \file
* \brief Source file for MainWindow class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg); //обработчик сообщений в лог
QScopedPointer<QFile> logFile; //умный указатель на файл лога

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    logFile.reset(new QFile("D:\\Projects\\B-Tree-Refactor\\src\\logs\\log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss") + ".txt")); //устанавливаю файл лога
    logFile.data()->open(QFile::Append | QFile::Text); //открываем файл для записи

    qInstallMessageHandler(messageHandler); //устанавливаем обработчик

    qInfo(logInfo()) << "Приложение запущено, логгер успешно запущен.";
    ui->setupUi(this);
    tree = new Tree(nullptr, 2); //TODO: изменить порядок на вводимый

    //TODO: добавить QValidator-ы для полей ввода
    connect(ui->addElementButton, &QPushButton::clicked, this, &MainWindow::addElementClicked);
    connect(ui->searchElementButton, &QPushButton::clicked, this, &MainWindow::searchElementClicked);

    tree->scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(tree->scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * \brief Слот, вызываемый при клике по кнопке "Добавить элемент"
 */
void MainWindow::addElementClicked()
{
    qInfo(logInfo()) << "Нажата кнопка добавить элемент.";
    ui->statusLabel->setText("");

    int e = ui->addElementField->text().toInt();  //TODO: добавить валидацию

    if(tree->searchForElement(e) == nullptr)
    {
        qInfo(logInfo()) << "Элемента" << e << "ещё нет в дереве.";
        tree->addElement(e);
    }
    else
    {
        qInfo(logInfo()) << "Элемент" << e << "уже есть в дереве.";
        ui->statusLabel->setText("Элемент уже есть в дереве.");
    }

}

/*!
 * \brief Слот, вызываемый при клике по кнопке "Найти элемент"
 */
void MainWindow::searchElementClicked()
{
    qInfo(logInfo()) << "Нажата кнопка поиска элемента.";
    TreePage* result = tree->searchForElement(ui->searchElementField->text().toInt()); //TODO: добавить валидацию
    if(result == nullptr)
    {
        ui->statusLabel->setText("Элемент не найден.");
    }
    else
    {
        ui->statusLabel->setText("Элемент найден на странице \n" + result->formElementsToString() + ".");
    }
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) //обработчик сообщений в лог
{
    QTextStream out(logFile.data()); //открываем поток записи в файл
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss "); //записываем дату

    switch (type) //определяем по типу уровень сообщения
    {
    case QtInfoMsg:     out << "INF "; break;
    case QtDebugMsg:    out << "DBG "; break;
    case QtWarningMsg:  out << "WRN "; break;
    case QtCriticalMsg: out << "CRT "; break;
    case QtFatalMsg:    out << "FTL "; break;
    }
    out << context.category << ": " << msg << Qt::endl; //записываем категориюи сообщение
    out.flush();    // Очищаем буферизированные данные
}

