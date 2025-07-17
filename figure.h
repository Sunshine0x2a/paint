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

inline double operator*(const QPointF &p1, const QPointF &p2) {
    return QPointF::dotProduct(p1, p2);
}

class Figure {  // 抽象基类
public:
    Figure();
    Figure(const Figure &other);
    ~Figure();
    enum FigType { Rect, CopyCompose, Cps, Ell, Line };
    bool selected = false;
    bool inCps = false;
    virtual void paint(QPainter *painter) const = 0;
    virtual void translate(double x,
                           double y) = 0;         // 平移该图形(以bdrect为主体)
    virtual void moveTo(double x, double y) = 0;  // 移动至    
    void moveTo(QPointF p);
    virtual Figure *clone() = 0;
    virtual int contain(QPointF p) const = 0;
    virtual void print();  // 调试用
    virtual void setSelected(bool);
    virtual void adjust(double x, double y, double x0, double y0);
    std::vector<ControlPoint *> getCtrlList();
    // 默认的包围盒的实现，每一种图形都有这个统一的接口,且通过这个函数进行调整时，说明是通过包围盒调整
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
    std::vector<ControlPoint *> ctrlPtList;
    bool isdeleted = false;
};

class ControlPoint {
public:
    enum dir {
        Common,  // 除Common外其他都为特殊控制点，用以指示包围盒的变动
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right
    };
    ControlPoint(Figure *f, dir i, QPointF p);
    ControlPoint(const ControlPoint &other);
    ~ControlPoint();
    dir type;
    QRectF bdrect;
    double rad = 3;  // 半径
    double rx;
    double ry;
    void paint(QPainter *painter);
    void moveTo();
    void translate(double x, double y);  // 纯粹移动
    void translate(QPointF p);
    void moveTo(double x, double y);
    void moveTo(QPointF p);
    void ctrlTranslate(double x, double y);  // 控制点移动
    void ctrlTranslate(QPointF p);
    void ctrlMoveTo(double x, double y);
    void ctrlMoveTo(QPointF p);
    void setFig(Figure *fig);
    ControlPoint *clone();
    bool contain(QPointF p);
    Figure *getParent();

private:
    Figure *fig;
};

class CpsFig : public Figure {
public:
    // CpsFig(QPointF p,int w, int h);
    CpsFig(std::vector<Figure *> f);
    CpsFig(const CpsFig &other);
    void paint(QPainter *painter) const override;
    void translate(double x, double y) override;
    void moveTo(double x, double y) override;
    int contain(QPointF p) const override;
    CpsFig *clone() override;
    void adjust(double x, double y, double x0, double y0) override;
    void setSelected(bool b) override;

    std::vector<Figure *> List();  // 调试用

private:
    std::vector<Figure *> figList;
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
    void adjust(double x, double y, double x0,
                double y0) override;  // 矩形重写提高效率
    //  上左，上右，下左，下右
};

class EllFig : public Figure {
public:
    EllFig(QPointF p, int w, int h);
    EllFig(const EllFig &other);
    EllFig *clone() override;
    void paint(QPainter *painter) const override;
    void translate(double x, double y) override;
    void moveTo(double x, double y) override;
    int contain(QPointF p) const override;
    void adjust(double x, double y, double x0,
                double y0) override;  // 重写提高效率
};

class Line : public Figure {
public:
    Line(QPointF p1, QPointF p2);
    Line(const Line &other);
    Line *clone() override;
    void paint(QPainter *painter) const override;
    void translate(double x, double y) override;
    void moveTo(double x, double y) override;
    int contain(QPointF p) const override;
    void adjust(double x, double y, double x0,
                double y0) override;  // 重写提高效率
};

#endif  // FIGURE_H
