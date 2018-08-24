#ifndef THUMBNAILS_P_H
#define THUMBNAILS_P_H

#include "thumbnails.h"

#include <QPointer>
#include <QtWidgets/private/qabstractscrollarea_p.h>

QT_BEGIN_NAMESPACE

class ThumbnailsPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(Thumbnails)

public:
    void calculateViewport();
    void setViewport(QRect viewport);
    void updateScrollBar();

    QRect m_viewport;
};

QT_END_NAMESPACE

#endif // THUMBNAILS_P_H
