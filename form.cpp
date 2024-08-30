#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    connect(ui->comboBox,SIGNAL(currentTextChanged(QString)),this,SLOT(changeCodec(QString)));
    connect(ui->lineEdit_filter,SIGNAL(textChanged(QString)),this,SLOT(filter(QString)));
}

Form::~Form()
{
    delete ui;
}

void Form::changeCodec(QString codec)
{
    TC = QTextCodec::codecForName(codec.toLatin1());
    if(TC){
        ui->lineEdit_content->setText(TC->toUnicode(BA));
        ui->lineEdit_content->setCursorPosition(0);
    }
}


void Form::filter(QString s)
{
    ui->comboBox->clear();
    for(int i=0; i<listCodecs.size(); i++){
        if(listCodecs.at(i).contains(s.toLatin1()))
            ui->comboBox->addItem(listCodecs.at(i));
    }
}
