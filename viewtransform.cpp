#include "viewtransform.h"

ViewTransform::ViewTransform(QRectF wB, QPointF c)
    : worldBounds(wB), center(c), scale(1) {}

QPointF ViewTransform::WTV(QPointF worldPos) const {
    return {scale * (worldPos.x() - center.x()) + viewPort.width() / 2,
            scale * (worldPos.y() - center.y()) + viewPort.height() / 2};
}

QPointF ViewTransform::VTW(QPointF viewPos) const {
    return {(viewPos.x() - viewPort.width() / 2) / scale + center.x(),
            (viewPos.y() - viewPort.height() / 2) / scale + center.y()};
}

QRectF ViewTransform::WTV(QRectF worldPos) const {
    return QRectF(WTV(worldPos.topLeft()), WTV(worldPos.bottomRight()));
}

QRectF ViewTransform::VTW(QRectF worldPos) const {
    return QRectF(VTW(worldPos.topLeft()), VTW(worldPos.bottomRight()));
}

QPointF ViewTransform::GTW(QPointF globalPos) const {
    return globalPos / scale;
}

void ViewTransform::translate(double x, double y) {
    center.rx() += x;
    center.ry() += y;
}

void ViewTransform::translate(QPointF p) { translate(p.x(), p.y()); }
