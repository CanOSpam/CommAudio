#pragma once

#include <QMainWindow>
#include <QPushButton>

#include "server.h"
#include "client.h"
#include "ui_mainwindow.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Server *serverWindow;
    Client *clientWindow;

    void clientPressed();
    void serverPressed();
};
