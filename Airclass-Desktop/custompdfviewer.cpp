#include "custompdfviewer.hpp"

#include <QPdfPageNavigator>
#include <QScrollBar>
#include <iostream>

customPdfViewer::customPdfViewer(QWidget *parent) : QPdfView(parent) {
  qDebug() << "Custom PDF Viewer created";
  for (int i = 0; i < 100; ++i) points.append(QPointF(i, i));

  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          [this](int value) { this->scroll = value; });
}

void customPdfViewer::paintEvent(QPaintEvent *event) {
  // setZoomFactor(2);
  // verticalScrollBar()->setSliderPosition(200);

  QPdfView::paintEvent(event);
  QPainter painter(viewport());
  qDebug() << "Painting custom points";
  painter.setPen(QPen(Qt::red, 2));

  int pageIndex = pageNavigator()->currentPage();
  qreal zoom = zoomFactor();
  std::cout << "Current page index: " << pageIndex << std::endl;
  std::cout << "Current location: " << scroll << std::endl;
  std::cout << "Current zoom: " << zoom << std::endl;
  for (const QPointF &point : points) {
    QPointF adjustedPoint = point + QPointF(0, -scroll);
    adjustedPoint *= zoom;
    if (pageIndex == 0) {
      painter.drawPoint(adjustedPoint);
    }
  }
}
