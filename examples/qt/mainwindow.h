#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void showEvent(QShowEvent *event);

signals:
    void windowWasShown();

public slots:
    void initWinSparkle();
    void checkForUpdates();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
