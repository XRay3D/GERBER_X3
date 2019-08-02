//#ifndef OPENINGDIALOG_H
//#define OPENINGDIALOG_H

//#include <QtCore/QVariant>
//#include <QtWidgets/QAction>
//#include <QtWidgets/QApplication>
//#include <QtWidgets/QButtonGroup>
//#include <QtWidgets/QDialog>
//#include <QtWidgets/QDialogButtonBox>
//#include <QtWidgets/QHeaderView>
//#include <QtWidgets/QLabel>
//#include <QtWidgets/QProgressBar>
//#include <QtWidgets/QVBoxLayout>

//QT_BEGIN_NAMESPACE

//class OpeningDialog : public QDialog {
//    Q_OBJECT

//public:
//    explicit OpeningDialog(QWidget* parent = Q_NULLPTR)
//        : QDialog(parent)
//    {
//        setupUi(this);
//        connect(buttonBox, &QDialogButtonBox::rejected, [&] { cancel = true; hide(); });
//    }
//    ~OpeningDialog()
//    {
//    }
//    void setFileName(const QString& fileName)
//    {
//        label->setText("Reading: " + fileName);
//        progressBar->setValue(0);
//        show();
//    }
//    void setMaximum(int maximum)
//    {
//        progressBar->setMaximum(maximum);
//    }
//    void setValue(int value)
//    {
//        progressBar->setValue(value);
//    }

//    bool cancel = false;

//private:
//    QVBoxLayout* verticalLayout;
//    QLabel* label;
//    QProgressBar* progressBar;
//    QDialogButtonBox* buttonBox;
//    void setupUi(QDialog* Dialog)
//    {
//        if (Dialog->objectName().isEmpty())
//            Dialog->setObjectName(QStringLiteral("Dialog"));
//        Dialog->resize(408, 109);
//        Dialog->setSizeGripEnabled(false);
//        Dialog->setModal(true);
//        verticalLayout = new QVBoxLayout(Dialog);
//        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
//        label = new QLabel(Dialog);
//        label->setObjectName(QStringLiteral("label"));

//        verticalLayout->addWidget(label);

//        progressBar = new QProgressBar(Dialog);
//        progressBar->setObjectName(QStringLiteral("progressBar"));
//        progressBar->setValue(0);
//        progressBar->setAlignment(Qt::AlignCenter);
//        progressBar->setTextDirection(QProgressBar::TopToBottom);

//        verticalLayout->addWidget(progressBar);

//        buttonBox = new QDialogButtonBox(Dialog);
//        buttonBox->setObjectName(QStringLiteral("buttonBox"));
//        buttonBox->setOrientation(Qt::Horizontal);
//        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

//        verticalLayout->addWidget(buttonBox);

//        retranslateUi(Dialog);
//        //        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
//        //        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

//        QMetaObject::connectSlotsByName(Dialog);
//    } // setupUi

//    void retranslateUi(QDialog* Dialog)
//    {
//        Dialog->setWindowTitle(QApplication::translate("Dialog", "Opening File", Q_NULLPTR));
//        label->setText(QApplication::translate("Dialog", "TextLabel", Q_NULLPTR));
//    } // retranslateUi
//};

//#endif // OPENINGDIALOG_H
