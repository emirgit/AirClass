#include "presentationmanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QPdfPageNavigator>
#include <QElapsedTimer>
#include <QScrollBar>
#include <QPointF>
#include <QApplication>
#include <QTimer>
#include <QThread>

PresentationManager::PresentationManager(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_pdfView(nullptr)
    , m_currentPage(0)
    , m_totalPages(0)
    , m_zoomLevel(1.0)
{
    // Create document after initialization
    m_document = new QPdfDocument(this);
    connect(m_document, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status status) {
        qDebug() << "PDF Document status changed:" << status;
        if (status == QPdfDocument::Status::Error) {
            emit error("Error loading PDF document");
        }
    });
}

PresentationManager::~PresentationManager()
{
    if (m_pdfView) {
        m_pdfView->setDocument(nullptr);
    }
    if (m_document) {
        m_document->close();
    }
}

bool PresentationManager::loadPresentation(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        emit error(QString("Cannot read file: %1").arg(filePath));
        return false;
    }

    try {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        // qDebug() << "Starting to load PDF file:" << filePath;
        // qDebug() << "File size:" << fileInfo.size() << "bytes";

        // Reset state before loading new document
        if (m_pdfView) {
            // qDebug() << "1 - Clearing old PDF view";
            m_pdfView->setDocument(nullptr);
        }

        // Close any previously open document
        if (m_document) {
            // qDebug() << "2 - Closing old document";
            m_document->close();
        }

        // Load the new document
        // qDebug() << "Loading document...";
        auto result = m_document->load(filePath);
        // qDebug() << "Load result:" << result;
        // qDebug() << "3 - Document load attempt completed";

        if (result != QPdfDocument::Error::None) {
            // qDebug() << "4 - Document load error";
            QString errorMessage;
            switch (result) {
            case QPdfDocument::Error::InvalidFileFormat:
                errorMessage = "Invalid PDF format";
                // qDebug() << "5 - Invalid format";
                break;
            case QPdfDocument::Error::IncorrectPassword:
                errorMessage = "PDF is password protected";
                // qDebug() << "6 - Password protected";
                break;
            case QPdfDocument::Error::UnsupportedSecurityScheme:
                errorMessage = "Unsupported security scheme";
                // qDebug() << "7 - Unsupported security";
                break;
            default:
                errorMessage = "Unknown error loading PDF";
                // qDebug() << "7 - Unknown error";
            }

            QApplication::restoreOverrideCursor();
            emit error(errorMessage);
            return false;
        }

        // Wait for document to be ready
        int attempts = 0;
        qDebug() << "Initial document status:" << m_document->status();

        while (m_document->status() != QPdfDocument::Status::Ready && attempts < 50) {
            // qDebug() << "8 - Waiting for document to be ready, attempt:" << attempts;
            QApplication::processEvents();
            QThread::msleep(100);
            attempts++;
            // qDebug() << "Current document status:" << m_document->status();
        }

        if (m_document->status() != QPdfDocument::Status::Ready) {
            // qDebug() << "9 - Document failed to become ready";
            QApplication::restoreOverrideCursor();
            emit error("PDF document failed to load");
            return false;
        }

        // qDebug() << "10 - Document is ready";

        // Store file info
        m_filePath = filePath;
        m_presentationTitle = fileInfo.fileName();
        m_totalPages = m_document->pageCount();
        m_currentPage = 0;

        // qDebug() << "Document info - title:" << m_presentationTitle
        //          << "pages:" << m_totalPages
        //          << "status:" << m_document->status();

        // Reset zoom level
        m_zoomLevel = 1.0;

        // Clear highlights
        m_highlights.clear();

        // Configure PDF view
        if (m_pdfView) {
            // qDebug() << "11 - Configuring PDF view";
            m_pdfView->setPageMode(QPdfView::PageMode::SinglePage);
            m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

            // qDebug() << "Setting document on view...";
            m_pdfView->setDocument(m_document);

            // qDebug() << "Setting zoom factor...";
            m_pdfView->setZoomFactor(m_zoomLevel);

            // qDebug() << "12 - PDF view configured";

            // Ensure the view is updated
            m_pdfView->update();
            // qDebug() << "13 - View updated";

            // Go to first page after a short delay
            QTimer::singleShot(100, this, [this]() {
                goToSlide(0);
                // qDebug() << "14 - Initial page set";
            });
        } else {
            qDebug() << "Warning: PDF view is null";
        }

        // qDebug() << "15 - Emitting presentationLoaded signal";
        emit presentationLoaded(m_presentationTitle, m_totalPages);
        // qDebug() << "16 - Signal emitted";

        QApplication::restoreOverrideCursor();

        // Update page indicators AFTER restoring cursor and with delay
        QTimer::singleShot(300, this, [this]() {
            // qDebug() << "Delayed page indicators update starting...";
            safeUpdatePageIndicators();
            // qDebug() << "17 - Page indicators updated";
        });

        // qDebug() << "Loaded presentation:" << m_presentationTitle << "with" << m_totalPages << "pages at path:" << m_filePath;

        return true;
    } catch (const std::exception& e) {
        QApplication::restoreOverrideCursor();
        // qDebug() << "Exception while loading PDF:" << e.what();
        emit error(QString("Error loading PDF: %1").arg(e.what()));
        return false;
    } catch (...) {
        QApplication::restoreOverrideCursor();
        // qDebug() << "Unknown exception while loading PDF";
        emit error("Unknown error occurred while loading PDF");
        return false;
    }
}

void PresentationManager::setPdfView(QPdfView *view)
{
    if (m_pdfView == view) {
        return;
    }

    // Clear old view
    if (m_pdfView) {
        m_pdfView->setDocument(nullptr);
        disconnect(m_pdfView, nullptr, this, nullptr);
    }

    m_pdfView = view;

    if (m_pdfView) {
        // Configure the view
        m_pdfView->setPageMode(QPdfView::PageMode::SinglePage);
        m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);

        // If a document is already loaded, set it on the view
        if (m_document && !m_filePath.isEmpty()) {
            m_pdfView->setDocument(m_document);
            m_pdfView->setZoomFactor(m_zoomLevel);
            QTimer::singleShot(100, this, [this]() {
                goToSlide(m_currentPage);
            });
        }
    }
}

void PresentationManager::nextSlide(int i)
{
    static QElapsedTimer lastCallTime;
    if (lastCallTime.isValid() && lastCallTime.elapsed() < 200) {  // 200ms içinde ikinci çağrıyı yok say

        return;
    }

    lastCallTime.restart();
    qDebug() << "NextSlide accepted";

    if (isValidPage(m_currentPage + i)) {
        goToSlide(m_currentPage + i);
    }
}

void PresentationManager::previousSlide(int i)
{
    static QElapsedTimer lastCallTime;
    if (lastCallTime.isValid() && lastCallTime.elapsed() < 200) {  // 200ms içinde ikinci çağrıyı yok say

        return;
    }

    lastCallTime.restart();

    if (isValidPage(m_currentPage - i)) {
        goToSlide(m_currentPage - i);
    }
}

void PresentationManager::goToSlide(int pageNumber)
{
    try {
        qDebug() << "Attempting to go to slide:" << pageNumber;

        if (!m_document || !m_pdfView) {
            qDebug() << "Cannot change page: document or view is null";
            return;
        }

        // Check if the page number is valid
        if (pageNumber >= 0 && pageNumber < m_document->pageCount()) {
            qDebug() << "Page number is valid, changing to page:" << pageNumber;
            m_currentPage = pageNumber;

            // Sayfa değişimi pageNavigator üzerinden yapılır
            if (m_pdfView->pageNavigator()) {
                qDebug() << "Using page navigator to jump to page";
                m_pdfView->pageNavigator()->jump(m_currentPage, QPoint());
                if (m_pdfView) {
                    m_pdfView->update();
                }
                // m_pdfView->update(); // Force update of the view
            } else {
                qDebug() << "Page navigator is null";
            }

            // Güvenli şekilde page indicators güncelle
            safeUpdatePageIndicators();
            qDebug() << "Changed to page:" << m_currentPage + 1 << "of" << m_document->pageCount();
        } else {
            qDebug() << "Invalid page number:" << pageNumber;
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in goToSlide:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in goToSlide";
    }
}

void PresentationManager::zoomIn()
{
    static QElapsedTimer lastCallTime;
    if (lastCallTime.isValid() && lastCallTime.elapsed() < 200) {  // 200ms içinde ikinci çağrıyı yok say
        return;
    }

    lastCallTime.restart();
    qDebug() << "zoomin";
    setZoomLevel(m_zoomLevel * 1.25, QPointF(-1, -1));
}

void PresentationManager::zoomOut()
{
    static QElapsedTimer lastCallTime;
    if (lastCallTime.isValid() && lastCallTime.elapsed() < 200) {  // 200ms içinde ikinci çağrıyı yok say
        return;
    }

    lastCallTime.restart();
    setZoomLevel(m_zoomLevel * 0.8, QPointF(-1, -1));
}

void PresentationManager::setZoomLevel(qreal zoomFactor, const QPointF &center)
{
    // Limit zoom range
    if (zoomFactor < 0.25) zoomFactor = 0.25;
    if (zoomFactor > 5.0) zoomFactor = 5.0;

    m_zoomLevel = zoomFactor;
    if (m_pdfView) {
        m_pdfView->setZoomFactor(m_zoomLevel);

        // If a center point is provided, adjust the scroll position
        if (center != QPointF(-1, -1)) {
            QSize viewportSize = m_pdfView->viewport()->size();
            QPointF newViewTopLeft = center - QPointF(viewportSize.width() * 0.5, viewportSize.height() * 0.5);
            m_pdfView->horizontalScrollBar()->setValue(static_cast<int>(newViewTopLeft.x()));
            m_pdfView->verticalScrollBar()->setValue(static_cast<int>(newViewTopLeft.y()));
        }
    }
    emit zoomChanged(m_zoomLevel);
}

void PresentationManager::setZoomLevel(qreal zoomFactor) {
    setZoomLevel(zoomFactor, QPointF(-1, -1));
}

void PresentationManager::highlight(const QRect &area)
{
    m_highlights.append(area);

    // Here you would implement the actual highlight rendering
    // This depends on your UI implementation, but could involve
    // adding a transparent colored rectangle over the PDF view

    emit highlightApplied(area);
}

void PresentationManager::clearHighlights()
{
    m_highlights.clear();
    emit highlightsCleared();
}

int PresentationManager::getCurrentPage() const
{
    return m_currentPage;
}

int PresentationManager::getTotalPages() const
{
    return m_totalPages;
}

qreal PresentationManager::getZoomLevel() const
{
    return m_zoomLevel;
}

QString PresentationManager::getPresentationTitle() const
{
    return m_presentationTitle;
}

void PresentationManager::updatePageIndicators()
{
    try {
        qDebug() << "Updating page indicators...";
        qDebug() << "Current page:" << m_currentPage;
        qDebug() << "Total pages:" << m_totalPages;

        // Önce temel kontrolleri yap
        if (m_totalPages <= 0) {
            qDebug() << "Invalid total pages count:" << m_totalPages;
            return;
        }

        if (m_currentPage < 0) {
            qDebug() << "Current page is negative, setting to 0";
            m_currentPage = 0;
        }

        if (m_currentPage >= m_totalPages) {
            qDebug() << "Current page exceeds total pages, setting to last page";
            m_currentPage = m_totalPages - 1;
        }

        // Signal emit etmeden önce bağlantıları kontrol et
        qDebug() << "About to emit pageChanged signal...";

        // GEÇICI TEST: Signal'i devre dışı bırak
        if (QObject::receivers(SIGNAL(pageChanged(int, int))) > 0) {
            qDebug() << "Signal has" << QObject::receivers(SIGNAL(pageChanged(int, int))) << "receivers";
            qDebug() << "TEMPORARILY SKIPPING pageChanged signal emission for testing";

            // YORUM: Bu satırları tekrar aç eğer test başarılı olursa
            /*
            // Thread kontrolü yap
            if (QThread::currentThread() == QApplication::instance()->thread()) {
                qDebug() << "Emitting from main thread";
                emit pageChanged(m_currentPage, m_totalPages);
                qDebug() << "Signal emitted successfully";
            } else {
                qDebug() << "Not in main thread, using QueuedConnection";
                QMetaObject::invokeMethod(this, [this]() {
                    emit pageChanged(m_currentPage, m_totalPages);
                    qDebug() << "Signal emitted via QueuedConnection";
                }, Qt::QueuedConnection);
            }
            */
        } else {
            qDebug() << "No receivers for pageChanged signal";
        }

    } catch (const std::exception& e) {
        qDebug() << "Exception in updatePageIndicators:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in updatePageIndicators";
    }
}

// Güvenli update fonksiyonu
void PresentationManager::safeUpdatePageIndicators()
{
    try {
        qDebug() << "Safe update page indicators called";

        // Ana thread'de mi kontrolü
        if (QThread::currentThread() == QApplication::instance()->thread()) {
            qDebug() << "Already in main thread, calling updatePageIndicators directly";
            updatePageIndicators();
        } else {
            qDebug() << "Not in main thread, queuing update";
            // Qt::QueuedConnection kullanarak async olarak çağır
            QMetaObject::invokeMethod(this, [this]() {
                qDebug() << "Queued updatePageIndicators executing";
                updatePageIndicators();
            }, Qt::QueuedConnection);
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in safeUpdatePageIndicators:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in safeUpdatePageIndicators";
    }
}

bool PresentationManager::isValidPage(int page) const
{
    return (page >= 0 && page < m_totalPages);
}
