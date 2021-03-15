#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "TreePage.h"
#include "Tree.h"

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg); //обработчик сообщений в лог
QScopedPointer<QFile> logFile; //умный указатель на файл лога

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    logFile.reset(new QFile("D:/Projects/B-Tree/logs/log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss") + ".txt")); //устанавливаю файл лога
    logFile.data()->open(QFile::Append | QFile::Text); //открываем файл для записи

    qInstallMessageHandler(messageHandler); //устанавливаем обработчик

    qInfo(logInfo()) << "Приложение запущено, логгер успешно запущен.";
    ui->setupUi(this);

    connect(ui->addElementButton, &QPushButton::clicked, this, &MainWindow::addElementClicked);
    Tree *tree = Tree::getPTree(ui->treeOrderSpinBox->value());
    qInfo(logInfo()) << "Синглтон дерева успещно создан с порядком " + QString::number(ui->treeOrderSpinBox->value());

    tree->scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(tree->scene);
    qInfo(logInfo()) << "Файл UI установлен.";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addElementClicked()
{
    qInfo(logInfo()) << "Нажата кнопка добавить элемент.";
    Tree *tree = Tree::getPTree();
    tree->addElement(ui->addElementField->text().toInt()); //TODO: добавить валидацию
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

