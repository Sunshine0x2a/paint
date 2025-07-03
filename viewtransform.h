#ifndef VIEWTRANSFORM_H
#define VIEWTRANSFORM_H

#include <QPointF>
#include <QRectF>

class ViewTransform {
public:
    ViewTransform(QRectF wB, QPointF c);
    QRectF viewPort;     // 视图窗口
    QRectF worldBounds;  // 世界坐标系范围
    double scale;        // 缩放因子
    QPointF center;      // 视图中心
    QPointF WTV(QPointF worldPos) const;
    QPointF VTW(QPointF viewPos) const;
    QRectF WTV(QRectF worldPos) const;
    QRectF VTW(QRectF worldPos) const;
    QPointF GTW(QPointF globalPos) const;  // 实际作用是转化位移
    void translate(double x, double y);
    void translate(QPointF p);
};

#endif  // VIEWTRANSFORM_H
