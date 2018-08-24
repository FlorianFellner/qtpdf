/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pageselector.h"
#include "zoomselector.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPdfBookmarkModel>
#include <QPdfDocument>
#include <QPdfPageNavigation>
#include <QtMath>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

const qreal zoomMultiplier = qSqrt(2.0);

Q_LOGGING_CATEGORY(lcExample, "qt.examples.pdfviewer")

/*!
 * Constructs a new MainWindow with parent object \a parent.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_zoomSelector(new ZoomSelector(this))
    , m_pageSelector(new PageSelector(this))
    , m_document(new QPdfDocument(this))
{
    ui->setupUi(this);
    m_zoomSelector->setMaximumWidth(150);
    ui->mainToolBar->insertWidget(ui->actionZoom_In, m_zoomSelector);
    m_pageSelector->setMaximumWidth(150);
    ui->mainToolBar->addWidget(m_pageSelector);

    m_pageSelector->setPageNavigation(ui->pdfView->pageNavigation());

    connect(m_zoomSelector, &ZoomSelector::zoomModeChanged, ui->pdfView, &QPdfView::setZoomMode);
    connect(m_zoomSelector, &ZoomSelector::zoomFactorChanged, ui->pdfView, &QPdfView::setZoomFactor);
    m_zoomSelector->reset();

    QPdfBookmarkModel *bookmarkModel = new QPdfBookmarkModel(this);
    bookmarkModel->setDocument(m_document);

    ui->bookmarkView->setModel(bookmarkModel);
    connect(ui->bookmarkView, SIGNAL(activated(QModelIndex)), this, SLOT(bookmarkSelected(QModelIndex)));

    ui->tabWidget->setCurrentIndex(1);
    ui->pagesView->setDocument(m_document);
    connect(ui->pagesView, SIGNAL(buttonClicked(int)), this, SLOT(thumbnailClicked(int)));

    ui->pdfView->setDocument(m_document);

    connect(ui->pdfView, &QPdfView::zoomFactorChanged,
            m_zoomSelector, &ZoomSelector::setZoomFactor);
}

/*!
 * Destroys the MainWindow and deletes its ui.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * Opens the pdf file at \a docLocation.
 *
 * Creates a QMessageBox with an error message if \a docLocation is not a valid local file.
 * Otherwise m_document loads the file and the window title is set to the name of the document.
 */
void MainWindow::open(const QUrl &docLocation)
{
    if (docLocation.isLocalFile()) {
        QPdfDocument::DocumentError error = m_document->load(docLocation.toLocalFile());
        while (error == QPdfDocument::IncorrectPasswordError) {
            bool ok;
            QString password = QInputDialog::getText(this, tr("Password"), tr("Password:"), QLineEdit::Normal, QString(), &ok);
            if (ok) {
                m_document->setPassword(password);
                error = m_document->load(docLocation.toLocalFile());
            } else {
                return;
            }
        }
        const auto documentTitle = m_document->metaData(QPdfDocument::Title).toString();
        setWindowTitle(!documentTitle.isEmpty() ? documentTitle : QStringLiteral("PDF Viewer"));
    } else {
        qCDebug(lcExample) << docLocation << "is not a valid local file";
        QMessageBox::critical(this, tr("Failed to open"), tr("%1 is not a valid local file").arg(docLocation.toString()));
    }
    qCDebug(lcExample) << docLocation;
}

/*!
 * Navigates to \a page when the associated thumbnail button is clicked.
 */
void MainWindow::thumbnailClicked(int page)
{
    ui->pdfView->pageNavigation()->setCurrentPage(page);
}

/*!
 * Navigates to the associated page if a bookmark is selected.
 */
void MainWindow::bookmarkSelected(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const int page = index.data(QPdfBookmarkModel::PageNumberRole).toInt();
    ui->pdfView->pageNavigation()->setCurrentPage(page);
}

/*!
 * Rotates the pages by -90 degrees
 */
void MainWindow::on_actionRotateLeft_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    if (m_rotation == 0)
        m_rotation = 360;
    m_rotation -= 90;
    setRotation(m_rotation);
}

/*!
 * Rotates the pages by 90 degrees
 */
void MainWindow::on_actionRotateRight_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    m_rotation += 90;
    m_rotation %= 360;
    setRotation(m_rotation);
}

/*!
 * Sets the rotation of pdfView's render options to \a rotation.
 * In case \a rotation does not equal 90, 180 or 270 the rotation will always be set to 0.
 */
void MainWindow::setRotation(int rotation)
{
    switch (rotation) {
        case 90: ui->pdfView->setRotation(QPdf::Rotate90); break;
        case 180: ui->pdfView->setRotation(QPdf::Rotate180); break;
        case 270: ui->pdfView->setRotation(QPdf::Rotate270); break;
        default: ui->pdfView->setRotation(QPdf::Rotate0);
    }
}

/*!
 * Creates a QPrintDialog and prints the document using QPrinter
 */
void MainWindow::on_actionPrint_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFileName("print.ps");
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    dlg->setWindowTitle(tr("Print Document"));
    if(dlg->exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        for (int page = 0; page < m_document->pageCount(); page++) {
            QImage img = m_document->render(page, printer.pageRect().size());
            painter.drawImage(QPoint(0, 0), img);
            if (page != m_document->pageCount()-1) {
                printer.newPage();
            }
        }
        painter.end();
    }
    delete dlg;
}

/*!
 * Shows a QFileDialog to choose the pdf file that should be opened.
 * If the path is valid it is passed to open() and the current page is set to the first one.
 */
void MainWindow::on_actionOpen_triggered()
{
    QUrl toOpen = QFileDialog::getOpenFileUrl(this, tr("Choose a PDF"), QUrl(), "Portable Documents (*.pdf)");
    if (toOpen.isValid()) {
        open(toOpen);
        m_pageSelector->onCurrentPageChanged(0);
        m_zoomSelector->setEnabled(true);
    }
}

/*!
 * Opens a new QFileDialog that chooses the file path where to document should be saved.
 * Does not open the dialog and does not save the file if there is no pdf file loaded in pdfView->document().
 */
void MainWindow::on_actionSave_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    QUrl toSave = QFileDialog::getSaveFileUrl(this, tr("Save copy of PDF"), QUrl(), "Portable Documents (*.pdf)");
    m_document->save(toSave.toLocalFile());
}

/*!
 * Quits the QApplication
 */
void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

/*!
 * Closes the current file
 */
void MainWindow::on_actionClose_triggered()
{
    m_document->close();
}


/*!
 * Displays a QMessageBox with a short text about this project
 */
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About PdfViewer"),
        tr("An example using QPdfDocument"));
}

/*!
 * Displays QMessageBox::aboutQt()
 */
void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

/*!
 * Multiplies the zoom factor by zoomMultipler.
 * Ignored if no pdf file loaded.
 */
void MainWindow::on_actionZoom_In_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() * zoomMultiplier);
}

/*!
 * Divides the zoom factor by zoomMultipler.
 * Ignored if no pdf file loaded.
 */
void MainWindow::on_actionZoom_Out_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    ui->pdfView->setZoomFactor(ui->pdfView->zoomFactor() / zoomMultiplier);
}

/*!
 * Navigates to the previous page if possible.
 * Ignored if no pdf file loaded.
 */
void MainWindow::on_actionPrevious_Page_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    ui->pdfView->pageNavigation()->goToPreviousPage();
}

/*!
 * Navigates to the following page if possible.
 * Ignored if no pdf file loaded.
 */
void MainWindow::on_actionNext_Page_triggered()
{
    if (ui->pdfView->document()->status() != QPdfDocument::Ready)
        return;
    ui->pdfView->pageNavigation()->goToNextPage();
}

/*!
 * Switches between multi and single page mode
 *
 * checked:   QPdfView::MultiPage
 * unchecked: QPdfView::SinglePage
 */
void MainWindow::on_actionContinuous_triggered()
{
    ui->pdfView->setPageMode(ui->actionContinuous->isChecked() ? QPdfView::MultiPage : QPdfView::SinglePage);
}

/*!
 * Sets \a flag if \a on is true or unsets it if \a on is false.
 * Calls QPdfView.setRenderFlags(QPdf::RenderFlags) to change the document's QPdfDocumentRenderOptions
 * and force the document to render the current page again with the new settings.
 * m_flags is set to QPdf::NoRenderFlags by default.
 */
void MainWindow::setRenderFlag(QPdf::RenderFlag flag, bool on)
{
    m_flags.setFlag(flag, on);
    ui->pdfView->setRenderFlags(m_flags);
}

/*!
 * Adds/removes QPdf::RenderAnnotations to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionAnnotations_triggered()
{
    setRenderFlag(QPdf::RenderAnnotations, ui->actionAnnotations->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderOptimizedForLcd to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionOptimiziedForLcd_triggered()
{
    setRenderFlag(QPdf::RenderOptimizedForLcd, ui->actionAnnotations->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderGrayscale to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionGrayscale_triggered()
{
    setRenderFlag(QPdf::RenderGrayscale, ui->actionGrayscale->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderForceHalftone to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionForceHalftone_triggered()
{
    setRenderFlag(QPdf::RenderForceHalftone, ui->actionForceHalftone->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderTextAliased to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionTextAliased_triggered()
{
    setRenderFlag(QPdf::RenderTextAliased, ui->actionTextAliased->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderImageAliased to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionImageAliased_triggered()
{
    setRenderFlag(QPdf::RenderImageAliased, ui->actionImageAliased->isChecked() ? true : false);
}

/*!
 * Adds/removes QPdf::RenderPathAliased to/from m_flags if the associated button is checked/unchecked.
 */
void MainWindow::on_actionPathAliased_triggered()
{
    setRenderFlag(QPdf::RenderPathAliased, ui->actionPathAliased->isChecked() ? true : false);
}
