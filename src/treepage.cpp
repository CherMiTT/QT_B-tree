/*!
* \file
* \brief Source file for treePage class
* \author CherMiTT
* \date	12.04.2021
* \version 2.0
*/

#include "treepage.h"


/*!
 * \brief Конструктор страницы
 * \param parent - родительский виджеи
 * \param parentPage - родительская страница в структуре дерева
 * \param n - порядок дерева
 */
TreePage::TreePage(QWidget *parent, TreePage* parentPage, size_t n) : QWidget(parent)
{
    elementsCount = 0;
    descendantsCount = 0;
    this->parentPage = parentPage;
    elements.reserve(2 * n + 1); //На один больше, чем надо
    descendands.reserve(2 * n + 2); //На один больше, чем надо
}

/*!
 * \brief Формирует элементы страницы в строку
 * \return Строку с элемнтами страницы через пробел
 */
QString TreePage::formElementsToString()
{
    QString str = "";
    for(auto i = elements.constBegin(); i != elements.constEnd(); i++)
    {
        //if(*i == NOT_VALUE) continue;
        str.append(QString::number(*i));
        str.append(' ');
    }
    str = str.trimmed();
    return str;
}

/*!
 * \brief Сортирует страницу по возрастанию
 */
void TreePage::sort()
{
    std::sort(elements.begin(), elements.end());
}
