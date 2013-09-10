#ifndef MOUSECLICKHANDLER_H
#define MOUSECLICKHANDLER_H

#include "mainwindow.h"
#include <QObject>

class MainWindow;

class MouseClickHandler : public QObject
{
    Q_OBJECT
    //typedef void (MainWindow::*CallbackFct)(void);

    public:
        explicit MouseClickHandler(MainWindow *p);
        ~MouseClickHandler();

    protected:
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        MainWindow *parent;
};


#endif // MOUSECLICKHANDLER_H
