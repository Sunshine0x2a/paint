#include "figure.h"

Figure::Figure() {}

Figure::Figure(const Figure &other)
    : selected(false),
      bdrect(other.bdrect),
      pen(other.pen),
      brush(other.brush),
      type(other.type),
      viewTf(other.viewTf)  // 目前实现的是单个画布，实现多画布时在进行改动
{
    for (auto &it : other.ctrlPtList) {
        ctrlPtList.push_back(it->clone());
    }
    for (auto it : ctrlPtList) {
        it->setFig(this);
    }
}

Figure::~Figure() {
    for (auto it : ctrlPtList) {
        delete it;
    }
    ctrlPtList.clear();
}

void Figure::moveTo(QPointF p) { moveTo(p.x(), p.y()); }

void Figure::print() {
    qDebug() << "Type:" << type;
    qDebug() << "selected:" << selected;
    qDebug() << "bdrect:" << bdrect;
    qDebug() << "ListSize:" << ctrlPtList.size();
}

void Figure::setSelected(bool i) { selected = i; }

void Figure::adjust(std::vector<QPointF> p) {
    if (p.size() != ctrlPtList.size()) {
        qDebug() << "adjust::无效的列表";
        return;
    }
    for (int i = 0; i < p.size(); i++) {
        ctrlPtList[i]->ctrlMoveTo(p[i]);
    }
}

QRectF Figure::boundingRect() { return bdrect; }

void Figure::setVTf(ViewTransform *v) { viewTf = v; }

std::vector<QPointF> Figure::getCtrlPoint() {
    std::vector<QPointF> l;
    for (auto &it : ctrlPtList) {
        l.push_back(it->bdrect.center());
    }
    return l;
}

Figure::FigType Figure::getType() { return type; }

ControlPoint::ControlPoint(Figure *f, dir i, QPointF p) {
    fig = f;
    type = i;
    bdrect = QRectF{p - QPointF{rad, rad}, p + QPointF{rad, rad}};
}

ControlPoint::ControlPoint(const ControlPoint &other) {
    fig = other.fig;
    type = other.type;
    bdrect = other.bdrect;
    rad = other.rad;
}

ControlPoint::~ControlPoint() { qDebug() << "ctrlPt released" << this; }

void ControlPoint::paint(QPainter *painter) {
    painter->save();
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    // 动态调整控制点的大小

    painter->drawRect(fig->viewTf->WTV(bdrect));

    painter->restore();
}

void ControlPoint::translate(double x, double y) {
    bdrect.translate(x, y);
}
void ControlPoint::translate(QPointF p) { bdrect.translate(p); }

void ControlPoint::ctrlTranslate(double x, double y) {
    switch (type) {
        case TopLeft:
            fig->adjust(x, y, 0, 0);
            break;
        case TopRight:
            fig->adjust(0, y, x, 0);
            break;
        case BottomLeft:
            fig->adjust(x, 0, 0, y);
            break;
        case BottomRight:
            fig->adjust(0, 0, x, y);
            break;
    }
}

void ControlPoint::ctrlTranslate(QPointF p) { ctrlTranslate(p.x(), p.y()); }

void ControlPoint::ctrlMoveTo(double x, double y) { ctrlMoveTo({x, y}); }

void ControlPoint::ctrlMoveTo(QPointF p) { ctrlTranslate(p - bdrect.center()); }

void ControlPoint::setFig(Figure *f) { fig = f; }

ControlPoint *ControlPoint::clone() { return new ControlPoint(*this); }

bool ControlPoint::contain(QPointF p) { return bdrect.contains(p); }

Figure *ControlPoint::getParent() { return fig; }

RectFig::RectFig(QPointF p, int w, int h) {
    bdrect = QRectF{p, QPointF(p.x() + w, p.y() + h)};
    selected = false;
    ctrlPtList.push_back(
        new ControlPoint(this, ControlPoint::TopLeft, bdrect.topLeft()));
    ctrlPtList.push_back(
        new ControlPoint(this, ControlPoint::TopRight, bdrect.topRight()));
    ctrlPtList.push_back(
        new ControlPoint(this, ControlPoint::BottomLeft, bdrect.bottomLeft()));
    ctrlPtList.push_back(new ControlPoint(this, ControlPoint::BottomRight,
                                          bdrect.bottomRight()));
    qDebug() << "a rect is created";
}

RectFig::RectFig(const RectFig &other) : Figure(other) {}

void RectFig::paint(QPainter *painter) const {
    painter->save();
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawRect(viewTf->WTV(bdrect));
    if (selected) {
        QPen sectionPen(Qt::DashLine);
        sectionPen.setColor(Qt::blue);
        sectionPen.setWidth(3);
        painter->setPen(sectionPen);
        painter->drawRect(viewTf->WTV(bdrect));
        for (auto it : ctrlPtList) {
            it->paint(painter);
        }
    }
    painter->restore();
}

void RectFig::translate(double x, double y) {
    bdrect.translate(x, y);
    for (auto it : ctrlPtList) {
        it->translate(x, y);
    }
}

void RectFig::moveTo(double x, double y) {
    for (auto it : ctrlPtList) {
        it->translate(QPoint(x, y) - bdrect.topLeft());
    }
    bdrect.moveTo(x, y);
}

int RectFig::contain(QPointF p) const {
    if (bdrect.contains(p)) {
        return 2;
    } else
        return 0;
}

RectFig *RectFig::clone() { return new RectFig(*this); }

void RectFig::adjust(double x, double y, double x0, double y0) {
    bdrect.adjust(x, y, x0, y0);
    ctrlPtList[0]->translate(x, y);
    ctrlPtList[1]->translate(x0, y);
    ctrlPtList[2]->translate(x, y0);
    ctrlPtList[3]->translate(x0, y0);
}

CpsFig::CpsFig(std::vector<Figure *> f) {
    type = Cps;
    figList = f;
    bdrect = f[0]->boundingRect();
    for (auto it : f) {
        bdrect = bdrect.united(it->boundingRect());
    }
}

CpsFig::CpsFig(const CpsFig &other) : Figure(other) {
    for (auto it : other.figList) {
        auto i = it->clone();
        figList.push_back(i);
    }
}

void CpsFig::paint(QPainter *painter) const {
    if (selected) {
        for (auto &it : figList) {
            it->selected = true;
        }
    } else {
        for (auto &it : figList) {
            it->selected = false;
        }
    }
    for (auto &it : figList) {
        it->paint(painter);
    }
}

void CpsFig::translate(double x, double y) {
    for (auto it : figList) {
        it->translate(x, y);
    }
}

void CpsFig::moveTo(double x, double y) {
    for (auto it : figList) {
        it->translate(x - bdrect.x(), y - bdrect.y());
    }
}

int CpsFig::contain(QPointF p) const {
    int ans = 0;
    for (auto &it : figList) {
        ans = std::max(ans, it->contain(p));
        if (ans == 2) {  // 检测到一个精准移动时，直接返回
            return ans;
        }
    }
    return ans;
}

CpsFig *CpsFig::clone() { return new CpsFig(*this); }

void CpsFig::adjust(double x, double y, double x0, double y0) {}

std::vector<Figure *> CpsFig::List() { return figList; }
