#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDropEvent>
#include <QComboBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString path;
    void open(QString filename);
    QTextCodec *TC;
    QComboBox *comboBox;
    QList<QByteArray> listCodecs;
    QLineEdit *lineEdit_filter;

private slots:
    void on_action_open_triggered();
    void on_action_about_triggered();
    void changeCodec(QString codec);
    void filter(QString s);

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
};

#endif // MAINWINDOW_H
