#ifndef PRESENTATIONMANAGER_H
#define PRESENTATIONMANAGER_H

#include <QObject>
#include <QPdfDocument>
#include <QPdfView>
#include <QString>
#include <QRect>
#include <QPointF>
#include <QStringList>

class PresentationManager : public QObject
{
    Q_OBJECT
public:
    explicit PresentationManager(QObject *parent = nullptr);
    ~PresentationManager();

    bool loadPresentation(const QString &filePath);
    void setPdfView(QPdfView *view);
    void safeUpdatePageIndicators();
    void nextSlide(int i);
    void previousSlide(int i);
    void goToSlide(int pageNumber);

    void zoomIn();
    void zoomOut();
    void setZoomLevel(qreal zoomFactor, const QPointF &center = QPointF(-1, -1));
    void setZoomLevel(qreal zoomFactor);
    void highlight(const QRect &area);
    void clearHighlights();

    int getCurrentPage() const;
    int getTotalPages() const;
    qreal getZoomLevel() const;
    QString getPresentationTitle() const;

    void openDocument(const QString &filePath);
    void addRecentPresentation(const QString &filePath);
    QStringList getRecentPresentations() const;
    void clearRecentPresentations();
    QPdfDocument* getDocument() const { return m_document; }

signals:
    void pageChanged(int currentPage, int totalPages);
    void zoomChanged(qreal zoomLevel);
    void presentationLoaded(const QString &title, int totalPages);
    void highlightApplied(const QRect &area);
    void highlightsCleared();
    void error(const QString &errorMessage);
    void documentLoaded();
    void documentClosed();
    void gestureDetected(const QString &gestureType);

private:
    QPdfDocument *m_document;
    QPdfView *m_pdfView;
    int m_currentPage;
    int m_totalPages;
    qreal m_zoomLevel;
    QString m_presentationTitle;
    QString m_filePath;
    QList<QRect> m_highlights;
    QStringList m_recentPresentations;

    void updatePageIndicators();
    bool isValidPage(int page) const;
    void loadRecentPresentations();
    void saveRecentPresentations();
};

#endif // PRESENTATIONMANAGER_H
