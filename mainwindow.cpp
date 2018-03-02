#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    maxClients = 2;
    currClients = 0;

    for(int i = 0; i < maxClients; i++)
    {
        clientWindow[i] = Q_NULLPTR;
    }

    connect(ui->clientButton, &QPushButton::clicked, this, &MainWindow::clientPressed);
    connect(ui->serverButton, &QPushButton::clicked, this, &MainWindow::serverPressed);
}

MainWindow::~MainWindow()
{
    delete ui;
    for(int i = 0; i < maxClients; i++)
    {
        if(clientWindow[i] != Q_NULLPTR)
        {
            delete clientWindow[i];
        }
    }
    delete serverWindow;
}


void MainWindow::serverPressed()
{
    serverWindow = new Server();
    serverWindow->show();
}

void MainWindow::clientPressed()
{
    if(currClients <= maxClients)
    {
        clientWindow[currClients] = new Client();
        clientWindow[currClients]->show();
        currClients++;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Maximum number of clients reached.");
        msgBox.setWindowTitle("Information");
        msgBox.exec();
    }
}
