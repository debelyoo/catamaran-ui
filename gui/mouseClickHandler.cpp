#include "mouseClickHandler.h"
#include <QEvent>
//#include <QKeyEvent>

MouseClickHandler::MouseClickHandler(MainWindow *p)
{
    parent = p;
}

MouseClickHandler::~MouseClickHandler() {}

bool MouseClickHandler::eventFilter(QObject *obj, QEvent *event)
{
    bool res = false;
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(obj->objectName() == "directionSpinBox" || obj->objectName() == "speedSpinBox")
        {
            if ( mouseEvent->button() == Qt::LeftButton)
            {
                // left click on cat control (speed or direction)
                parent->setSliderIsMoving(true);
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if(obj->objectName() == "directionSpinBox" || obj->objectName() == "speedSpinBox")
        {
            if ( mouseEvent->button() == Qt::LeftButton)
            {
                // left click on cat control (speed or direction)
                parent->setSliderIsMoving(false);
                parent->sendEngineCommand();
            }
        }
        else if (obj->objectName() == "mapViewport")
        {
            if (mouseEvent->button() == Qt::RightButton)
            {
                // right click on map
                // draw way point on map
                parent->drawWayPointOnMap(mouseEvent->posF(), true);
            }
        }
        else
        {
            // no action on unspecified object
        }
        break;
    }
    case QEvent::Wheel:
    {
        QWheelEvent* ev = static_cast<QWheelEvent *>(event);
        if (obj->objectName() == "mapViewport")
        {
            //qDebug() << "Wheel event handled";
            if (ev->delta() > 0)
            {
                parent->zoomIn();
            }
            else
            {
                parent->zoomOut();
            }
            event->setAccepted(true);
            res = true; // to stop propagating event
        }
        break;
    }
    default:
        // standard event processing
        break;
    }
    return res;
}
