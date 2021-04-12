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
        //TODO: recountNeededSpace()
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

        //TODO: recountNeededSpace()
    }

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
        page->descendands.push_back(nullptr);
        page->descendands.push_back(nullptr);
        page->descendantsCount += 2;
    }
    else
    {
        page->descendands.insert(index + 1, nullptr);
        page->descendantsCount++;
    }

    qInfo(logInfo()) << "Элемент добавлен на страницу по индексу" << page->elements.indexOf(e) << ".";
    qInfo(logInfo()) << "Результирующая страница: " << page->formElementsToString();
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
    while(ptr->descendantsCount != ptr->descendands.count(nullptr)) //Просматриваем, пока не находим лист
    {
        for(auto i = ptr->elements.constBegin(); i != ptr->elements.constEnd(); i++) //Цикл по всем элементам страницы
        {
            if(e < *i)
            {
                ptr = ptr->descendands[ptr->elements.indexOf(*i)];
                if(ptr == nullptr) qCritical(logCritical()) << "Пытается перейти на nullptr как левый потомок элемента" << *i;
                break;
            }
            if((i + 1) == ptr->elements.constEnd() && e > *i) //Если нет следующего элемента в странице, а новый элемент больше последнего
            {
                ptr = ptr->descendands[ptr->elements.indexOf(*i) + 1]; //Тогда переходим к самому последнему потомку
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
        newRoot->descendands.push_back(page);
        newRoot->descendands.push_back(newPage);
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
        parent->descendands.emplace(index + 1, newPage);
        qInfo(logInfo()) << "Родитель этой страницы теперь: " << parent->formElementsToString();

        qDebug(logDebug()) << "Потомки этого родителя:";
        for(auto i = parent->descendands.constBegin(); i != parent->descendands.constEnd(); i++)
        {
            if(*i == nullptr) continue;
            qDebug(logDebug()) << (*i)->formElementsToString();
        }
    }
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
    QList<TreePage*> temp = page1.descendands.sliced(n + 1);
    page2.descendands = QList<TreePage*>(temp);
    page2.descendands.reserve(2 * n + 2);
    page2.descendantsCount = page2.descendands.size();
    page1.descendands.remove(n + 1, n + 1);
    page1.descendantsCount = page1.descendands.size();

    qInfo(logInfo()) << "Получились страницы " << page1.formElementsToString() << " и " << page2.formElementsToString();
    qInfo(logInfo()) << "Средний элемент = " << middle;
    return middle;
}
