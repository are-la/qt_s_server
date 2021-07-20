#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "header.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    QVector<QTcpSocket*> all_socket;

private:
    Ui::Widget *ui;
    QTcpServer *server;
    QTcpSocket *serverSocket;

private slots:
    void acceptConnection();

    void replyToClient();
};

#endif // WIDGET_H
