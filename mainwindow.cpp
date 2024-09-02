#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form.h"
#include "ui_form.h"
#include <QDesktopWidget>
#include <QFileDialog>
#include <QDebug>
#include <QTextCodec>
#include <QMessageBox>
#include <QMimeData>

#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move((QApplication::desktop()->width()-width())/2, (QApplication::desktop()->height()-height())/2);
    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    comboBox = new QComboBox;
    comboBox->setFixedWidth(100);
    comboBox->setFocusPolicy(Qt::NoFocus);
    connect(comboBox,SIGNAL(currentTextChanged(QString)),this,SLOT(changeCodec(QString)));
    ui->toolBar->addWidget(comboBox);

    lineEdit_filter = new QLineEdit;
    lineEdit_filter->setFixedWidth(70);
    lineEdit_filter->setPlaceholderText("filter");
    connect(lineEdit_filter,SIGNAL(textChanged(QString)),this,SLOT(filter(QString)));
    ui->toolBar->addWidget(lineEdit_filter);

    listCodecs = QTextCodec::availableCodecs();
    filter("");
    TC = QTextCodec::codecForLocale();
    comboBox->setCurrentIndex(comboBox->findText(TC->name()));
    ui->statusBar->showMessage("together "+ QString::number(listCodecs.size()) + "Kind of coding, system coding" + TC->name());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if (path == "") {
        path = QFileDialog::getOpenFileName(this, "Open", ".", "*.mp3");
    } else {
        path = QFileDialog::getOpenFileName(this, "Open", path, "*.mp3");
    }
    if (!path.isEmpty()) {
        open(path);
    }
}

void MainWindow::open(QString filename)
{
    //qDebug() << "open" << filename;
    ui->label->setPixmap(QPixmap());
    path = filename;
    setWindowTitle("MP3Tag - " + QFileInfo(filename).fileName());
    ui->textBrowser->setText("");

    QLayoutItem *layoutItem;
    while ((layoutItem = ui->verticalLayout2->takeAt(0)) != 0 ) {
        delete layoutItem->widget();
        delete layoutItem;
    }
    while ((layoutItem = ui->verticalLayout1->takeAt(0)) != 0 ) {
        delete layoutItem->widget();
        delete layoutItem;
    }

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QString ID3,Ver,Revision,Flag;
    bool ok;
    qint64 pos,size;
    ID3 = QString(file.read(3));
    if (ID3 == "ID3") {
        ui->textBrowser->append("ID3V2");
        Ver = QString::number(file.read(1).toHex().toInt(&ok,16));
        ui->textBrowser->append("Version" + Ver);
        Revision = QString::number(file.read(1).toHex().toInt(&ok,16));
        ui->textBrowser->append("Revision" + Revision);
        Flag = QString::number(file.read(1).toHex().toInt(&ok,16));
        ui->textBrowser->append("Flag" + Flag);
        //size = file.read(4).toHex().toLongLong(&ok,16);
//        QByteArray a = file.read(1);
//        QByteArray b = file.read(1);
//        QByteArray c = file.read(1);
//        QByteArray d = file.read(1);
//        qDebug() << a << b << c << d;
        size = (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 21 | (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 14 | (file.read(1).toHex().toInt(&ok,16) & 0xEF) << 7 | file.read(1).toHex().toInt(&ok,16) & 0xEF;        
        qDebug() << "ID3V2 Size:" << size;
        ui->textBrowser->append("Size" + QString::number(size));
        while (file.pos() < size) {
            QString FTag(file.read(4));
            qint64 FSize = file.read(4).toHex().toLongLong(&ok,16);
            //qint64 FSize = file.read(1).toHex().toInt(&ok,16) << 24 | file.read(1).toHex().toInt(&ok,16) << 16 | file.read(1).toHex().toInt(&ok,16) << 8 | file.read(1).toHex().toInt(&ok,16);
            Flag = QString::number(file.read(2).toHex().toInt(&ok,16));            
            QByteArray BA = file.read(FSize);
            if (FTag == "APIC") {
                BA = BA.right(FSize-14);
                QPixmap pixmap;
                ok = pixmap.loadFromData(BA);
                //qDebug() << "QPixmap.loadFromData(QByteArray)" << ok;
                ui->label->setPixmap(pixmap.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                //break;
            } else if (FTag == "TYER") {
                qDebug() << FTag << FSize << BA.toHex().toUpper();
                if(BA.contains("\xFF\xFE")){
                    qDebug() << FTag << "Unicode" << BA.mid(3,FSize-3).toHex().toUpper();
                    QString content =  QString::fromUtf16(reinterpret_cast<const ushort*>(BA.mid(3,FSize-3).data()));
                    ui->textBrowser->append(FTag + ": " + content);
                    Form *form = new Form;
                    form->listCodecs = listCodecs;
                    form->filter("");
                    form->BA = BA;
                    form->ui->label_tag->setText(FTag + ":");
                    form->ui->lineEdit_content->setText(content);
                    form->ui->lineEdit_content->setCursorPosition(0);
                    form->ui->comboBox->setEnabled(false);
                    form->ui->lineEdit_filter->setEnabled(false);
                    ui->verticalLayout2->addWidget(form);
                }else{
                    qDebug() << FTag << FSize << BA.mid(1,FSize-2);
                    //ui->textBrowser->append(FTag + ": " + BA.mid(1,FSize-2));
                    ui->textBrowser->append(FTag + ": " + TC->toUnicode(BA));
                }
            } else if (FTag == "COMM") {
                QString language = BA.mid(1,3);
                qDebug() << FTag << FSize << language << BA.mid(10,FSize-12).toHex().toUpper();
                QString content = FTag + ": " + language + " " + QString::fromUtf16(reinterpret_cast<const ushort*>(BA.mid(10,FSize-12).data()));
                ui->textBrowser->append(content);
                //ui->textBrowser->append(FTag + ": " + TC->toUnicode(BA));
                Form *form = new Form;
                form->listCodecs = listCodecs;
                form->filter("");
                form->BA = BA;
                form->ui->label_tag->setText(FTag + ":");
                form->ui->lineEdit_content->setText(content);
                form->ui->lineEdit_content->setCursorPosition(0);
                form->ui->comboBox->setEnabled(false);
                form->ui->lineEdit_filter->setEnabled(false);
                ui->verticalLayout2->addWidget(form);
            } else {
                if(FTag != ""){
                    QByteArray UFlag = BA.left(1);
                    qDebug() << "UFlag" << UFlag;
                    //qDebug() << "UFlag" << UFlag.toHex().toUpper();
                    if(UFlag.toHex().toInt() == 0){
                        qDebug() << FTag << BA.right(FSize-1).toHex().toUpper();
                        ui->textBrowser->append(FTag + ": " + TC->toUnicode(BA));
                        Form *form = new Form;
                        form->listCodecs = listCodecs;
                        form->filter("");
                        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
                        form->BA = BA;
                        form->ui->label_tag->setText(FTag + ":");
                        form->ui->lineEdit_content->setText(TC->toUnicode(BA));
                        form->ui->lineEdit_content->setCursorPosition(0);
                        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
                        ui->verticalLayout2->addWidget(form);
                    }else{
                        qDebug() << FTag << BA.right(FSize-3).toHex().toUpper();
                        QString content = QString::fromUtf16(reinterpret_cast<const ushort*>(BA.right(FSize-3).data()));
                        ui->textBrowser->append(FTag + ": " + content);
                        Form *form = new Form;
                        form->listCodecs = listCodecs;
                        form->filter("");
                        form->BA = BA;
                        form->ui->label_tag->setText(FTag + ":");
                        form->ui->lineEdit_content->setText(content);
                        form->ui->lineEdit_content->setCursorPosition(0);
                        form->ui->comboBox->setEnabled(false);
                        form->ui->lineEdit_filter->setEnabled(false);
                        ui->verticalLayout2->addWidget(form);
                    }
                }
            }
        }
    } else {
        ui->textBrowser->append("ID3V2");
    }
    ui->textBrowser->append("----------------------------------------");
    pos = file.size()-128;
    file.seek(pos);
    QString TAG,Title,Artist,Album,Year,Comment,Reserved,Track,Genre;
    TAG = QString(file.read(3));
    if (TAG == "TAG") {
        qDebug() << "----------------------------------------ID3V1--------------------------------------------";

        QByteArray BA;
        ui->textBrowser->append("ID3V1");
        BA = file.read(30);
        qDebug() << "Title" << BA.toHex().toUpper();
        Title = TC->toUnicode(BA);
        ui->textBrowser->append("标题：" + Title);
        Form *form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("标题：");
        form->ui->lineEdit_content->setText(Title);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(30);
        qDebug() << "Artist" << BA.toHex().toUpper();
        Artist = TC->toUnicode(BA);
        ui->textBrowser->append("Artist" + Artist);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Artist");
        form->ui->lineEdit_content->setText(Artist);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(30);
        qDebug() << "Album" << BA.toHex().toUpper();
        Album = TC->toUnicode(BA);
        ui->textBrowser->append("Album" + Album);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Album");
        form->ui->lineEdit_content->setText(Album);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(4);
        qDebug() << "Year" << BA.toHex().toUpper();
        Year = QString(BA);
        ui->textBrowser->append("Year" + Year);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Year");
        form->ui->lineEdit_content->setText(Year);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(28);
        qDebug() << "Comment" << BA.toHex().toUpper();
        Comment = TC->toUnicode(BA);
        ui->textBrowser->append("Comment" + Comment);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Comment");
        form->ui->lineEdit_content->setText(Comment);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(1);
        qDebug() << "Reserved" << BA.toHex().toUpper();
        Reserved = QString(BA);
        ui->textBrowser->append("Reserved" + Reserved);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Reserved");
        form->ui->lineEdit_content->setText(Reserved);
        form->ui->lineEdit_content->setCursorPosition(0);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(1);
        qDebug() << "Track" << BA.toHex().toUpper();
        Track = QString(BA);
        ui->textBrowser->append("Track" + Track);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Track");
        form->ui->lineEdit_content->setText(Track);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        BA = file.read(1);
        qDebug() << "Genre" << BA.toHex().toUpper();
        Genre = QString::number(BA.toInt());
        ui->textBrowser->append("Genre" + Genre);
        form = new Form;
        form->listCodecs = listCodecs;
        form->filter("");
        form->ui->lineEdit_filter->setText(lineEdit_filter->text());
        form->BA = BA;
        form->ui->label_tag->setText("Genre");
        form->ui->lineEdit_content->setText(Genre);
        form->ui->comboBox->setCurrentIndex(form->ui->comboBox->findText(comboBox->currentText()));
        ui->verticalLayout1->addWidget(form);

        ui->verticalLayout1->addStretch();
    } else {
        ui->textBrowser->append("ID3V1");
    }
    file.close();
    ui->textBrowser->moveCursor(QTextCursor::Start);
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "mp3tageditor", "MP3 ID3 1.0\n\n Qt  MP3 ID3 \n\nE-mail: dimadimof81@gmail.com\n https://github.com/dmitryreaper");
    aboutMB.setIconPixmap(QPixmap(":/icon.svg"));
    aboutMB.setWindowIcon(QIcon(":/icon.svg"));
    aboutMB.exec();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //qDebug() << "dragEnter:" << e->mimeData()->formats();
    if(e->mimeData()->hasFormat("text/uri-list"))
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;

    QString fileName = urls.first().toLocalFile();

    foreach (QUrl u, urls) {
        qDebug() << u.toString();
    }
    qDebug() << urls.size();

    if(fileName.isEmpty())
        return;

    qDebug() << "drop:" << fileName;
    open(fileName);
}

void MainWindow::changeCodec(QString codec)
{
    if(path!=""){
        TC = QTextCodec::codecForName(codec.toLatin1());
        if(TC){
            ui->statusBar->showMessage("to " + TC->name() + " decrypt ");
            open(path);
        }else{
            ui->statusBar->showMessage(codec + " Code not found ");
        }
    }
}

void MainWindow::filter(QString s)
{
    comboBox->clear();
    for(int i=0; i<listCodecs.size(); i++){
        if(listCodecs.at(i).contains(s.toLatin1()))
            comboBox->addItem(listCodecs.at(i));
    }
    ui->statusBar->showMessage("Filter out" + QString::number(comboBox->count()) + "Kind of code");
}
