#include "mouseClickHandler.h"
#include <QEvent>
#include <QKeyEvent>

MouseClickHandler::MouseClickHandler(MainWindow *p)
{
    parent = p;
}

MouseClickHandler::~MouseClickHandler() {}

bool MouseClickHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        //qDebug("mouse released");
        parent->sendEngineCommand();
        return false;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
