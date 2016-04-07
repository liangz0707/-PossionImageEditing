/********************************************************************************
** Form generated from reading UI file 'cgwork.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CGWORK_H
#define UI_CGWORK_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <activelabel.h>

QT_BEGIN_NAMESPACE

class Ui_CGworkClass
{
public:
    QAction *openActionA;
    QAction *saveAction;
    QAction *exitAction;
    QAction *vignetAction;
    QAction *action1231;
    QAction *action;
    QAction *medianFilterAction;
    QAction *sharpenAction;
    QAction *openActionB;
    QWidget *centralWidget;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QTabWidget *vigTabWidget;
    QWidget *tab_2;
    QGroupBox *groupBox_10;
    QSpinBox *brushSpinBox;
    QLabel *label_7;
    QPushButton *PIEButton;
    QGroupBox *groupBox_12;
    QPushButton *PMButton;
    QPushButton *srcImgButton;
    QPushButton *dstImgButton;
    QPushButton *returnMaskButton;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    ActiveLabel *srcImgLabel;
    ActiveLabel *dstImgLabel;
    QMenuBar *menuBar;
    QMenu *menu;

    void setupUi(QMainWindow *CGworkClass)
    {
        if (CGworkClass->objectName().isEmpty())
            CGworkClass->setObjectName(QStringLiteral("CGworkClass"));
        CGworkClass->resize(1390, 700);
        openActionA = new QAction(CGworkClass);
        openActionA->setObjectName(QStringLiteral("openActionA"));
        saveAction = new QAction(CGworkClass);
        saveAction->setObjectName(QStringLiteral("saveAction"));
        exitAction = new QAction(CGworkClass);
        exitAction->setObjectName(QStringLiteral("exitAction"));
        vignetAction = new QAction(CGworkClass);
        vignetAction->setObjectName(QStringLiteral("vignetAction"));
        action1231 = new QAction(CGworkClass);
        action1231->setObjectName(QStringLiteral("action1231"));
        action = new QAction(CGworkClass);
        action->setObjectName(QStringLiteral("action"));
        medianFilterAction = new QAction(CGworkClass);
        medianFilterAction->setObjectName(QStringLiteral("medianFilterAction"));
        sharpenAction = new QAction(CGworkClass);
        sharpenAction->setObjectName(QStringLiteral("sharpenAction"));
        openActionB = new QAction(CGworkClass);
        openActionB->setObjectName(QStringLiteral("openActionB"));
        centralWidget = new QWidget(CGworkClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        horizontalLayoutWidget_2 = new QWidget(centralWidget);
        horizontalLayoutWidget_2->setObjectName(QStringLiteral("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(0, 0, 1391, 671));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        groupBox = new QGroupBox(horizontalLayoutWidget_2);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setStyleSheet(QStringLiteral(""));
        verticalLayoutWidget_2 = new QWidget(groupBox);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 20, 191, 401));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        vigTabWidget = new QTabWidget(verticalLayoutWidget_2);
        vigTabWidget->setObjectName(QStringLiteral("vigTabWidget"));
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        groupBox_10 = new QGroupBox(tab_2);
        groupBox_10->setObjectName(QStringLiteral("groupBox_10"));
        groupBox_10->setGeometry(QRect(0, 10, 171, 111));
        brushSpinBox = new QSpinBox(groupBox_10);
        brushSpinBox->setObjectName(QStringLiteral("brushSpinBox"));
        brushSpinBox->setGeometry(QRect(80, 80, 91, 22));
        brushSpinBox->setMinimum(1);
        brushSpinBox->setMaximum(40);
        brushSpinBox->setValue(12);
        label_7 = new QLabel(groupBox_10);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(20, 50, 101, 20));
        PIEButton = new QPushButton(groupBox_10);
        PIEButton->setObjectName(QStringLiteral("PIEButton"));
        PIEButton->setGeometry(QRect(80, 20, 91, 23));
        groupBox_12 = new QGroupBox(tab_2);
        groupBox_12->setObjectName(QStringLiteral("groupBox_12"));
        groupBox_12->setGeometry(QRect(10, 130, 171, 51));
        PMButton = new QPushButton(groupBox_12);
        PMButton->setObjectName(QStringLiteral("PMButton"));
        PMButton->setGeometry(QRect(70, 20, 91, 23));
        vigTabWidget->addTab(tab_2, QString());

        verticalLayout_2->addWidget(vigTabWidget);

        srcImgButton = new QPushButton(verticalLayoutWidget_2);
        srcImgButton->setObjectName(QStringLiteral("srcImgButton"));

        verticalLayout_2->addWidget(srcImgButton);

        dstImgButton = new QPushButton(verticalLayoutWidget_2);
        dstImgButton->setObjectName(QStringLiteral("dstImgButton"));

        verticalLayout_2->addWidget(dstImgButton);

        returnMaskButton = new QPushButton(verticalLayoutWidget_2);
        returnMaskButton->setObjectName(QStringLiteral("returnMaskButton"));

        verticalLayout_2->addWidget(returnMaskButton);

        horizontalLayoutWidget = new QWidget(groupBox);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(210, 10, 1171, 651));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        srcImgLabel = new ActiveLabel(horizontalLayoutWidget);
        srcImgLabel->setObjectName(QStringLiteral("srcImgLabel"));
        srcImgLabel->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(srcImgLabel);

        dstImgLabel = new ActiveLabel(horizontalLayoutWidget);
        dstImgLabel->setObjectName(QStringLiteral("dstImgLabel"));
        dstImgLabel->setStyleSheet(QStringLiteral(""));

        horizontalLayout->addWidget(dstImgLabel);


        horizontalLayout_2->addWidget(groupBox);

        CGworkClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CGworkClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1390, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QStringLiteral("menu"));
        CGworkClass->setMenuBar(menuBar);

        menuBar->addAction(menu->menuAction());
        menu->addAction(openActionA);
        menu->addAction(openActionB);
        menu->addAction(saveAction);
        menu->addSeparator();
        menu->addAction(exitAction);

        retranslateUi(CGworkClass);

        vigTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(CGworkClass);
    } // setupUi

    void retranslateUi(QMainWindow *CGworkClass)
    {
        CGworkClass->setWindowTitle(QApplication::translate("CGworkClass", "CGwork", 0));
        openActionA->setText(QApplication::translate("CGworkClass", "\346\211\223\345\274\200\345\233\276\345\203\217A", 0));
        saveAction->setText(QApplication::translate("CGworkClass", "\344\277\235\345\255\230\347\273\223\346\236\234", 0));
        exitAction->setText(QApplication::translate("CGworkClass", "\351\200\200\345\207\272", 0));
        vignetAction->setText(QApplication::translate("CGworkClass", "Vignetting", 0));
        action1231->setText(QApplication::translate("CGworkClass", "1231", 0));
        action->setText(QApplication::translate("CGworkClass", "\347\231\275\345\271\263\350\241\241", 0));
        medianFilterAction->setText(QApplication::translate("CGworkClass", "\344\270\255\345\200\274\346\273\244\346\263\242", 0));
        sharpenAction->setText(QApplication::translate("CGworkClass", "\351\224\220\345\214\226", 0));
        openActionB->setText(QApplication::translate("CGworkClass", "\346\211\223\345\274\200\345\233\276\345\203\217B", 0));
        groupBox->setTitle(QApplication::translate("CGworkClass", "\346\223\215\344\275\234\351\235\242\346\235\277", 0));
        groupBox_10->setTitle(QApplication::translate("CGworkClass", "possion image editing", 0));
        label_7->setText(QApplication::translate("CGworkClass", "\347\224\273\347\254\224\345\215\212\345\276\204\357\274\210\345\203\217\347\264\240\357\274\211\357\274\232", 0));
        PIEButton->setText(QApplication::translate("CGworkClass", "RUN", 0));
        groupBox_12->setTitle(QApplication::translate("CGworkClass", "patchmatch", 0));
        PMButton->setText(QApplication::translate("CGworkClass", "RUN", 0));
        vigTabWidget->setTabText(vigTabWidget->indexOf(tab_2), QApplication::translate("CGworkClass", "\351\253\230\347\272\247\345\244\204\347\220\206", 0));
        srcImgButton->setText(QApplication::translate("CGworkClass", "\346\230\276\347\244\272\345\216\237\345\233\276", 0));
#ifndef QT_NO_TOOLTIP
        dstImgButton->setToolTip(QApplication::translate("CGworkClass", "<html><head/><body><p>\345\234\250opencv\344\270\255\346\230\276\347\244\272\345\233\276\347\211\207\357\274\214\345\217\257\344\273\245\345\234\250\350\261\241\347\264\240\347\272\247\345\210\253\346\237\245\347\234\213\345\233\276\345\203\217\347\202\271</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        dstImgButton->setText(QApplication::translate("CGworkClass", "\346\230\276\347\244\272\347\273\223\346\236\234", 0));
        returnMaskButton->setText(QApplication::translate("CGworkClass", "\346\222\244\351\224\200\346\250\241\346\235\277", 0));
        srcImgLabel->setText(QApplication::translate("CGworkClass", "\345\212\240\350\275\275\345\233\276\347\211\207A", 0));
        dstImgLabel->setText(QApplication::translate("CGworkClass", "\345\212\240\350\275\275\345\233\276\347\211\207B", 0));
        menu->setTitle(QApplication::translate("CGworkClass", "\346\226\207\344\273\266 ", 0));
    } // retranslateUi

};

namespace Ui {
    class CGworkClass: public Ui_CGworkClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CGWORK_H
