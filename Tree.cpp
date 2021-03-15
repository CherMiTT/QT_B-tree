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
    qInfo(logInfo()) << "Вошли в деструктор дерева.";
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
    qInfo(logInfo()) << "Удаляем страницу " + page->formElementsToString() + "и всех её потомков.";
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
    qInfo(logInfo()) << "Добавляем элемент " + QString::number(e) + " в дерево.";
    if(root == nullptr) //если это первый элемент в дереве
    {
        qInfo(logInfo()) << "Это первый элемент дерева. Создаём корневую страницу.";
        root = new TreePage(nullptr, n, nullptr); //создаём корневую страницу
        addElementToPage(e, root); //добавляем первый элемент
        recountNeededSpace(root);
    }
    else
    {
        qInfo(logInfo()) << "Ищем, куда добавить этот элемент";
        TreePage *ptr = root;
        while(ptr->descendantsCount != 0) //просматриваем, пока не находим лист
        {
            for(int i = 0; i < 2 * n; i++) //цикл по всем элементам страницы
            {
                if(ptr->elements[i] != -10000) //если этот элемент есть
                {
                    if(e < ptr->elements[i])
                    {
                        //Debug log
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
                        ptr = ptr->arrPDescendants[i];
                        break;
                    }
                    if (ptr->elementsCount - 1 == i && e > ptr->elements[i]) //если нет следующего элемента в странице, а новый элемент больше последнего
                    {
                        //Debug log
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
    qInfo(logInfo()) << "Добавляем элемент на страницу " + pPage->formElementsToString();
    pPage->elements[pPage->elementsCount++] = e; //Добавить элемент на страницу и увеличить кол-во элементов
    pPage->sort();
    for(int i = 0; i < pPage->elementsCount; i++)
    {
        if(e == pPage->elements[i])
        {
            qInfo(logInfo()) << "Элемент добавлен на страницу по индексу" << i << ".";
            return i;
        }
    }
}

void Tree::restoreTree(TreePage *pPage) //восстанавливает свойство дерева, если на какой-то странице больше 2n элементов
{
    if (pPage->elementsCount < 2*n + 1) return; //если для страницы выполняется свойство

    qInfo(logInfo()) << "На странице " << pPage->formElementsToString() << " больше 2n элементов. Восстаналвиваем свойства дерева.";

    TreePage* newPage;
    if(pPage->pParentPage == nullptr) //если текущая страница - корень
    {
        TreePage* newRoot = new TreePage(nullptr, pPage->n, nullptr);
        newPage = new TreePage(nullptr, pPage->n, newRoot);
        pPage->pParentPage = newRoot;
        root = newRoot;

        int middleElement = pPage->elements[n];
        qInfo(logInfo()) << "Медианный элемент =" << middleElement <<", добавляем его на страницу-родитель" << newRoot->formElementsToString();
        addElementToPage(middleElement, newRoot);
        pPage->elements[n] = -10000;
        pPage->elementsCount--;

        newRoot->arrPDescendants[0] = pPage;
        newRoot->arrPDescendants[1] = newPage;
        newRoot->descendantsCount = 2;

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

        int middleElement = pPage->elements[n];
        qInfo(logInfo()) << "Медианный элемент =" << middleElement <<", добавляем его на страницу-родитель" << pParent->formElementsToString();
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
        if(pPage->descendantsCount > pPage->elementsCount + 1)
        {
            qInfo(logInfo()) << "Делим потомки страницы.";
            for(int i = pPage->elementsCount + 1; i < 2 * n + 2; i++)
            {
                if(pPage->arrPDescendants[i] != nullptr)
                {
                    for(int j = 0; j < newPage->elementsCount; j++)
                    {
                        if(newPage->elements[j] > pPage->arrPDescendants[i]->elements[0])
                        {
                            pPage->arrPDescendants[i]->pParentPage = newPage;
                            newPage->arrPDescendants[j] = pPage->arrPDescendants[i];
                            pPage->arrPDescendants[i] = nullptr;
                            newPage->descendantsCount++;
                            pPage->descendantsCount--;
                            qInfo(logInfo()) << "Добавляем страницу " << newPage->arrPDescendants[j]->formElementsToString() << "как потомок страницы" << newPage->formElementsToString() << "с индексом" << j;
                            break;
                        }
                        else if (j == newPage->elementsCount - 1)
                        {
                            pPage->arrPDescendants[i]->pParentPage = newPage;
                            newPage->arrPDescendants[j + 1] = pPage->arrPDescendants[i];
                            pPage->arrPDescendants[i] = nullptr;
                            newPage->descendantsCount++;
                            pPage->descendantsCount--;
                            qInfo(logInfo()) << "Добавляем страницу !!" << newPage->arrPDescendants[j]->formElementsToString() << "как потомок страницы" << newPage->formElementsToString() << "с индексом" << j + 1;
                            break;
                        }
                        else
                        {
                            newPage->descendantsCount++;
                            //qDebug(logCritical()) << "<= -> пропускаем, j = " << j;
                            continue;
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }

     recountNeededSpace(newPage);
     recountNeededSpace(pPage);
}
void Tree::orderChanged(int newOrder)
{

}

TreePage* Tree::searchForElement(int e)
{
    qInfo(logInfo()) << "Ищем элемент " + QString::number(e) << ".";

    if(root == nullptr)
    {
        qInfo(logInfo()) << "Дерево ещё не создано. Элемента нет.";
        return nullptr;
    }

    TreePage* result = recursiveSearch(root, e);
    if(result == nullptr)
    {
        qInfo(logInfo()) << "Элемент не найден.";
    }
    else
    {
        qInfo(logInfo()) << "Элемент найден на странице" << result->formElementsToString() << ".";
    }
    return result;
}

TreePage* Tree::recursiveSearch(TreePage* ptr, int e)
{
    qInfo(logInfo()) << "Просматриваем страницу" << ptr->formElementsToString() << ".";
    if(ptr->descendantsCount != 0) //если просматриваем не лист
    {
        qInfo(logInfo()) << "Это не лист.";
        for(int i = 0; i < ptr->elementsCount; i++)
        {
            if(e == ptr->elements[i]) return ptr;
            if(e < ptr->elements[i])
            {
                return ptr->arrPDescendants[i] == nullptr ? nullptr : recursiveSearch(ptr->arrPDescendants[i], e);
            }
            if(i == ptr->elementsCount - 1)
            {
                return ptr->arrPDescendants[i + 1] == nullptr ? nullptr : recursiveSearch(ptr->arrPDescendants[i + 1], e);
            }
        }
    }
    else //если просматриваем лист
    {
        qInfo(logInfo()) << "Это лист.";
        for(int i = 0; i < ptr->elementsCount; i++)
        {
            if(e == ptr->elements[i]) return ptr;
        }
        return nullptr;
    }
}

void Tree::deleteElement(int e)
{

}

void Tree::repaintTree(TreePage *pPage, int x, int y)
{
    qInfo(logInfo()) << "Перерисовываем дерево (страница" << pPage->formElementsToString() << ").";

    if(pPage == nullptr) return;

    QString str = pPage->formElementsToString();

    if(pPage->pParentPage == nullptr)
    {
        scene->clear();
    }

    int width = 25 + 6 * str.length();
    int height = 30;
    //qDebug(logDebug()) << "x = " << x << "; y = " << y << "; width = " << width << "; height = " << height;
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
