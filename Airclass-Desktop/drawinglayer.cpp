#include "drawinglayer.h"
#include <QPainter>
#include <QMouseEvent>

DrawingLayer::DrawingLayer(QWidget *parent)
    : QWidget(parent)
    , m_penColor(Qt::red)
    , m_penWidth(2)
    , m_isDrawing(false)
    , m_isDrawingEnabled(true)
    , m_showPointer(false)
    , m_currentPage(0)
    , m_scale(1.0)
    , m_scrollPosition(0, 0)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
}

void DrawingLayer::clear()
{
    m_pageDrawings.clear();
    m_paths.clear();
    m_currentPath.clear();
    update();
}

void DrawingLayer::clearCurrentPage()
{
    m_pageDrawings[m_currentPage].clear();
    m_paths.clear();
    m_currentPath.clear();
    update();
}

void DrawingLayer::setPenColor(const QColor &color)
{
    m_penColor = color;
}

void DrawingLayer::setPenWidth(int width)
{
    m_penWidth = width;
}

void DrawingLayer::setDrawingEnabled(bool enabled)
{
    m_isDrawingEnabled = enabled;
    if (!enabled && m_isDrawing) {
        m_isDrawing = false;
        if (!m_currentPath.isEmpty()) {
            DrawingPath path;
            path.points = m_currentPath;
            path.color = m_penColor;
            path.width = m_penWidth;
            m_pageDrawings[m_currentPage].append(path);
            m_currentPath.clear();
            update();
        }
    }
}

void DrawingLayer::setCurrentPage(int page)
{
    // Save current path if any
    if (!m_currentPath.isEmpty()) {
        DrawingPath path;
        path.points = m_currentPath;
        path.color = m_penColor;
        path.width = m_penWidth;
        m_pageDrawings[m_currentPage].append(path);
        m_currentPath.clear();
    }
    
    m_currentPage = page;
    update();
}

void DrawingLayer::updateScale(qreal scale)
{
    m_scale = scale;
    update();
}

QPointF DrawingLayer::scalePoint(const QPointF &point) const
{
    return point / m_scale;
}

QPointF DrawingLayer::unscalePoint(const QPointF &point) const
{
    return point * m_scale;
}

void DrawingLayer::drawRemotePoint(const QPointF &point, bool isStart)
{
    if (!m_isDrawingEnabled) return;
    
    QPointF scaledPoint = scalePoint(point);
    
    if (isStart) {
        if (!m_currentPath.isEmpty()) {
            DrawingPath path;
            path.points = m_currentPath;
            path.color = m_penColor;
            path.width = m_penWidth;
            m_pageDrawings[m_currentPage].append(path);
            m_currentPath.clear();
        }
    }
    m_currentPath.append(scaledPoint);
    update();
}

void DrawingLayer::showPointer(double x, double y)
{
    m_pointerPosition = scalePoint(QPointF(x, y));
    m_showPointer = true;
    update();
}

void DrawingLayer::updateScrollPosition(const QPoint &scrollPos)
{
    m_scrollPosition = scrollPos;
    update();
}

QPointF DrawingLayer::adjustForScroll(const QPointF &point) const
{
    return point + QPointF(m_scrollPosition);
}

void DrawingLayer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Scale the painter
    painter.scale(m_scale, m_scale);
    
    // Adjust for scroll position
    painter.translate(-m_scrollPosition);

    // Draw all completed paths for current page
    if (m_pageDrawings.contains(m_currentPage)) {
        for (const DrawingPath &path : m_pageDrawings[m_currentPage]) {
            if (path.points.size() < 2) continue;

            QPen pen(path.color, path.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);

            for (int i = 1; i < path.points.size(); ++i) {
                painter.drawLine(path.points[i-1], path.points[i]);
            }
        }
    }

    // Draw current path
    if (m_currentPath.size() >= 2) {
        QPen pen(m_penColor, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);

        for (int i = 1; i < m_currentPath.size(); ++i) {
            painter.drawLine(m_currentPath[i-1], m_currentPath[i]);
        }
    }

    // Draw pointer if active
    if (m_showPointer) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::red);
        int pointerSize = 10;
        painter.drawEllipse(m_pointerPosition, pointerSize/m_scale, pointerSize/m_scale);
        
        // Draw crosshair
        painter.drawLine(m_pointerPosition.x() - pointerSize*2/m_scale, m_pointerPosition.y(),
                        m_pointerPosition.x() + pointerSize*2/m_scale, m_pointerPosition.y());
        painter.drawLine(m_pointerPosition.x(), m_pointerPosition.y() - pointerSize*2/m_scale,
                        m_pointerPosition.x(), m_pointerPosition.y() + pointerSize*2/m_scale);
    }
}

void DrawingLayer::mousePressEvent(QMouseEvent *event)
{
    if (!m_isDrawingEnabled) return;
    
    if (event->button() == Qt::LeftButton) {
        m_isDrawing = true;
        m_currentPath.clear();
        QPointF adjustedPos = scalePoint(event->pos()) + QPointF(m_scrollPosition);
        m_currentPath.append(adjustedPos);
    }
}

void DrawingLayer::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDrawingEnabled || !m_isDrawing) return;
    
    QPointF adjustedPos = scalePoint(event->pos()) + QPointF(m_scrollPosition);
    m_currentPath.append(adjustedPos);
    update();
}

void DrawingLayer::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_isDrawingEnabled) return;
    
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;
        if (!m_currentPath.isEmpty()) {
            DrawingPath path;
            path.points = m_currentPath;
            path.color = m_penColor;
            path.width = m_penWidth;
            m_pageDrawings[m_currentPage].append(path);
            m_currentPath.clear();
            update();
        }
    }
} 