#include "mainwindow.h"
#include "ui_mainwindow.h"

/*------------------------------------------------------------------------------------------------------------------
   -- SOURCE FILE: mainwindow.cpp
   --
   -- PROGRAM: CommAudio
   --
   -- FUNCTIONS:
   -- void MainWindow::serverPressed()
   -- void MainWindow::clientPressed()
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   --
   -- DESIGNER: Haley Booker, Mackenzie Craig, Tim Bruecker, Alfred Swinton
   --
   -- PROGRAMMER: Haley Booker, Mackenzie Craig, Tim Bruecker, Alfred Swinton
   --
   -- NOTES:
   -- This file handles the creation of the UI for the CommAudio program. It also handles selecting the server or client
   -- component of the program.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: ~MainWindow (destructor)
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER:
   --
   -- PROGRAMMER:
   --
   -- INTERFACE: MainWindow::~MainWindow()
   --
   -- RETURNS: N/A
   --
   -- NOTES:
   -- Destroys the MainWindow object. Deletes all clients, deletes window.
   ----------------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: serverPressed
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER:
   --
   -- PROGRAMMER:
   --
   -- INTERFACE: void MainWindow::serverPressed()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Starts the program in server mode.
   ----------------------------------------------------------------------------------------------------------------------*/
void MainWindow::serverPressed()
{
        serverWindow = new Server();
        serverWindow->show();
}

/*------------------------------------------------------------------------------------------------------------------
   -- FUNCTION: clientPressed
   --
   -- DATE: April 16, 2018
   --
   -- REVISIONS: (Date and Description)
   -- N/A
   --
   -- DESIGNER:
   --
   -- PROGRAMMER:
   --
   -- INTERFACE: void MainWindow::clientPressed()
   --
   -- RETURNS: void.
   --
   -- NOTES:
   -- Starts the program in client mode.
   ----------------------------------------------------------------------------------------------------------------------*/
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
