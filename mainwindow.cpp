#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->clientButton, &QPushButton::clicked, this, &MainWindow::clientPressed);
    connect(ui->serverButton, &QPushButton::clicked, this, &MainWindow::serverPressed);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete clientWindow;
    delete serverWindow;
}


void MainWindow::serverPressed()
{
    serverWindow = new Server();
    serverWindow->show();
}

void MainWindow::clientPressed()
{
    clientWindow = new Client();
    clientWindow->show();
}
