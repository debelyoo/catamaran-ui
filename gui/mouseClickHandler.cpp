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
    switch (event->type()) {
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(obj->objectName() == "directionSpinBox" || obj->objectName() == "speedSpinBox")
        {
            if ( mouseEvent->button() == Qt::LeftButton)
            {
                // left click on cat control (speed or direction)
                parent->sendEngineCommand();
            }
        }
        else if (obj->objectName() == "mapViewport")
        {
            if (mouseEvent->button() == Qt::RightButton)
            {
                // right click on map
                // draw way point on map
                parent->drawWayPointOnMap(mouseEvent->pos());
            }
        }
        else
        {
            // no action on unspecified object
        }
        return false;
        break;
    }
    default:
        // standard event processing
        return QObject::eventFilter(obj, event);
        break;
    }
}
