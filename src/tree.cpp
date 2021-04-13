/*!
* \file
* \brief Source file for tree class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#include "tree.h"

/*!
 * \brief Конструктор дерева; ставит roor = nullptr, elementCount = 0;
 * \param parent - родительский элемент, по умолчанию nullptr
 * \param n - порядок дерева, по умполчанию 0
 */
Tree::Tree(QWidget *parent, size_t n) : QWidget(parent)
{
    this->n = n;
    root = nullptr;
    elementCount = 0;
}

/*!
 * \brief Ищет элемент e, рекурсивно просматривая потомков страницы page
 * \param page - текущая страница
 * \param e - искомый элемент
 * \return nullptr, если не нашло e, указатель на станицу, если нашло
 */
TreePage *Tree::recursiveSearch(TreePage* page, int e)
{
    qInfo(logInfo()) << "Просматриваем страницу" << page->formElementsToString() << ".";
    if(page->descendantsCount != page->descendants.count(nullptr)) //если просматриваем не лист
    {
        qInfo(logInfo()) << "Это не лист.";
        //for(int i = 0; i < page->elementsCount; i++)
        for(auto i : page->elements)
        {
            if(e == i) return page;
            if(e < i)
            {
                return page->descendants[page->elements.indexOf(i)] == nullptr ? nullptr : recursiveSearch(page->descendants[page->elements.indexOf(i)], e);
            }
            if(i == page->elements.last())
            {
                return page->descendants.last() == nullptr ? nullptr : recursiveSearch(page->descendants.last(), e);
            }
        }
    }
    else //если просматриваем лист
    {
        qInfo(logInfo()) << "Это лист.";
        for(int i = 0; i < page->elementsCount; i++)
        {
            if(e == page->elements[i]) return page;
        }
        return nullptr;
    }

}


/*!
 * \brief Слот, вызываемый при изменении порядка дерева.
 * Меняет порядок n и выдаёт сигнал rebuildTree
 * \param newOrder - новый порядок дерева
 */
void Tree::orderChanged(int newOrder)
{
    n = newOrder;
    emit rebuildTree();
}

/*!
 * \brief Слот, вызываемый при добавлении элемента в дерево.
 * Ищет страницу, куда добавить, и вызывает метод добавления.
 * \param e - новый элемент
 */
void Tree::addElement(int e)
{
    qInfo(logInfo()) << "Добавляем элемент " + QString::number(e) + " в дерево.";
    elementCount++;
    if(root == nullptr) //Если это первый элемент в дереве
    {
        qInfo(logInfo()) << "Это первый элемент дерева. Создаём корневую страницу.";
        root = new TreePage(nullptr, nullptr, n);
        addElementToPage(root, e);
        recountNeededSpace(root);
    }
    else
    {
        TreePage* page = findPageWhereAdd(e);
        qInfo(logInfo()) << "Будем добавлять элемент на страницу " << page->formElementsToString();
        addElementToPage(page, e);

        //проверяем свойства дерева и перестраиваем (если надо), начиная с этой страницы и до корня, пока не начнут выполняться свойства
        while(page->elementsCount > 2 * n) //не выполнилось свойство после добавления элемента
        {
            if(page == root) //если это корень
            {
                restoreTree(root); //восстанавливаем свойство
                break;
            }
            else //если это не корень
            {
                restoreTree(page); //восстанавливаем свойство в странице
                page = page->parentPage; //проверяем родителя этой страницы
            }
        }
    }

    repaintTree(root, 0, 0);
    emit treeStructureChanged();
}

/*!
 * \brief Слот, вызываемый для поиска элемента в дереве
 * \param e - искомый элемнт
 * \return Возвращает nullptr, если элемента нет, и указатель на страницу с элементом, если есть
 */
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

/*!
 * \brief Слот, вызываемый для удаления элемента из дерева
 * \param page - страница, на которой находится элемент e
 * \param e - удаляемый элемент
 */
void Tree::deleteElement(TreePage* page, int e)
{
    if(page->descendantsCount == page->descendants.count(nullptr)) //Если лист
    {
        qInfo(logInfo()) << "Страница " << page->formElementsToString() << " является листом";
        deleteFromLeaf(page, e);
    }
    else
    {
        qInfo(logInfo()) << "Страница " << page->formElementsToString() << "не является листом";
        deleteFromNonLeaf(page, e);
    }
    repaintTree(root, 0, 0);
}

/*!
 * \brief Добавляет элемент e на страницу, проверка свойств не производится
 * \param page - страница, на которую добавляем
 * \param e - элемент, который добавляем
 * \return индекс элемента на странице
 */
int Tree::addElementToPage(TreePage *page, int e)
{
    qInfo(logInfo()) << "Добавляем элемент на страницу " + page->formElementsToString();

    page->elements.push_back(e);
    page->elementsCount++;
    page->sort();
    int index = page->elements.indexOf(e);
    //Потомков всегда m+1, где m - число элементов страницы
    if(page->descendantsCount == 0) //Если это пустая страница
    {
        page->descendants.push_back(nullptr);
        page->descendants.push_back(nullptr);
        page->descendantsCount += 2;
    }
    else
    {
        //page->descendants.insert(index, nullptr);
        page->descendants.push_back(nullptr); //TODO: проверить место
        page->descendantsCount++;
    }

    qInfo(logInfo()) << "Элемент добавлен на страницу по индексу" << page->elements.indexOf(e) << ".";
    qInfo(logInfo()) << "Результирующая страница: " << page->formElementsToString();
    recountNeededSpace(page);
    return index;
}

/*!
 * \brief Находит страницу, на которую надо добавить элемент
 * \param e - добавляемый элемент
 * \return указатель на найденную страницу
 */
TreePage *Tree::findPageWhereAdd(int e)
{
    TreePage *ptr = root;
    while(ptr->descendantsCount != ptr->descendants.count(nullptr)) //Просматриваем, пока не находим лист
    {
        for(auto i = ptr->elements.constBegin(); i != ptr->elements.constEnd(); i++) //Цикл по всем элементам страницы
        {
            if(e < *i)
            {
                ptr = ptr->descendants[ptr->elements.indexOf(*i)];
                if(ptr == nullptr) qCritical(logCritical()) << "Пытается перейти на nullptr как левый потомок элемента" << *i;
                break;
            }
            if((i + 1) == ptr->elements.constEnd() && e > *i) //Если нет следующего элемента в странице, а новый элемент больше последнего
            {
                ptr = ptr->descendants[ptr->elements.indexOf(*i) + 1]; //Тогда переходим к самому последнему потомку
                if(ptr == nullptr) qCritical(logCritical()) << "Пытается перейти на nullptr как правый потомок элемента" << *i;
                break;
            }
        }
    }
    return ptr;
}

/*!
 * \brief Восстанавливает свойства дерева, если на странице больше 2n элементов
 * \param page - страница с нарушенным свойством
 */
void Tree::restoreTree(TreePage* page)
{
    if (page->elementsCount < 2*n + 1) return; //если для страницы выполняется свойство
    qInfo(logInfo()) << "На странице " << page->formElementsToString() << " больше 2n элементов. Восстаналвиваем свойства дерева.";

    TreePage* newPage = new TreePage(nullptr, nullptr, n);
    int middle = splitPage(*page, *newPage);
    if(page == root) //Если делили корень, то он перестаёт быть корнем, надо создать новый
    {
        qInfo(logInfo()) << "Делили корень, поэтому создаём новый корень";
        TreePage* newRoot = new TreePage(nullptr, nullptr, n);
        page->parentPage = newRoot;
        newPage->parentPage = newRoot;
        newRoot->descendants.push_back(page);
        newRoot->descendants.push_back(newPage);
        newRoot->descendantsCount = 2;
        newRoot->elements.push_back(middle);
        newRoot->elementsCount = 1;
        root = newRoot;
        qInfo(logInfo()) << "Теперь корневая страница: " <<
                            root->formElementsToString() << ", её потомки: " << root->descendants.indexOf(page) << ":"
                            << page->formElementsToString() << " и " <<  root->descendants.indexOf(newPage) << ":"
                            << newPage->formElementsToString();
    }
    else //Если делили не корень
    {
        TreePage* parent = page->parentPage;
        newPage->parentPage = parent;
        int index = addElementToPage(parent, middle);
        parent->descendants.emplace(index + 1, newPage);

        /*if(parent->descendants.at(index + 2) == nullptr)
        {
            parent->descendants.remove(index + 2);
        }*/
        parent->descendants.removeAll(nullptr);

        qInfo(logInfo()) << "Родитель этой страницы теперь: " << parent->formElementsToString();

        qDebug(logDebug()) << "Потомки этого родителя:";
        for(auto i = parent->descendants.constBegin(); i != parent->descendants.constEnd(); i++)
        {
            if(*i == nullptr) continue;
            qDebug(logDebug()) << "Индекс = " << parent->descendants.indexOf(*i) << ": " << (*i)->formElementsToString();
        }
    }
    recountNeededSpace(page);
    recountNeededSpace(newPage);
}

/*!
 * \brief Делит страницу на две при переполнении
 * \param page1 Делимая страница, станет левой страницей (по ссылке)
 * \param page2 Новая страница, станет правой страницей (по ссылке)
 * \return Срединный элемент, который должен уйти на уровень выше
 */
int Tree::splitPage(TreePage& page1, TreePage& page2)
{
    qInfo(logInfo()) << "Делим страницу " << page1.formElementsToString();
    int middle = page1.elements[n]; //срединный элемент

    //Переносим элементы на новую страницу
    QList<int> tmp = page1.elements.sliced(n+1); //элементы, которые уходят в новую страницу
    page2.elements = QList<int>(tmp);
    page2.elements.reserve(2 * n + 1);
    page2.elementsCount = page2.elements.size();
    page1.elements.remove(n, n + 1);
    page1.elementsCount = page1.elements.size();

    //Переносим потомков на новую страницу
    QList<TreePage*> temp = page1.descendants.sliced(n + 1);
    page2.descendants = QList<TreePage*>(temp);
    page2.descendants.reserve(2 * n + 2);
    page2.descendantsCount = page2.descendants.size();
    page1.descendants.remove(n + 1, n + 1);
    page1.descendantsCount = page1.descendants.size();
    for(auto i = page2.descendants.begin(); i != page2.descendants.end(); i++)
    {
        if(*i == nullptr) continue;
        (*i)->parentPage = &page2;
    }

    /*qDebug(logDebug()) << "У страницы " << page1.formElementsToString() << "потомки:";
    for(auto i = page1.descendants.constBegin(); i != page1.descendants.constEnd(); i++)
    {
        if(*i == nullptr) continue;
        qDebug() << (*i)->formElementsToString();
    }
    qDebug(logDebug()) << "У страницы " << page2.formElementsToString() << "потомки:";
    for(auto i = page2.descendants.constBegin(); i != page2.descendants.constEnd(); i++)
    {
        if(*i == nullptr) continue;
        qDebug() << (*i)->formElementsToString();
    }
    qDebug(logDebug());*/
    qInfo(logInfo()) << "Получились страницы " << page1.formElementsToString() << " и " << page2.formElementsToString();
    qInfo(logInfo()) << "Средний элемент = " << middle;
    return middle;
}

/*!
 * \brief Функция удаления элемента со страницы-листа
 * \param page - страница, с которой удаляем
 * \param e - удаляемый элемень
 */
void Tree::deleteFromLeaf(TreePage* page, int e)
{
    qInfo(logInfo()) << "На странице " << page->formElementsToString() << " " << page->elementsCount << "элементов";

    if(page == root)
    {
        qInfo(logInfo()) << "Страница является корнем дерева";
        page->elements.remove(page->elements.indexOf(e));
        page->elementsCount -= 1;
        //TODO: проверить, надо ли удалять потомка и если да, то где потом добавлять
        page->descendants.removeLast();
        page->descendantsCount -= 1;
        qInfo(logInfo()) << "Элемент удалён из корня";
    }
    else
    {
        qInfo(logInfo()) << "Страница не является корнем дерева";
        page->elements.remove(page->elements.indexOf(e));
        page->elementsCount -= 1;
        page->descendants.removeLast();
        page->descendantsCount -= 1;
        qInfo(logInfo()) << "Элемент удалён из листа";
        qInfo(logInfo()) << "В листе осталось " << page->elementsCount << "элементов";
        if(page->elementsCount >= n)
        {
            qInfo(logInfo()) << "Выполняются свойства дерева, элемент успешно удалён";
            return;
        }
        else
        {
            qInfo(logInfo()) << "В листе" << page->formElementsToString() << " теперь < n элементов, нарушено свойство.";
            repairUnderflow(page);
        }
    }
}

/*!
 * \brief Восстанавливает свойства дерева, если на странице < n элементов
 * \param page - страница с нарушенным свойством
 */
void Tree::repairUnderflow(TreePage *page)
{
    if(page->descendantsCount == page->descendants.count(nullptr)) //Если лист
    {
        //Ищем соседнюю страницу
        //Если текущий лист самый правый, то возьмём левую. Если нет, то правую
        bool right;
        TreePage* neighbor;
        int parentSeparator = findNeighborPage(page, neighbor, right);

        qInfo(logInfo()) << "Наш лист " << page->formElementsToString() <<
                            "; взяли его соседа " << neighbor->formElementsToString() <<
                            "; в родительской странице их разделяет элемент" << parentSeparator;

        if(neighbor->elementsCount == n) //Если из соседа нельзя удалить элементы, не нарушая свойства
        {
            //Сливаем текущий лист и соседа
            mergePages(page, neighbor, parentSeparator, right);
        }
        else //Балансируем текущий лист и соседа
        {
            balancePages(page, neighbor, parentSeparator, right);
        }
    }
    //TODO: else для не листа

}

int Tree::findNeighborPage(TreePage* page, TreePage* neighbor, bool& right)
{
    TreePage* parent = page->parentPage;
    int parentSeparator;
    if(parent->descendants.endsWith(page)) //Если текущий лист самый правый
    {
        neighbor = *(parent->descendants.cend() - 2); //Берём левого соседа, предпоследний потомок родителя
        parentSeparator = parent->elements.last(); //Ключ в родительской странице между page и neibor
        right = false;
    }
    else
    {
        int index = parent->descendants.indexOf(page); //Индекс листа в массиве потомков
        neighbor = parent->descendants.at(index + 1); //Берём правого соседа
        parentSeparator = parent->elements.at(index);
        right = true;
    }
    return parentSeparator;
}

/*!
 * \brief Балансирует количество элементов между страницами
 * \param page1 - первая страница
 * \param page2 - правая страница
 * \param parentSeparator - разделитель в родительской странице между страницами
 * \param right - true, если page2 справа от page1
 */
void Tree::balancePages(TreePage* page1, TreePage* page2, int parentSeparator, bool right)
{
    //TODO: тут не реализована балансировка, сделать как у Вирта
    qInfo(logInfo()) << "Балансируем страницы";
    if(right)
    {
        page1->elements.push_back(parentSeparator);
        page1->elementsCount += 1;
        page1->parentPage->elements.replace(page1->parentPage->elements.indexOf(parentSeparator),
                                            page2->elements.first());
        deleteElement(page2, page2->elements.first());
    }
    else
    {
        page1->elements.prepend(parentSeparator);
        page1->elementsCount += 1;
        page1->parentPage->elements.replace(page1->parentPage->elements.indexOf(parentSeparator),
                                            page2->elements.last());
        deleteElement(page2, page2->elements.last());
    }
    qInfo(logInfo()) << "Балансировка страниц завершена, в итоге наша страница: " <<
                        page1->formElementsToString() << "; соседняя страница: " <<
                        page2->formElementsToString() << "; родительская страница: " <<
                        page1->parentPage->formElementsToString();
}

/*!
 * \brief Сливает две страницы и разделитель между ними в родительской странице
 * \param page1 - первая страница, в неё сольётся
 * \param page2 - вторая страница, её удаляем
 * \param parentSeparator - разделитель в родительской странице между страницами
 * \param right - right - true, если page2 справа от page1
 */
void Tree::mergePages(TreePage* page1, TreePage* page2, int parentSeparator, bool right)
{
    qInfo(logInfo()) << "В соседней странице слишком мало элементов, поэтому сливаем страницы";
    //Тогда надо сделать новый корень
    if(right)
    {
        page1->elements.push_back(parentSeparator);
        page1->elements += page2->elements;
        //TODO: проверить с потомками
        page1->descendants += page2->descendants;
    }
    else
    {
        page1->elements.push_front(parentSeparator);
        for(auto i = page2->elements.rbegin(); i != page2->elements.rend(); i++)
        {
            page1->elements.push_front(*i);
        }
        //TODO: проверить с потомками
        for(auto i = page2->descendants.rbegin(); i != page2->descendants.rend(); i++)
        {
            page1->descendants.push_front(*i);
        }
    }

    page1->elementsCount = page1->elements.size();
    //TODO: проверить с потомками
    page1->descendantsCount = page1->descendants.size();

    page1->parentPage->elements.removeAt(page1->parentPage->elements.indexOf(parentSeparator));
    page1->parentPage->elementsCount -= 1;
    page1->parentPage->descendants.removeAt(page1->parentPage->descendants.indexOf(page2));
    page1->parentPage->descendantsCount -= 1;
    delete page2;

    if(page1->descendants.isEmpty())
    {
        for(int i = 0; i < page1->elementsCount + 1; i++)
        {
            page1->descendants.push_back(nullptr);
        }
        page1->descendantsCount = page1->elementsCount + 1;
    }

    if(page1->parentPage->elementsCount == 0) //Если удаляем родительский элемент
    {
        if(page1->parentPage == root)
        {
            delete root;
            page1->parentPage = nullptr;
            root = page1;
            for(auto i = root->descendants.begin(); i != root->descendants.end(); i++)
            {
                if(*i == nullptr) continue;
                (*i)->parentPage = root;
            }
        }
        else
        {
            auto ptr = page1->parentPage;
            auto parent = ptr->parentPage;
            page1->parentPage = parent;
            parent->descendants.replace(parent->descendants.indexOf(ptr), page1);
            delete ptr; //TODO: проверить
        }
    }
}


/*!
 * \brief Функция удаления элемента со страницы, которая не является листом
 * \param page - страница, с которой удаляем
 * \param e - удаляемый элемень
 */
void Tree::deleteFromNonLeaf(TreePage* page, int e)
{
    int index = page->elements.indexOf(e);
    auto descendant = page->descendants.at(index + 1); //Правый потомок, удаляем из него самый левый элемент
    int tmp = descendant->elements.first();
    page->elements.replace(index, tmp);
    qDebug(logDebug()) << tmp;
    deleteElement(descendant, tmp);

    //TODO: балансировка
}

/*!
 * \brief Подсчитывает, сколько занимает страница на экране
 * \param page - страница
 */

void Tree::recountNeededSpace(TreePage *page)
{
    qInfo(logInfo()) << "Пересчитываем пространство для страницы " << page->formElementsToString();

    //page->needsSpace = 0; //TODO: обнулять
    //qDebug(logDebug()) << "page->descendands.count(nullptr) = " << page->descendands.count(nullptr);
    if(page->descendantsCount != page->descendants.count(nullptr)) //если лист
    {
        page->needsSpace = 100;
    }

    for(auto i = page->descendants.constBegin(); i != page->descendants.constEnd(); i++) //цикл по всем потомкам
    {
        if(*i == nullptr) continue;
        page->needsSpace += (*i)->needsSpace;
    }
    qInfo(logInfo()) << "Пересчёт закончен. Страница " << page->formElementsToString() << "занимает " << page->needsSpace << "места";
    if(page->parentPage != nullptr) recountNeededSpace(page->parentPage);
}

/*!
 * \brief Перерисовывает страницу дерева и все её дочерние страницы
 * \param page - текущая страница
 * \param x - горизонтальная координата текущей страницы
 * \param y - вертикальная координата текущей страницы
 */
void Tree::repaintTree(TreePage* page, int x, int y)
{
    if(page == nullptr) return;
    qInfo(logInfo()) << "Перерисовываем дерево (страница" << page->formElementsToString() << ").";

    QString str = page->formElementsToString(); //Строка содержимого страницы

    if(page->parentPage == nullptr) //Если это корень, то стираем прошлое дерево
    {
        scene->clear();
    }

    int width = 25 + 6 * str.length(); //TODO: переделать, тут не учитывается шрифт
    int height = 30;

    QGraphicsRectItem *rectItem = new QGraphicsRectItem(x, y, width, height, nullptr);
    rectItem->setPen(QPen(Qt::black));
    rectItem->setBrush(QBrush(Qt::white));
    scene->addItem(rectItem);
    page->rect = rectItem;

    QGraphicsTextItem *textItem = new QGraphicsTextItem(str, rectItem);
    textItem->setPos(x, y);
    textItem->setTextWidth(textItem->boundingRect().width());

    if(page->parentPage != nullptr)
    {
        QGraphicsLineItem *lineItem = new QGraphicsLineItem(rectItem->boundingRect().center().x(),
                                                            rectItem->boundingRect().top(),
                                                            page->parentPage->rect->boundingRect().center().x(),
                                                            page->parentPage->rect->boundingRect().bottom());
        scene->addItem(lineItem);
    }

    int offsetLeft = 0;
    for(auto i = page->descendants.constBegin(); i != page->descendants.constEnd(); i++)
    {
        if(*i == nullptr) continue;
        repaintTree(*i, x - page->needsSpace/2 + offsetLeft, y + 100);
        offsetLeft += (*i)->needsSpace;
    }

}
