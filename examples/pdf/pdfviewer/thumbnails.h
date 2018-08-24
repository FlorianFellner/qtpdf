#ifndef THUMBNAILS_H
#define THUMBNAILS_H

#include <QObject>
#include <QtWidgets>
#include <QPdfDocument>

QT_BEGIN_NAMESPACE

class ThumbnailsPrivate;

class Thumbnails : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit Thumbnails(QWidget *parent = nullptr);
    ~Thumbnails();
    void setDocument(QPdfDocument *document);

signals:
    void buttonClicked(int page);

public slots:
    void createButtons();
    void resizeButtons();

protected:
    explicit Thumbnails(ThumbnailsPrivate &, QWidget *);

    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    void clearButtons();
    QPdfDocument *m_document;
    int m_height;
    int m_spacing;
    int m_imageSize;

    Q_DECLARE_PRIVATE(Thumbnails)
};

QT_END_NAMESPACE

#endif // THUMBNAILS_H
