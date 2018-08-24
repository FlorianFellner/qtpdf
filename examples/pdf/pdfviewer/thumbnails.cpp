#include "thumbnails.h"
#include "thumbnails_p.h"
#include <QString>

QT_BEGIN_NAMESPACE

/*!
 * Calculates the viewport from the value of the vertical QScrollBar. The horizontal bar will not be used in this widget.
 */
void ThumbnailsPrivate::calculateViewport()
{
    Q_Q(Thumbnails);

    const int y = q->verticalScrollBar()->value();
    const int width = q->viewport()->width();
    const int height = q->viewport()->height();

    setViewport(QRect(0, y, width, height));
}

/*!
 * Sets the size of the scroll area's viewport to \a viewport.
 */
void ThumbnailsPrivate::setViewport(QRect viewport)
{
    if (m_viewport == viewport)
        return;

    m_viewport = viewport;
}

/*!
 * Calculates range and page step for the vertical QScrollBar from the vieport's height and m_height.
 * The horizontal gets ignored since it is irrelevant for this widget.
 */
void ThumbnailsPrivate::updateScrollBar()
{
    Q_Q(Thumbnails);

    const int viewHeight = q->viewport()->size().height();

    q->verticalScrollBar()->setRange(0, q->m_height - viewHeight);
    q->verticalScrollBar()->setPageStep(viewHeight);
}

/*!
 * Constructs a new thumbnail widget with parent object \a parent.
 * Sets the default value of m_spacing to 10 and the scroll bar's single step to 20.
 */
Thumbnails::Thumbnails(QWidget *parent)
    : QAbstractScrollArea(*new ThumbnailsPrivate(), parent)
    , m_spacing(10)
{
    verticalScrollBar()->setSingleStep(20);
    QScroller::grabGesture(this);

    Q_D(Thumbnails);
    d->calculateViewport();
}

/*!
 * Constructs a new thumbnail widget with parent object \a parent and ThumbnailsPrivate object \a dd.
 */
Thumbnails::Thumbnails(ThumbnailsPrivate &dd, QWidget *parent)
    : QAbstractScrollArea(dd, parent)
{
}

/*!
 * Destroys the object of the thumbnail widget.
 */
Thumbnails::~Thumbnails()
{
}

/*!
 * Calls resizeButtons() when the widget is resized. If the page count of m_document is 0, the QResizeEvent is ignored.
 */
void Thumbnails::resizeEvent(QResizeEvent *event)
{
    if (m_document->pageCount() == 0)
        return;
    QAbstractScrollArea::resizeEvent(event);

    resizeButtons();
}

/*!
 * Moves all buttons that are children of this widget by \a dy along the y-axis.
 * The buttons will not be modified by \a dx because this is always expected to be 0,
 * since it should not be possible to scroll along the x-axis.
 */
void Thumbnails::scrollContentsBy(int dx, int dy)
{
    Q_D(Thumbnails);

    QList<QPushButton*> list = findChildren<QPushButton*>();
    for (int i = 0; i < list.length(); i++) {
        QPoint pos = list.value(i)->pos();
        list.value(i)->move(pos.x(), pos.y() + dy);
    }
    QAbstractScrollArea::scrollContentsBy(dx, dy);

    d->calculateViewport();
}

/*!
 * Recalculates the size and position of the buttons.
 *
 * width: will always be 2/3 of the total width
 * height: is calculated from the width and the aspect ratio which is taken from the page size of m_document
 *
 * posX: the buttons will always be centered
 * posY: the buttons are placed below each other with a distance of m_spacing
 *
 * m_heigth is set to the total length of all buttons and the spacing before, after and between the buttons
 *
 * If the new width of the buttons is greater than m_imageSize, all thumbnails are rendered again and m_imageSize is set
 * to the new value. The images are not rendered if the icons already got rendered in a larger size because they just
 * get scaled down to the required size. This does not work the other way around, due to the way Qt handles icons.
 */
void Thumbnails::resizeButtons()
{
    float ratio = m_document->pageSize(0).width() / m_document->pageSize(0).height();
    int width = this->size().width() / 1.5;
    int height = width / ratio;
    QSize size(width, height);
    QList<QPushButton*> list = findChildren<QPushButton*>();
    int posX = (this->size().width() - width) / 2;
    if (verticalScrollBar()->isVisible()) {
        posX -= verticalScrollBar()->width() / 2;
    }
    int posY = m_spacing - verticalScrollBar()->value();
    for (int i = 0; i < list.length(); i++) {
        QPushButton* button = list.value(i);
        button->setGeometry(posX, posY, width, height);
        posY += height + m_spacing;
        if (m_imageSize < width) {
            QImage image = m_document->render(i, size);
            button->setIcon(QIcon(QPixmap::fromImage(image)));
        }
        button->setIconSize(size);
    }
    if (m_imageSize < width) {
        m_imageSize = width;
    }
    m_height = m_spacing + (m_document->pageCount() * (height + m_spacing));

    Q_D(Thumbnails);
    d->updateScrollBar();
    d->calculateViewport();
}

/*!
 * Clears all of the old buttons and creates new ones if there is at least one page.
 *
 * m_imageSize is set to 0 which means that all thumbnails are rendered again, because the required width
 * will always be greater than 0.
 *
 * Calls resizeButtons(), so the new buttons are placed at the right position, are sized correctly and
 * the new icons get rendered.
 */
void Thumbnails::createButtons()
{
    clearButtons();
    m_imageSize = 0;
    if (m_document->pageCount() == 0) {
        verticalScrollBar()->setRange(0, 0);
        return;
    }
    for (int i = 0; i < m_document->pageCount(); i++) {
        QPushButton *button = new QPushButton(this);
        button->setStyleSheet("border:1px solid black;");
        button->show();
        connect(button, &QPushButton::clicked, [this, i](){emit buttonClicked(i);});
    }
    resizeButtons();
}

/*!
 * Loops through all the buttons that are children of this widget and deletes them.
 */
void Thumbnails::clearButtons()
{
    QList<QPushButton*> list = findChildren<QPushButton*>();
    for (int i = 0; i < list.length(); i++) {
        delete list.value(i);
    }
}

/*!
 * Sets m_document to \a document and connects its signal pageCountChanged(int) to the createButtons() slot.
 * This will also work when a new document that has the same pageCount as the last one is loaded, because QPdfDocument.load()
 * always sets the count to 0 before it starts loading the new file. This means that pageCountChanged(int) is called twice
 * every time a new file is opened.
 */
void Thumbnails::setDocument(QPdfDocument *document)
{
    m_document = document;
    connect(m_document, SIGNAL(pageCountChanged(int)), this, SLOT(createButtons()));
}

QT_END_NAMESPACE

#include "moc_thumbnails.cpp"
