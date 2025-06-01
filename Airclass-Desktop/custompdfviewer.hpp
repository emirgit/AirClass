#ifndef CUSTOMPDFVIEWER_HPP
#define CUSTOMPDFVIEWER_HPP

#include <QDebug>
#include <QPainter>
#include <QPdfView>
#include <QPen>

class customPdfViewer : public QPdfView {
 public:
  customPdfViewer(QWidget *parent = nullptr);
  int scroll = 0;

 protected:
  QList<QPointF> points;

  void paintEvent(QPaintEvent *event) override;
};

#endif  // CUSTOMPDFVIEWER_HPP
