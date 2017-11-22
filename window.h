#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT
    bool deactivate;

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

private slots:
    int on_connectButton_clicked();

    int on_createButton_clicked();

    int on_activateButton_clicked();

    void on_deactivateButton_clicked();

    void on_createButton_2_clicked();

private:
    Ui::Window *ui;
};

#endif // WINDOW_H
