#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTextCodec>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    Ui::Form *ui;
     QList<QByteArray> listCodecs;
    QByteArray BA;

private:
    QTextCodec *TC;

public slots:
    void filter(QString s);

private slots:
    void changeCodec(QString codec);

};

#endif // FORM_H
