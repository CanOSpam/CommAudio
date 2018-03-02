#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>

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
    Client *clientWindow[32];
    int maxClients;
    int currClients;

    void clientPressed();
    void serverPressed();
};
