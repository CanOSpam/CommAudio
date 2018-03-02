#pragma once
#include "ui_client.h"

#include <QWidget>

namespace Ui
{
    class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = Q_NULLPTR);
    ~Client();


private:
    Ui::Client *ui;
};
