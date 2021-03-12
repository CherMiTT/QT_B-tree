#include "Tree.h"

#include <QDebug>
#include <QTextBlockFormat>
#include <QTextCursor>

Tree* Tree::pTree = nullptr;

Tree::Tree(QWidget *parent, int order) : QWidget(parent)
{
    this->elmentCount = 0;
    this->n = order;
    this->root = nullptr;
}

Tree::~Tree()
{
    qInfo(logInfo()) << "Tree's destructor started";
    if(pTree != nullptr)
    {
        if(root != nullptr)
        {
            deletePage(root);
        }
        delete pTree;
        pTree = nullptr;
    }
}

void Tree::deletePage(TreePage *page) //рекуисивно освобождает память под страницу и все её потомки
{
    qInfo(logInfo()) << "Deleting page " + page->formElementsToString() + "and all its children";
    if(page != nullptr)
    {
        for(int i = 0; i < n * 2 + 1; i++)
        {
            if(page->arrPDescendants[i] != nullptr) deletePage(page->arrPDescendants[i]);
        }
        delete page;
    }
}

Tree* Tree::getPTree(int order)
{
    if(pTree == nullptr)
    {
        pTree = new Tree(nullptr, order);
    }
    return pTree;
}

Tree* Tree::getPTree()
{
    if(pTree == nullptr)
    {
        pTree = new Tree();
    }
    return pTree;
}


void Tree::addElement(int e) //добавить элемнент в дерево (с поиском страницы, куда добавить)
{
    qInfo(logInfo()) << "Adding element " + QString::number(e) + " to the tree";
    if(root == nullptr) //если это первый элемент в дереве
    {
        qInfo(logInfo()) << "That's the first element in the tree. Creating root page";
        root = new TreePage(nullptr, n, nullptr); //создаём корневую страницу
        //root->addElementToThisPage(e);
        addElementToPage(e, root); //добавляем первый элемент
    }
    else
    {
        qInfo(logInfo()) << "Searching for a plase to put that element";
        TreePage *ptr = root;
        while(ptr->descendantsCount != 0) //просматриваем, пока не находим лист
        {
            //TODO: проверить с индексами в цикле
            for(int i = 0; i <= 2 * n; i++) //цикл по всем элементам страницы
            {
                if(ptr->elements[i] != -10000) //если этот элемент есть
                    //TODO: подумать, как заменить с elementsCount
                {
                    if(e < ptr->elements[i])
                    {
                        ptr = ptr->arrPDescendants[i]; //TODO: прокомментировать
                        break;
                    }

                    if ((ptr->elements[i+1] == -10000) && e > ptr->elements[i]) //если нет следующего элемента в странице, а новый элемент больше последнего
                    {
                        ptr = ptr->arrPDescendants[i+1]; //тогда переходим к самому последнему потомку
                        break;
                    }
                }
                else {break;}
            }
        }

        addElementToPage(e, ptr); //добавляем элемент на страницу
        ptr->sort();

        //проверяем свойства дерева и перестраиваем (если надо), начиная с этой страницы и до корня, пока не начнут выполняться свойства
        while(ptr->elementsCount > 2 * n) //не выполнилось свойство после добавления элемента
        {
            if(ptr == root) //если это корень
            {
                restoreTree(root); //восстанавливаем свойство
                break;
            }
            else //если это не корень
            {
                restoreTree(ptr); //восстанавливаем свойство в странице
                ptr = ptr->pParentPage; //проверяем родителя этой страницы
            }
        }
    }
    repaintTree(root, 1);
}

void Tree::addElementToPage(int e, TreePage *pPage) //Добавление элемента на конкретную страницу
{
    qInfo(logInfo()) << "Adding element to the page " + pPage->formElementsToString();
    pPage->elements[pPage->elementsCount++] = e; //Добавить элемент на страницу и увеличить кол-во элементов
    pPage->sort();
}

void Tree::restoreTree(TreePage *pPage) //восстанавливает свойство дерева, если на какой-то странице больше 2n элементов
{
    if (pPage->elementsCount <= 2*n) return; //если для страницы выполняется свойство

    qInfo(logInfo()) << "Page " << pPage->formElementsToString() << " has more than 2n elements. Restoring tree";
}

void Tree::orderChanged(int newOrder)
{

}

void Tree::searchForElement(int e)
{

}

void Tree::deleteElement(int e)
{

}

void Tree::repaintTree(TreePage *pPage, int level)
{
    qInfo(logInfo()) << "Repainting tree (page " << pPage->formElementsToString() << ")";
    scene->clear();

    if(pPage == nullptr) return;

    QString str = pPage->formElementsToString();

    int x = size().width()/2; //TODO
    int y = 30 * level;
    int width = 25 + 6 * str.length();
    int height = 30;
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(x, y, width, height, nullptr);
    rectItem->setPen(QPen(Qt::black));
    rectItem->setBrush(QBrush(Qt::white));
    scene->addItem(rectItem);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(str, rectItem);
    textItem->setPos(size().width()/2, 30 * level);
    textItem->setTextWidth(textItem->boundingRect().width());

    for(int i = 0; i < pPage->descendantsCount; i++)
    {
        repaintTree(pPage->arrPDescendants[i], level + 1);
    }
}
