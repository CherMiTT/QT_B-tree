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
    //TODO: сейчас тут заглушка, реализовать
    return nullptr;
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
        page->descendants.insert(index + 1, nullptr);
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
                            root->formElementsToString() << ", её потомки: " << page->formElementsToString() << " и " << newPage->formElementsToString();
    }
    else //Если делили не корень
    {
        TreePage* parent = page->parentPage;
        newPage->parentPage = parent;
        int index = addElementToPage(parent, middle);
        parent->descendants.emplace(index + 1, newPage);
        qInfo(logInfo()) << "Родитель этой страницы теперь: " << parent->formElementsToString();

        qDebug(logDebug()) << "Потомки этого родителя:";
        for(auto i = parent->descendants.constBegin(); i != parent->descendants.constEnd(); i++)
        {
            if(*i == nullptr) continue;
            qDebug(logDebug()) << (*i)->formElementsToString();
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

    qInfo(logInfo()) << "Получились страницы " << page1.formElementsToString() << " и " << page2.formElementsToString();
    qInfo(logInfo()) << "Средний элемент = " << middle;
    return middle;
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
                                                            page->parentPage->rect->boundingRect().bottom(), rectItem);
    }

    int offsetLeft = 0;
    for(auto i = page->descendants.constBegin(); i != page->descendants.constEnd(); i++)
    {
        if(*i == nullptr) continue;
        repaintTree(*i, x - page->needsSpace/2 + offsetLeft, y + 100);
        offsetLeft += (*i)->needsSpace;
        //scene->addLine(x, y + 30, x - page->needsSpace/2 + offsetLeft, y + 100);
    }

}
