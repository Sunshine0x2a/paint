#ifndef CANVA_H
#define CANVA_H

#include <QBrush>
#include <QColor>
#include <QColorDialog>
#include <QCursor>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <vector>

#include "command.h"
#include "figure.h"
#include "viewtransform.h"

class Canva : public QWidget {
    Q_OBJECT
public:
    explicit Canva(QWidget *parent = nullptr);
    ~Canva();
    QPen dftPen;
    QBrush dftBrush;
    void addToList(Figure *);
    void removeFromList(Figure *);
    void addToSelList(Figure *);
    void removeFromSelList(Figure *);
    void addToCtrlPtList(std::vector<ControlPoint *>);
    void removeFromCtrlPtList(std::vector<ControlPoint *>);
    QPointF viewPos;           // 鼠标视图位置
    QPointF worldPos;          // 鼠标在逻辑世界中的位置
    QPointF globalPos;         // 鼠标全局位置
    QPointF lastRightViewPos;  // 记录最后一次鼠标右键的位置，主要用于创建图形
    QPointF lastWorldPos;      // 记录移动图形时鼠标的位置
    QPointF lastGlobalPos;
    QPointF tempLastWorldPos;  // 临时记录移动图形时鼠标的位置，实现平滑移动
    QPointF tempLastGlobalPos;
    ViewTransform *viewTf;
    std::vector<Figure *> &SELLIST();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    std::vector<Figure *> figList;
    std::vector<Figure *> selList;     // 选中图形列表
    std::vector<Figure *> preSelList;  // 记录过去选择的图形列表
    std::vector<ControlPoint *> ctrlPtList;
    std::vector<QPointF> preCtrlPtLocation;  // 记录过去控制图形的位置
    QMenu *normalMenu;  // 未选中图形下的菜单
    QMenu *figMenu;     // 2级图形菜单
    QMenu *selMenu;     // 有选中图形的菜单
    QMenu *penColorMenu;  // 颜色菜单
    QMenu *brushColorMenu;
    CmdStack *cmdStack;
    QTimer *fpstimer;
    QRectF worldBound = {2000, 2000, 2000, 2000};
    bool is_panning = false;    // 记录否在平移视图
    bool is_selecting = false;  // 记录是否在进行多选
    bool is_zooming = false;    // 记录是否在缩放屏幕
    bool is_dragging = false;   // 记录是否在拖拽图形
    bool is_adjusting = false;
    bool has_selCPt = false;    // 记录是否已经选中了控制点
    double preScale = 1;
    double newScale = 1;  // 分别记录缩放
    Figure *copyFig = nullptr;
    ControlPoint *selCPt = nullptr;
    void onAdjust();
    void onCreate(Figure::FigType type);
    void handleSingleSel();
    void handleMultipleSel();
    template <typename T>
    bool contains(const std::vector<T> &vec, const T &element) {
        return std::find(vec.begin(), vec.end(), element) != vec.end();
    }
    void initContextMenu();
    void updateNormalMenu();
    void updateSelMenu();
    void addColorActions(QMenu *menu,
                         std::function<void(const QColor &)> callback);

private slots:
    void onCopy();
    void onPaste();
    void onCancelCps();

    friend class AddFigCmd;
    friend class CpsFigCmd;  // 仅用于调用控制点列表
    friend class CancelCpsCmd;
};

#endif  // CANVA_H
