/********************************************************************************
** Form generated from reading UI file 'activelabel.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACTIVELABEL_H
#define UI_ACTIVELABEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_ActiveLabel
{
public:

    void setupUi(QLabel *ActiveLabel)
    {
        if (ActiveLabel->objectName().isEmpty())
            ActiveLabel->setObjectName(QStringLiteral("ActiveLabel"));
        ActiveLabel->resize(400, 300);

        retranslateUi(ActiveLabel);

        QMetaObject::connectSlotsByName(ActiveLabel);
    } // setupUi

    void retranslateUi(QLabel *ActiveLabel)
    {
        ActiveLabel->setWindowTitle(QApplication::translate("ActiveLabel", "ActiveLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class ActiveLabel: public Ui_ActiveLabel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACTIVELABEL_H
