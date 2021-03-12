#include "TreePage.h"

#include <QColor>


TreePage::TreePage(QWidget *parent, int order, TreePage *pParentPage) : QWidget(parent)
{
    this->n = order;
    this->pParentPage = pParentPage;
    this->elements = new int[n * 2];
    this->elementsCount = 0;
    this->arrPDescendants = new TreePage*[n * 2 + 1];
    this->descendantsCount = 0;

    for(int i = 0; i < 2 * n; i++)
    {
        elements[i] = -10000; //TODO: исправить
        arrPDescendants[i] = nullptr;
    }
    arrPDescendants[n + 2] = nullptr;
}

//TODO: исправить
void TreePage::sort() //Сортировка пузырьком по возрастанию
{
    qInfo(logInfo()) << "Sorting elements on page " + formElementsToString();
    int tmp;
    for(int i = 0; i < elementsCount; i++)
    {
        bool finished = true;
        for(int j = 0; j < elementsCount - (i + 1); j++)
        {
            if(elements[j] > elements[j + 1]) //TODO: добавить проверку на несуществующий элемент
            {
                finished = false;
                tmp = elements[j];
                elements[j] = elements[i];
                elements[i] = tmp;
            }
        }
        if(finished) break;
    }
    qInfo(logInfo()) << "Elements sorted. Resulting page: " + formElementsToString();
}

QString TreePage::formElementsToString() //возвращает строку с элементами страницы, разделёнными пробелами
{
    QString str = "";
    for(int i = 0; i < this->elementsCount; i++)
    {
        str.append(QString::number(this->elements[i]));
        str.append(' ');
    }
    str = str.trimmed();
    return str;
}
