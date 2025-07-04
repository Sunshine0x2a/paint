#ifndef FIGURE_H
#define FIGURE_H

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <algorithm>
#include <memory>

#include "viewtransform.h"

class ControlPoint;

class Figure : public std::enable_shared_from_this<Figure> {  // 抽象基类
public:
    Figure();
    Figure(const Figure &other);
    ~Figure();
    enum FigType { Rect, CopyCompose, Cps };
    bool selected;
    virtual void paint(QPainter *painter) const = 0;
    virtual void translate(double x,
                           double y) = 0;         // 平移该图形(以bdrect为主体)
    virtual void moveTo(double x, double y) = 0;  // 移动至    
    void moveTo(QPointF p);
    virtual Figure *clone() = 0;
    virtual int contain(QPointF p) const = 0;
    virtual void print();  // 调试用
    virtual void setSelected(bool);
    virtual void adjust(double x, double y, double x0, double y0) = 0;
    void adjust(std::vector<QPointF> p);

    QRectF boundingRect();
    void setPen(QPen p) { pen = p; };
    void setBrush(QBrush b) { brush = b; };
    void setVTf(ViewTransform *v);
    std::vector<QPointF> getCtrlPoint();
    FigType getType();

    friend class ControlPoint;
    friend class Canva;

protected:
    QRectF bdrect;  // 碰撞长方形;
    FigType type;
    QPen pen;
    QBrush brush;
    ViewTransform *viewTf;
    std::vector<std::shared_ptr<ControlPoint>> ctrlPtList;
    bool isdeleted = false;
};

class ControlPoint {
public:
    enum dir {
        Common,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right
    };
    ControlPoint(std::shared_ptr<Figure> f, dir i, QPointF p);
    ControlPoint(const ControlPoint &other);
    ~ControlPoint();
    dir type;
    QRectF bdrect;
    double rad = 3;  // 半径
    void paint(QPainter *painter);
    void moveTo();
    void translate(double x, double y);  // 纯粹移动
    void translate(QPointF p);
    void ctrlTranslate(double x, double y);  // 控制点移动
    void ctrlTranslate(QPointF p);
    void ctrlMoveTo(double x, double y);
    void ctrlMoveTo(QPointF p);
    void setFig(std::shared_ptr<Figure> fig);
    ControlPoint *clone();
    bool contain(QPointF p);
    std::weak_ptr<Figure> getParent();

private:
    std::weak_ptr<Figure> fig;
};

class RectFig : public Figure {
public:
    RectFig(QPointF p, int w, int h);
    RectFig(const RectFig &other);
    void paint(QPainter *painter) const override;
    void translate(double x, double y) override;
    void moveTo(double x, double y) override;
    int contain(QPointF p) const override;
    RectFig *clone() override;
    void adjust(double x, double y, double x0, double y0) override;
    // 上左，上右，下左，下右
};

class CpsFig : public Figure {
public:
    // CpsFig(QPointF p,int w, int h);
    CpsFig(std::vector<std::shared_ptr<Figure>> f);
    CpsFig(const CpsFig &other);
    void paint(QPainter *painter) const override;
    void translate(double x, double y) override;
    void moveTo(double x, double y) override;
    int contain(QPointF p) const override;
    CpsFig *clone() override;
    void adjust(double x, double y, double x0, double y0) override;

    std::vector<std::shared_ptr<Figure>> List();  // 调试用

private:
    std::vector<std::shared_ptr<Figure>> figList;
};

#endif  // FIGURE_H
