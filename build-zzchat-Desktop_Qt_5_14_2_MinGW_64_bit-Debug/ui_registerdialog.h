/********************************************************************************
** Form generated from reading UI file 'registerdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERDIALOG_H
#define UI_REGISTERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RegisterDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QWidget *err_widget;
    QHBoxLayout *horizontalLayout_7;
    QLabel *err_tip;
    QHBoxLayout *horizontalLayout;
    QLabel *user_label;
    QLineEdit *user_edit;
    QHBoxLayout *horizontalLayout_5;
    QLabel *email_label;
    QLineEdit *email_edit;
    QHBoxLayout *horizontalLayout_6;
    QLabel *pass_label;
    QLineEdit *pass_edit;
    QHBoxLayout *horizontalLayout_4;
    QLabel *confirm_label;
    QLineEdit *confirm_edit;
    QHBoxLayout *horizontalLayout_3;
    QLabel *varify_label;
    QLineEdit *varify_edit;
    QPushButton *get_code;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *confirm;
    QPushButton *concel;
    QSpacerItem *verticalSpacer_3;

    void setupUi(QDialog *RegisterDialog)
    {
        if (RegisterDialog->objectName().isEmpty())
            RegisterDialog->setObjectName(QString::fromUtf8("RegisterDialog"));
        RegisterDialog->resize(361, 416);
        verticalLayout_2 = new QVBoxLayout(RegisterDialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer = new QSpacerItem(20, 60, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        err_widget = new QWidget(RegisterDialog);
        err_widget->setObjectName(QString::fromUtf8("err_widget"));
        err_widget->setMinimumSize(QSize(20, 20));
        horizontalLayout_7 = new QHBoxLayout(err_widget);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        err_tip = new QLabel(err_widget);
        err_tip->setObjectName(QString::fromUtf8("err_tip"));
        err_tip->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(err_tip);


        verticalLayout->addWidget(err_widget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        user_label = new QLabel(RegisterDialog);
        user_label->setObjectName(QString::fromUtf8("user_label"));

        horizontalLayout->addWidget(user_label);

        user_edit = new QLineEdit(RegisterDialog);
        user_edit->setObjectName(QString::fromUtf8("user_edit"));

        horizontalLayout->addWidget(user_edit);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        email_label = new QLabel(RegisterDialog);
        email_label->setObjectName(QString::fromUtf8("email_label"));

        horizontalLayout_5->addWidget(email_label);

        email_edit = new QLineEdit(RegisterDialog);
        email_edit->setObjectName(QString::fromUtf8("email_edit"));

        horizontalLayout_5->addWidget(email_edit);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        pass_label = new QLabel(RegisterDialog);
        pass_label->setObjectName(QString::fromUtf8("pass_label"));

        horizontalLayout_6->addWidget(pass_label);

        pass_edit = new QLineEdit(RegisterDialog);
        pass_edit->setObjectName(QString::fromUtf8("pass_edit"));
        pass_edit->setReadOnly(false);

        horizontalLayout_6->addWidget(pass_edit);


        verticalLayout->addLayout(horizontalLayout_6);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        confirm_label = new QLabel(RegisterDialog);
        confirm_label->setObjectName(QString::fromUtf8("confirm_label"));

        horizontalLayout_4->addWidget(confirm_label);

        confirm_edit = new QLineEdit(RegisterDialog);
        confirm_edit->setObjectName(QString::fromUtf8("confirm_edit"));

        horizontalLayout_4->addWidget(confirm_edit);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        varify_label = new QLabel(RegisterDialog);
        varify_label->setObjectName(QString::fromUtf8("varify_label"));

        horizontalLayout_3->addWidget(varify_label);

        varify_edit = new QLineEdit(RegisterDialog);
        varify_edit->setObjectName(QString::fromUtf8("varify_edit"));

        horizontalLayout_3->addWidget(varify_edit);

        get_code = new QPushButton(RegisterDialog);
        get_code->setObjectName(QString::fromUtf8("get_code"));

        horizontalLayout_3->addWidget(get_code);


        verticalLayout->addLayout(horizontalLayout_3);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 0, -1, 0);
        confirm = new QPushButton(RegisterDialog);
        confirm->setObjectName(QString::fromUtf8("confirm"));

        horizontalLayout_2->addWidget(confirm);

        concel = new QPushButton(RegisterDialog);
        concel->setObjectName(QString::fromUtf8("concel"));

        horizontalLayout_2->addWidget(concel);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(RegisterDialog);

        QMetaObject::connectSlotsByName(RegisterDialog);
    } // setupUi

    void retranslateUi(QDialog *RegisterDialog)
    {
        RegisterDialog->setWindowTitle(QCoreApplication::translate("RegisterDialog", "Dialog", nullptr));
        err_tip->setText(QCoreApplication::translate("RegisterDialog", "\351\224\231\350\257\257\346\217\220\347\244\272", nullptr));
        user_label->setText(QCoreApplication::translate("RegisterDialog", "\347\224\250\346\210\267\357\274\232", nullptr));
        email_label->setText(QCoreApplication::translate("RegisterDialog", "\351\202\256\347\256\261\357\274\232", nullptr));
        pass_label->setText(QCoreApplication::translate("RegisterDialog", "\345\257\206\347\240\201\357\274\232", nullptr));
        confirm_label->setText(QCoreApplication::translate("RegisterDialog", "\347\241\256\350\256\244\357\274\232", nullptr));
        varify_label->setText(QCoreApplication::translate("RegisterDialog", "\351\252\214\350\257\201\347\240\201\357\274\232", nullptr));
        get_code->setText(QCoreApplication::translate("RegisterDialog", "\350\216\267\345\217\226", nullptr));
        confirm->setText(QCoreApplication::translate("RegisterDialog", "\347\241\256\350\256\244", nullptr));
        concel->setText(QCoreApplication::translate("RegisterDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RegisterDialog: public Ui_RegisterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERDIALOG_H
