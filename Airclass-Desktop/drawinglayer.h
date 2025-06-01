#ifndef DRAWINGLAYER_H
#define DRAWINGLAYER_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <QMap>

class DrawingLayer : public QWidget
{
    Q_OBJECT

public:
    explicit DrawingLayer(QWidget *parent = nullptr);
    void clear();
    void clearCurrentPage();
    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setDrawingEnabled(bool enabled);
    void setCurrentPage(int page);
    QColor getPenColor() const { return m_penColor; }
    bool isDrawingEnabled() const { return m_isDrawingEnabled; }
    void showPointer(double x, double y);
    void updateScale(qreal scale);
    void updateScrollPosition(const QPoint &scrollPos);

public slots:
    void drawRemotePoint(const QPointF &point, bool isStart);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    struct DrawingPath {
        QVector<QPointF> points;
        QColor color;
        int width;
    };

    QMap<int, QVector<DrawingPath>> m_pageDrawings; // Drawings for each page
    QVector<DrawingPath> m_paths;
    QVector<QPointF> m_currentPath;
    QColor m_penColor;
    int m_penWidth;
    bool m_isDrawing;
    bool m_isDrawingEnabled;
    QPointF m_lastPoint;
    QList<QPointF> m_points;
    QPointF m_pointerPosition;
    bool m_showPointer;
    int m_currentPage;
    qreal m_scale;
    QPoint m_scrollPosition;

    QPointF scalePoint(const QPointF &point) const;
    QPointF unscalePoint(const QPointF &point) const;
    QPointF adjustForScroll(const QPointF &point) const;
};

#endif // DRAWINGLAYER_H 