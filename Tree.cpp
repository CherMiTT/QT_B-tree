#include "Tree.h"

#include <QDebug>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <math.h>

Tree* Tree::pTree = nullptr;

Tree::Tree(QWidget *parent, int order) : QWidget(parent)
{
    this->elementCount = 0;
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
    elementCount++;
    qInfo(logInfo()) << "Adding element " + QString::number(e) + " to the tree";
    if(root == nullptr) //если это первый элемент в дереве
    {
        qInfo(logInfo()) << "That's the first element in the tree. Creating root page";
        root = new TreePage(nullptr, n, nullptr); //создаём корневую страницу
        addElementToPage(e, root); //добавляем первый элемент
        recountNeededSpace(root);
    }
    else
    {
        qInfo(logInfo()) << "Searching for a place to put that element";
        TreePage *ptr = root;
        while(ptr->descendantsCount != 0) //просматриваем, пока не находим лист
        {
            for(int i = 0; i < 2 * n; i++) //цикл по всем элементам страницы
            {
                qDebug(logDebug()) << "Смотрим на " << i << "-ый элемент страницы" << ptr->formElementsToString();
                if(ptr->elements[i] != -10000) //если этот элемент есть
                {
                    if(e < ptr->elements[i])
                    {
                        if(ptr->arrPDescendants[i] == nullptr)
                        {
                            qCritical(logCritical()) << "Пытается перейти на nullptr по индексу" << i;
                            qDebug(logDebug()) << "Потомки страницы ptr" << ptr->formElementsToString() << "по порядку:";
                            for(int i = 0; i < 2*n+1; i++)
                            {
                                if(ptr->arrPDescendants[i] == nullptr) continue;
                                qDebug(logDebug()) << "i = " << i << ":" << ptr->arrPDescendants[i]->formElementsToString();
                            }
                        }
                        ptr = ptr->arrPDescendants[i]; //TODO: прокомментировать
                        qDebug(logDebug()) << "Переходим на " << ptr->formElementsToString();
                        break;
                    }
                    if (ptr->elementsCount - 1 == i && e > ptr->elements[i]) //если нет следующего элемента в странице, а новый элемент больше последнего
                    {
                        if(ptr->arrPDescendants[i] == nullptr)
                        {
                            qCritical(logCritical()) << "Пытается перейти на nullptr (2) по индексу" << i;
                            qDebug(logDebug()) << "Потомки страницы ptr" << ptr->formElementsToString() << "по порядку:";
                            for(int i = 0; i < 2*n+1; i++)
                            {
                                if(ptr->arrPDescendants[i] == nullptr) continue;
                                qDebug(logDebug()) << "i = " << i << ":" << ptr->arrPDescendants[i]->formElementsToString();
                            }
                        }
                        ptr = ptr->arrPDescendants[ptr->elementsCount]; //тогда переходим к самому последнему потомку
                        qDebug(logDebug()) << "Переходим на " << ptr->formElementsToString();
                        break;
                    }
                }
                else {break;}
            }
        }
        addElementToPage(e, ptr); //добавляем элемент на страницу

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
    repaintTree(root, 0, 0);
}

int Tree::addElementToPage(int e, TreePage *pPage) //Добавление элемента на конкретную страницу
{
    qInfo(logInfo()) << "Adding element to the page " + pPage->formElementsToString();
    pPage->elements[pPage->elementsCount++] = e; //Добавить элемент на страницу и увеличить кол-во элементов
    pPage->sort();
    for(int i = 0; i < pPage->elementsCount; i++)
    {
        if(e == pPage->elements[i])
        {
            qInfo(logInfo()) << "Element added to the page at index " << i;
            return i;
        }
    }
}

void Tree::restoreTree(TreePage *pPage) //восстанавливает свойство дерева, если на какой-то странице больше 2n элементов
{
    if (pPage->elementsCount < 2*n + 1) return; //если для страницы выполняется свойство

    qInfo(logInfo()) << "Page " << pPage->formElementsToString() << " has more than 2n elements. Restoring tree.";

    qDebug(logDebug()) << "Parent page = " << ((pPage->pParentPage == nullptr) ? "nullptr" : pPage->pParentPage->formElementsToString());

    TreePage* newPage;
    if(pPage->pParentPage == nullptr) //если текущая страница - корень
    {
        TreePage* newRoot = new TreePage(nullptr, pPage->n, nullptr);
        newPage = new TreePage(nullptr, pPage->n, newRoot);
        pPage->pParentPage = newRoot;
        root = newRoot;

        int middleElement = pPage->elements[n];
        qInfo(logInfo()) << "Middle element is," << middleElement <<", adding it to the parent page" << newRoot->formElementsToString();
        addElementToPage(middleElement, newRoot);
        pPage->elements[n] = -10000;
        pPage->elementsCount--;

        newRoot->arrPDescendants[0] = pPage;
        newRoot->arrPDescendants[1] = newPage;
        newRoot->descendantsCount = 2;

        qDebug(logDebug()) << "Родитель newPage"<< newPage->formElementsToString() << "= " << ((newPage->pParentPage == nullptr) ? "nullptr" : newPage->pParentPage->formElementsToString());
        qDebug(logDebug()) << "Родитель pPage "<< pPage->formElementsToString() << "= " << ((pPage->pParentPage == nullptr) ? "nullptr" : pPage->pParentPage->formElementsToString());

        for(int i = n + 1; i < 2 * n + 1; i++)
        {
            int tmp = pPage->elements[i];
            pPage->elements[i] = -10000;
            pPage->elementsCount--;
            addElementToPage(tmp, newPage);
        }
    }
    else
    {
        TreePage* pParent = pPage->pParentPage;
        newPage = new TreePage(nullptr, pPage->n, pParent);
        qDebug(logDebug()) << "Родитель newpage "<< newPage->formElementsToString() << "= " << ((newPage->pParentPage == nullptr) ? "nullptr" : newPage->pParentPage->formElementsToString());

        int middleElement = pPage->elements[n];
        qInfo(logInfo()) << "Middle element is," << middleElement <<", adding it to the parent page" << pParent->formElementsToString();
        int index = addElementToPage(middleElement, pParent);
        pPage->elements[n] = -10000;
        pPage->elementsCount--;

        for(int i = n + 1; i <= 2 * n; i++)
        {
            int tmp = pPage->elements[i];
            pPage->elements[i] = -10000;
            pPage->elementsCount--;
            addElementToPage(tmp, newPage);
        }

        //добавляем новую страницу в массив потомков страницы-родителя
        qDebug(logCritical()) << "Сравниваем newPage->elements[0] = " << newPage->elements[0] << "с" << pParent->elements[index];
        if(newPage->elements[0] > pParent->elements[index])
        {
            index++;
        }

        //смещаем все потомки на один индекс вправо и добавляем нового потомка
        for(int i = pParent->descendantsCount - 1; i >= index + 1; i--) //TODO: исправить
        {
            pParent->arrPDescendants[i+1] = pParent->arrPDescendants[i];
        }
        pParent->arrPDescendants[index] = newPage;
        pParent->descendantsCount++;
    }

    if(pPage->descendantsCount != 0) //если это не лист, то потомки тоже надо делить между страницами
    {
        if(pPage->descendantsCount > n + 1)
        {
            qInfo(logInfo()) << "Делим потомки страницы";
            for(int i = n + 2; i < 2 * n + 2; i++)
            {
                if(pPage->arrPDescendants[i] != nullptr) //TODO: как-то исправить тут сравнение
                {
                    for(int j = 0; j < newPage->elementsCount; j++)
                    {
                        qDebug(logCritical()) << "Сравниваем newPage->elements[0] = " << newPage->elements[j] << "с" << pPage->arrPDescendants[i]->elements[0];
                        if(newPage->elements[j] > pPage->arrPDescendants[i]->elements[0])
                        {
                            pPage->arrPDescendants[i]->pParentPage = newPage;
                            newPage->arrPDescendants[j] = pPage->arrPDescendants[i];
                            pPage->arrPDescendants[i] = nullptr;
                            newPage->descendantsCount++;
                            pPage->descendantsCount--;
                            qInfo(logInfo()) << "Добавляем страницу " << newPage->arrPDescendants[j]->formElementsToString() << "как потомок страницы" << newPage->formElementsToString() << "с индексом" << j;
                        }
                        else if (j == newPage->elementsCount - 1)
                        {
                            pPage->arrPDescendants[i]->pParentPage = newPage;
                            newPage->arrPDescendants[j + 1] = pPage->arrPDescendants[i];
                            pPage->arrPDescendants[i] = nullptr;
                            newPage->descendantsCount++;
                            pPage->descendantsCount--;
                            qInfo(logInfo()) << "Добавляем страницу " << newPage->arrPDescendants[j]->formElementsToString() << "как потомок страницы" << newPage->formElementsToString() << "с индексом" << j + 1;
                        }
                        else
                        {
                            //newPage->descendantsCount++;
                            newPage->arrPDescendants[j] = nullptr;
                            qDebug(logCritical()) << "<= -> пропускаем, j = " << j;
                            continue;
                        }
                    }
                }
            }
        }
    }


    qDebug(logDebug()) << "Потомки страницы pPage->pParentPage" << pPage->pParentPage->formElementsToString() << "по порядку:";
    for(int i = 0; i < 2*n+1; i++)
    {
        if(pPage->pParentPage->arrPDescendants[i] == nullptr) continue;;
        qDebug(logDebug()) << "i = " << i << ":" << pPage->pParentPage->arrPDescendants[i]->formElementsToString();
    }

    qDebug(logDebug()) << "Потомки страницы pPage" << pPage->formElementsToString() << "по порядку:";
    for(int i = 0; i < 2*n+1; i++)
    {
        if(pPage->arrPDescendants[i] == nullptr) continue;;
        qDebug(logDebug()) << "i = " << i << ":" << pPage->arrPDescendants[i]->formElementsToString();
    }

    qDebug(logDebug()) << "Потомки страницы newPage" << newPage->formElementsToString() << "по порядку:";
    for(int i = 0; i < 2*n+1; i++)
    {
        if(newPage->arrPDescendants[i] == nullptr) continue;;
        qDebug(logDebug()) << "i = " << i << ":" << newPage->arrPDescendants[i]->formElementsToString();
    }

     recountNeededSpace(newPage);
     recountNeededSpace(pPage);
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

void Tree::repaintTree(TreePage *pPage, int x, int y)
{
    qInfo(logInfo()) << "Repainting tree (page " << pPage->formElementsToString() << ")";

    if(pPage == nullptr) return;

    QString str = pPage->formElementsToString();

    if(pPage->pParentPage == nullptr)
    {
        scene->clear();
    }

    int width = 25 + 6 * str.length();
    int height = 30;
    qDebug(logDebug()) << "x = " << x << "; y = " << y << "; width = " << width << "; height = " << height;
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(x, y, width, height, nullptr);
    rectItem->setPen(QPen(Qt::black));
    rectItem->setBrush(QBrush(Qt::white));
    scene->addItem(rectItem);

    QGraphicsTextItem *textItem = new QGraphicsTextItem(str, rectItem);
    textItem->setPos(x, y);
    textItem->setTextWidth(textItem->boundingRect().width());

    int offsetLeft = 0;
    for(int i = 0; i < 2 * n + 1; i++)
    {
        if(pPage->arrPDescendants[i] == nullptr) continue;
        repaintTree(pPage->arrPDescendants[i], x - pPage->needsSpace/2 + offsetLeft, y + 100);
        offsetLeft += pPage->arrPDescendants[i]->needsSpace;
    }
}

void Tree::recountNeededSpace(TreePage* ptr)
{
    qInfo(logInfo()) << "Пересчитываем пространство для страницы " << ptr->formElementsToString();
    ptr->needsSpace = 0;
    if(ptr->descendantsCount == 0)
    {
        ptr->needsSpace = 100;
    }

    for(int i = 0; i < 2 * n + 1; i++)
    {
        if(ptr->arrPDescendants[i] == nullptr) continue;
        ptr->needsSpace += ptr->arrPDescendants[i]->needsSpace;
    }
    qInfo(logInfo()) << "Пересчёт закончен. Страница " << ptr->formElementsToString() << "занимает " << ptr->needsSpace << "места";
    if(ptr->pParentPage != nullptr) recountNeededSpace(ptr->pParentPage);
}
