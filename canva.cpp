#include "canva.h"

Canva::Canva(QWidget *parent) : QWidget{parent} {
    fpstimer = new QTimer(this);
    connect(fpstimer, &QTimer::timeout, this, [this]() { this->update(); });
    fpstimer->start(16);  // 约60fps

    cmdStack = new CmdStack;

    // 视图管理器创建
    viewTf = new ViewTransform(worldBound, {worldBound.center()});
    viewTf->viewPort = this->geometry();

    // 菜单项创建
    initContextMenu();
    // 普通菜单项
    /*
    normalMenu = new QMenu(this);
    figMenu = normalMenu->addMenu("添加");
    figMenu->addAction("长方形", this,
                       [this]() { this->onCreate(Figure::Rect); });
    figMenu->addAction("圆形/椭圆", this,
                       [this]() { this->onCreate(Figure::Ell); });
    figMenu->addAction("线条", this,
                       [this]() { this->onCreate(Figure::Line); });
    normalMenu->addAction("粘贴", this, &Canva::onPaste);
    normalMenu->addAction("撤销", this, [this]() { cmdStack->undoCommand(); });
    normalMenu->addAction("重做", this, [this]() { cmdStack->redoCommand(); });
    normalMenu->addAction("恢复默认缩放比例", this, [this]() {
        cmdStack->executeCommand(
            std::make_shared<RecoverCmd>(this, viewTf->scale));
    });

    // 选中图形时的菜单项
    selMenu = new QMenu(this);
    selMenu->addAction("复制", this, &Canva::onCopy);
    selMenu->addAction("删除", this, [this]() {
        cmdStack->executeCommand(std::make_shared<DelFigCmd>(this, selList));
    });
    selMenu->addAction("组合", this, [this]() {
        cmdStack->executeCommand(std::make_shared<CpsFigCmd>(this, selList));
    });
    selMenu->addAction("拆散", this, &Canva::onCancelCps);
    penColorMenu = selMenu->addMenu("设置边框颜色");
    penColorMenu->addAction("无边框");
    brushColorMenu = selMenu->addMenu("设置填充颜色");
    // 启用鼠标追踪和上下文菜单
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    */
    // 测试用矩形

    dftPen = QPen(Qt::black, 2.0);
    dftBrush = QBrush(Qt::white);
    /*
     RectFig *testrect = new RectFig({200, 200}, 50, 50);
     testrect->setBrush(dftBrush);
     testrect->setPen(dftPen);
     testrect->setVTf(viewTf);
     figList.push_back(testrect);*/
}

Canva::~Canva() {
    delete cmdStack;
    delete viewTf;
}

void Canva::addToList(Figure *f) {
    figList.push_back(f);
    // qDebug() << figList.size();
}

void Canva::removeFromList(Figure *f) {
    auto it = std::find(figList.begin(), figList.end(), f);
    if (it != figList.end()) {
        figList.erase(it);
    }
}

void Canva::addToSelList(Figure *f) {
    selList.push_back(f);
    f->setSelected(true);
}

void Canva::removeFromSelList(Figure *f) {
    auto it = std::find(selList.begin(), selList.end(), f);
    if (it != selList.end()) {
        selList.erase(it);
        f->setSelected(false);
    }
}

void Canva::addToCtrlPtList(std::vector<ControlPoint *> l) {
    ctrlPtList.insert(ctrlPtList.end(), l.begin(), l.end());
}

void Canva::removeFromCtrlPtList(std::vector<ControlPoint *> l) {
    auto begin = std::find(ctrlPtList.begin(), ctrlPtList.end(), *l.begin());
    auto end = std::find(ctrlPtList.begin(), ctrlPtList.end(), *l.end());
    ctrlPtList.erase(begin, end);
}

std::vector<Figure *> &Canva::SELLIST() { return selList; }

void Canva::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.setPen(dftPen);
    painter.setBrush(dftBrush);
    for (auto &it : figList) {
        // qDebug() << it->boundingRect();
        it->paint(&painter);
    }
}
void Canva::mousePressEvent(QMouseEvent *event) {
    globalPos = QCursor::pos();
    viewPos = event->pos();
    worldPos = viewTf->VTW(viewPos);
    if (event->button() == Qt::RightButton) {
        lastRightViewPos = event->pos();
        if (selList.empty()) {
            updateNormalMenu();
            normalMenu->exec(globalPos.toPoint());

        } else {
            updateSelMenu();
            selMenu->exec(globalPos.toPoint());
        }
    }
    if (event->button() == Qt::LeftButton) {
        bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
        if (!ctrlPressed) {
            handleSingleSel();
            if (selList.empty()) {  // 如果未选中图形，则可以执行平移视图
                if (!is_panning) {
                    is_panning = true;
                    lastGlobalPos = globalPos;
                }
                tempLastGlobalPos = lastGlobalPos;
            }
        } else {  // 多选模式
            handleMultipleSel();
        }
        if (!selList.empty()) {
            if (!is_dragging) {
                lastWorldPos = viewTf->VTW(viewPos);
                is_dragging = true;
            }
            tempLastWorldPos = lastWorldPos;
        }
    }
}

void Canva::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        worldPos = viewTf->VTW(viewPos);
        if (is_dragging &&
            (lastWorldPos != worldPos)) {  // 确保发生移动时执行命令
            cmdStack->executeCommand(std::make_shared<MoveCmd>(
                this, selList, lastWorldPos, worldPos));
        }
        if (is_panning && (lastGlobalPos != globalPos)) {
            cmdStack->executeCommand(std::make_shared<PanCmd>(
                this, viewTf->GTW(globalPos - lastGlobalPos)));
        }
        is_dragging = false;
        is_panning = false;
        has_selCPt = false;
        lastWorldPos = viewTf->VTW(event->pos());
        lastGlobalPos = QCursor::pos();
    }
}

void Canva::mouseMoveEvent(QMouseEvent *event) {
    globalPos = QCursor::pos();
    viewPos = event->pos();
    worldPos = viewTf->VTW(viewPos);
    // 实现移动图片逻辑
    if (is_dragging) {
        for (auto it : selList) {
            it->translate(worldPos.x() - tempLastWorldPos.x(),
                          worldPos.y() - tempLastWorldPos.y());
        }
        tempLastWorldPos = worldPos;
    }
    // 实现视图移动逻辑
    if (is_panning) {
        viewTf->translate(viewTf->GTW(-globalPos + tempLastGlobalPos));
        tempLastGlobalPos = globalPos;
    }
    // 实现形状改变
    if (has_selCPt) {
        selCPt->ctrlTranslate((worldPos - tempLastWorldPos));
        tempLastWorldPos = worldPos;

        is_dragging = false;  // 如果选中了控制点，就不能拖动图形了
    }
}

void Canva::wheelEvent(QWheelEvent *event) {
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    QPoint angleDelta = event->angleDelta();
    if (!angleDelta.isNull() && ctrlPressed) {
        if (!is_zooming) {
            is_zooming = true;  // 确保只记录一次，即开始记录
            preScale = viewTf->scale;
        }
        viewTf->scale +=
            0.001 * angleDelta.y();  // 每单位滚动修正0.1%,限定缩放比例
        if (viewTf->scale < 0.1) viewTf->scale = 0.1;
        if (viewTf->scale > 10.0) viewTf->scale = 10.0;
        newScale = viewTf->scale;  // 刷新新的scale
    }
}

void Canva::keyPressEvent(QKeyEvent *event) {
    bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    if (ctrlPressed) {
        switch (event->key()) {
            case Qt::Key_Z:
                cmdStack->undoCommand();
                break;
            case Qt::Key_Y:
                cmdStack->redoCommand();
                break;
            case Qt::Key_C:
                onCopy();
                break;
            case Qt::Key_V:
                if (copyFig != nullptr) {
                    cmdStack->executeCommand(
                        std::make_shared<PasteCmd>(this, copyFig, worldPos));
                }
                break;
        }
    } else {
        switch (event->key()) {
            case Qt::Key_Delete:
                cmdStack->executeCommand(
                    std::make_shared<DelFigCmd>(this, selList));
                break;
            case Qt::Key_Backspace:
                cmdStack->executeCommand(
                    std::make_shared<DelFigCmd>(this, selList));
                break;
        }
    }
}

void Canva::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        if (preScale != newScale &&
            is_zooming) {  // 确保在缩放且发生变化时才存入命令栈
            qDebug() << preScale << newScale;
            cmdStack->executeCommand(
                std::make_shared<ZoomCmd>(this, preScale, newScale));
            preScale = newScale;
        }
        if (is_selecting) {
            cmdStack->executeCommand(
                std::make_shared<SelCmd>(this, preSelList, selList));
        }
        is_zooming = false;
        is_selecting = false;
    }
}

void Canva::resizeEvent(QResizeEvent *event) {
    viewTf->viewPort = this->geometry();
}

void Canva::onCreate(Figure::FigType type) {
    cmdStack->executeCommand(
        std::make_shared<AddFigCmd>(this, type, viewTf->VTW(lastRightViewPos)));
}


void Canva::handleSingleSel() {
    std::vector<Figure *> tempSelList = selList;
    for (auto &it : ctrlPtList) {  // 优先选中控制点
        if (it->contain(worldPos)) {
            if (!is_adjusting) {
                is_adjusting = true;
                preCtrlPtLocation = it->getParent()->getCtrlPoint();
            }
            if (!has_selCPt) {
                has_selCPt = true;
                lastWorldPos = viewTf->VTW(viewPos);
                tempLastWorldPos = lastWorldPos;           
            }
            selCPt = it;
            return;
        }
    }
    if (is_adjusting &&
        lastWorldPos !=
            worldPos) {  // 说明过去对操作点进行过操作且过去是有操作点的
        cmdStack->executeCommand(
            std::make_shared<AdjustCmd>(this, preCtrlPtLocation, selList[0]));
    }
    has_selCPt = false;
    is_adjusting = false;
    selCPt = nullptr;
    for (auto it = figList.rbegin(); it != figList.rend();
         it++) {  // 反向遍历，优先选中顶层
        if ((*it)->contain(worldPos)) {
            if (!contains(selList, (*it)) || selList.size() == 0) {
                // 当没选中多个已选中对象时，再执行单选逻辑
                for (auto &it : selList) {
                    it->selected = false;
                }
                selList.clear();
                selList.push_back(*it);
                (*it)->selected = true;
                ctrlPtList.clear();
                ctrlPtList = (*it)->ctrlPtList;
                cmdStack->executeCommand(
                    std::make_shared<SelCmd>(this, tempSelList, selList));
            }
            return;
        }
    }
    if (tempSelList.size() != 0) {  // 说明没有选中任何对象

        for (auto &it : selList) {
            it->selected = false;
        }
        selList.clear();
        ctrlPtList.clear();
        cmdStack->executeCommand(
            std::make_shared<SelCmd>(this, tempSelList, selList));
    }
}

void Canva::handleMultipleSel() {
    for (auto it = figList.rbegin(); it != figList.rend();
         it++) {  // 反向遍历，优先选中顶层
        if ((*it)->contain(worldPos)) {
            // 只有在选中到图形时才进行记录
            if (!is_selecting) {
                preSelList = selList;
                is_selecting = true;
            }
            if (!(*it)->selected) {  // 当这个图形未被选中时才会添加进多选
                selList.push_back(*it);
                (*it)->selected = true;
                addToCtrlPtList((*it)->ctrlPtList);
                break;
            } else {  // 已被选中时再次选中就是取消
                removeFromSelList(*it);
                removeFromCtrlPtList((*it)->ctrlPtList);
            }
        }
    }
}

void Canva::initContextMenu() {
    normalMenu = new QMenu(this);
    figMenu = normalMenu->addMenu("添加");
    figMenu->addAction("长方形", this,
                       [this]() { this->onCreate(Figure::Rect); });
    figMenu->addAction("圆形/椭圆", this,
                       [this]() { this->onCreate(Figure::Ell); });
    figMenu->addAction("线条", this,
                       [this]() { this->onCreate(Figure::Line); });
    normalMenu->addAction("粘贴", this, &Canva::onPaste);
    normalMenu->addAction("撤销", this, [this]() { cmdStack->undoCommand(); });
    normalMenu->addAction("重做", this, [this]() { cmdStack->redoCommand(); });
    normalMenu->addAction("恢复默认缩放比例", this, [this]() {
        cmdStack->executeCommand(
            std::make_shared<RecoverCmd>(this, viewTf->scale));
    });

    selMenu = new QMenu(this);
    selMenu->addAction("复制", this, &Canva::onCopy);
    selMenu->addAction("删除", this, [this]() {
        if (!selList.empty()) {
            cmdStack->executeCommand(
                std::make_shared<DelFigCmd>(this, selList));
        }
    });
    selMenu->addAction("组合", this, [this]() {
        cmdStack->executeCommand(std::make_shared<CpsFigCmd>(this, selList));
    });
    selMenu->addAction("拆散", this, &Canva::onCancelCps);
    qDebug() << 1;

    // 设置边框颜色菜单
    penColorMenu = selMenu->addMenu("设置边框颜色");
    penColorMenu->addAction("无边框", this, [this]() {
        if (!selList.empty()) {
            QPen newPen(Qt::NoPen);
            cmdStack->executeCommand(std::make_shared<SetPenCmd>(
                selList[0],
                selList[0]->pen,  // 使用当前图形的pen
                newPen));
        }
    });

    // 添加常用颜色选项
    addColorActions(penColorMenu, [this](const QColor &color) {
        if (!selList.empty()) {
            QPen newPen = selList[0]->pen;  // 获取当前图形的pen
            newPen.setColor(color);
            cmdStack->executeCommand(
                std::make_shared<SetPenCmd>(selList[0],
                                            selList[0]->pen,  // 记录原始pen
                                            newPen));
        }
    });

    // 添加"更多颜色..."选项
    penColorMenu->addSeparator();
    penColorMenu->addAction("更多颜色...", this, [this]() {
        if (!selList.empty()) {
            QColor color = QColorDialog::getColor(selList[0]->pen.color(), this,
                                                  "选择边框颜色");
            if (color.isValid()) {
                QPen newPen = selList[0]->pen;
                newPen.setColor(color);
                cmdStack->executeCommand(std::make_shared<SetPenCmd>(
                    selList[0], selList[0]->pen, newPen));
            }
        }
    });

    // 设置填充颜色菜单
    brushColorMenu = selMenu->addMenu("设置填充颜色");

    // 添加常用颜色选项
    addColorActions(brushColorMenu, [this](const QColor &color) {
        if (!selList.empty()) {
            QBrush newBrush = selList[0]->brush;  // 获取当前图形的brush
            newBrush.setColor(color);
            cmdStack->executeCommand(std::make_shared<SetBrushCmd>(
                selList[0],
                selList[0]->brush,  // 记录原始brush
                newBrush));
        }
    });

    // 添加"更多颜色..."选项
    brushColorMenu->addSeparator();
    brushColorMenu->addAction("更多颜色...", this, [this]() {
        if (!selList.empty()) {
            QColor color = QColorDialog::getColor(selList[0]->brush.color(),
                                                  this, "选择填充颜色");
            if (color.isValid()) {
                QBrush newBrush = selList[0]->brush;
                newBrush.setColor(color);
                cmdStack->executeCommand(std::make_shared<SetBrushCmd>(
                    selList[0], selList[0]->brush, newBrush));
            }
        }
    });

    // 无填充选项
    brushColorMenu->addAction("无填充", this, [this]() {
        if (!selList.empty()) {
            QBrush newBrush(Qt::NoBrush);
            cmdStack->executeCommand(std::make_shared<SetBrushCmd>(
                selList[0], selList[0]->brush, newBrush));
        }
    });
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    qDebug() << 2;
}

void Canva::updateNormalMenu() {
    normalMenu->actions()[1]->setEnabled(copyFig != nullptr);   // 粘贴
    normalMenu->actions()[2]->setEnabled(cmdStack->canUndo());  // 撤销
    normalMenu->actions()[3]->setEnabled(cmdStack->canRedo());  // 重做
}

void Canva::updateSelMenu() {
    selMenu->actions()[2]->setEnabled(selList.size() > 1);  // 组合
    selMenu->actions()[3]->setEnabled(
        selList.size() == 1 && selList[0]->getType() == Figure::Cps);  // 拆散
}

void Canva::addColorActions(QMenu *menu,
                            std::function<void(const QColor &)> callback) {
    QList<QColor> colors = {Qt::black,   Qt::white,  Qt::red,
                            Qt::green,   Qt::blue,   Qt::cyan,
                            Qt::magenta, Qt::yellow, Qt::gray};

    QStringList names = {"黑色", "白色", "红色", "绿色", "蓝色",
                         "青色", "洋红", "黄色", "灰色"};

    for (int i = 0; i < colors.size(); ++i) {
        QAction *action = new QAction(names[i], menu);
        QPixmap pixmap(16, 16);
        pixmap.fill(colors[i]);
        action->setIcon(QIcon(pixmap));

        connect(action, &QAction::triggered, this,
                [callback, color = colors[i]]() { callback(color); });

        menu->addAction(action);
    }
}

void Canva::onCopy() {
    if (selList.size() == 1) {
        copyFig = selList[0];
        copyFig->print();
    } else {
        qDebug() << selList.size();
    }
}

void Canva::onPaste() {
    if (copyFig == nullptr) {  // 没有复制对象不理会
        qDebug() << "沒有复制对象";
        return;
    }
    cmdStack->executeCommand(std::make_shared<PasteCmd>(
        this, copyFig, viewTf->VTW(lastRightViewPos)));
}

void Canva::onCancelCps() {
    if (selList.size() == 1 && selList[0]->getType() == Figure::Cps) {
        cmdStack->executeCommand(
            std::make_shared<CancelCpsCmd>(this, selList[0]));
    } else {
        qDebug() << "无效的选择";
    }
}
