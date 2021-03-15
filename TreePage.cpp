#include "TreePage.h"

#include <QColor>


TreePage::TreePage(QWidget *parent, int order, TreePage *pParentPage) : QWidget(parent)
{
    this->n = order;
    this->pParentPage = pParentPage;
    this->elements = new int[n * 2 + 1]; //на 1 больше
    this->elementsCount = 0;
    this->arrPDescendants = new TreePage*[n * 2 + 2]; //на 1 больше
    this->descendantsCount = 0;

    for(int i = 0; i < 2 * n; i++)
    {
        elements[i] = -10000; //TODO: исправить
        arrPDescendants[i] = nullptr;
    }
    arrPDescendants[n + 2] = nullptr;

    needsSpace = 100;
}

//TODO: исправить
void TreePage::sort() //Сортировка пузырьком по возрастанию
{
    qInfo(logInfo()) << "Сортируем элементы на странице" + formElementsToString();
    int tmp;
    for(int i = 0; i < elementsCount - 1; i++)
    {
        for (int i = 0; i < elementsCount - 1; i++) {
            for (int j = 0; j < elementsCount - i - 1; j++) {
                if (elements[j] > elements[j + 1]) {
                    // меняем элементы местами
                    tmp = elements[j];
                    elements[j] = elements[j + 1];
                    elements[j + 1] = tmp;
                }
            }
        }

    }
    qInfo(logInfo()) << "лементы отсортированы. Результирующая страница: " + formElementsToString();
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
