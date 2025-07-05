#include "command.h"

#include "canva.h"

Command::Command() {}

Command::~Command() {}

void CmdStack::executeCommand(std::shared_ptr<Command> cmd) {
    if (curPt < stack.size()) {
        stack.erase(stack.begin() + curPt,
                    stack.end());  // 删去所有撤销命令，得到新分支
    }
    cmd->execute();
    stack.push_back(cmd);
    curPt++;
    switch (cmd->type) {
        case Command::Add:
            qDebug() << "执行添加命令";
            break;
        case Command::Recover:
            qDebug() << "执行恢复命令";
            break;
        case Command::Paste:
            qDebug() << "执行粘贴命令";
            break;
        case Command::Sel:
            qDebug() << "执行选择命令";
            break;
        case Command::Zoom:
            qDebug() << "执行视图缩放命令";
            break;
        case Command::Cps:
            qDebug() << "执行组合命令";
            break;
        case Command::Move:
            qDebug() << "执行移动图形命令";
            break;
        case Command::Adjust:
            qDebug() << "执行调整图形命令";
            break;
    }
}

void CmdStack::undoCommand() {
    qDebug() << "执行撤销命令";
    if (curPt == 0) {
        qDebug() << "没有可以撤销的命令";
        return;
    }
    stack[curPt - 1]->undo();
    curPt--;
    qDebug() << "撤销完成";
}

void CmdStack::redoCommand() {
    qDebug() << "执行重做命令";
    if (curPt == stack.size()) {
        qDebug() << "没有可以重做的命令";
        return;
    }
    curPt++;
    stack[curPt - 1]->redo();
    qDebug() << "重做完成";
}

bool CmdStack::canUndo() { return curPt > 0; }

bool CmdStack::canRedo() { return curPt < stack.size(); }

AddFigCmd::AddFigCmd(Canva* c, Figure::FigType type, QPointF p) {
    canva = c;
    this->type = Add;
    switch (type) {
        case Figure::Rect:
            fig = std::make_shared<RectFig>(p, 50, 50);
            fig->setPen(canva->dftPen);
            fig->setBrush(canva->dftBrush);
            fig->setVTf(canva->viewTf);
            break;
        case Figure::Ell:
            fig = std::make_shared<EllFig>(p, 50, 50);
            fig->setPen(canva->dftPen);
            fig->setBrush(canva->dftBrush);
            fig->setVTf(canva->viewTf);
            break;
        case Figure::Line:
            fig = std::make_shared<Line>(p, p + QPointF{50, 0});
            fig->setPen(canva->dftPen);
            fig->setBrush(canva->dftBrush);
            fig->setVTf(canva->viewTf);
            break;
    }
}

AddFigCmd::~AddFigCmd() {}

void AddFigCmd::execute() { canva->addToList(fig.get()); }

void AddFigCmd::undo() {
    canva->removeFromList(fig.get());
    canva->ctrlPtList.clear();
}

void AddFigCmd::redo() { canva->addToList(fig.get()); }

DelFigCmd::DelFigCmd(Canva* c, std::vector<Figure*> f) {
    type = Del;
    canva = c;
    figList = f;
}

void DelFigCmd::execute() {
    for (auto it : figList) {
        canva->removeFromList(it);
    }
}

void DelFigCmd::undo() {
    for (auto it = figList.rbegin(); it != figList.rend(); it++) {
        canva->addToList(*it);
    }
}

void DelFigCmd::redo() { execute(); }

RecoverCmd::RecoverCmd(Canva* c, double scale) : preScale(scale) {
    canva = c;
    type = Recover;
}

RecoverCmd::~RecoverCmd() {}

void RecoverCmd::execute() { canva->viewTf->scale = 1.0; }

void RecoverCmd::undo() { canva->viewTf->scale = preScale; }

void RecoverCmd::redo() { canva->viewTf->scale = 1.0; }

ZoomCmd::ZoomCmd(Canva* c, double ps, double ns) : pastScale(ps), newScale(ns) {
    canva = c;
    type = Zoom;
}

ZoomCmd::~ZoomCmd() {}

void ZoomCmd::execute() {}  // 为了实时缩放屏幕比例，不做处理,只进行记录

void ZoomCmd::undo() { canva->viewTf->scale = pastScale; }

void ZoomCmd::redo() { canva->viewTf->scale = newScale; }

ComposeCmd::ComposeCmd() { type = Cps; }

ComposeCmd::~ComposeCmd() {}

void ComposeCmd::addCmd(std::shared_ptr<Command> cmd) { list.push_back(cmd); }

void ComposeCmd::execute() {
    for (auto it : list) {
        it->execute();
    }
}

void ComposeCmd::undo() {
    for (auto it = list.rbegin(); it != list.rend(); it++) {
        (*it)->undo();
    }
}

void ComposeCmd::redo() {
    for (auto it : list) {
        it->redo();
    }
}

PasteCmd::PasteCmd(Canva* c, Figure* f, QPointF p) {
    type = Paste;
    canva = c;
    std::shared_ptr<Figure> tempfig(f->clone());
    fig = tempfig;
    fig->moveTo(p);
    fig->print();
}

PasteCmd::~PasteCmd() {
    if (fig->getType() == Figure::Cps) {
        CpsFig* cpsfig = static_cast<CpsFig*>(fig.get());
        for (auto& it : cpsfig->List()) {
            delete it;
        }
    }
}

void PasteCmd::execute() {
    canva->addToList(fig.get());
    if (fig->getType() == Figure::CopyCompose) {
        // 复制对象要进行解包操作;
    }
}

void PasteCmd::undo() {
    if (fig->getType() == Figure::CopyCompose) {
    }
    canva->removeFromList(fig.get());
}

void PasteCmd::redo() { execute(); }

SelCmd::SelCmd(Canva* c, std::vector<Figure*> preL, std::vector<Figure*> newL) {
    type = Sel;
    preSelList = preL;
    newSelList = newL;
    canva = c;
}

void SelCmd::execute() {}  // 并不执行操作，只进行记录

void SelCmd::undo() {
    canva->SELLIST() = preSelList;
    for (auto it : newSelList) {
        it->selected = false;
    }
    for (auto it : preSelList) {
        it->selected = true;
    }
}

void SelCmd::redo() {
    canva->SELLIST() = newSelList;
    for (auto it : newSelList) {
        it->selected = true;
    }
    for (auto it : preSelList) {
        it->selected = false;
    }
}

MoveCmd::MoveCmd(Canva* c, std::vector<Figure*> selL, QPointF preP,
                 QPointF newP) {
    canva = c;
    selList = selL;
    prePoint = preP;
    newPoint = newP;
    type = Move;
}

void MoveCmd::execute() {}  // 只记录不执行

void MoveCmd::undo() {
    for (auto it : selList) {
        it->translate((prePoint.x() - newPoint.x()),
                      (prePoint.y()) - newPoint.y());
        qDebug() << it->boundingRect();
        for (auto i : it->getCtrlPoint()) {
            qDebug() << i;
        }
    }
}

void MoveCmd::redo() {
    for (auto it : selList) {
        it->translate((newPoint.x() - prePoint.x()),
                      (newPoint.y()) - prePoint.y());
    }
}

PanCmd::PanCmd(Canva* c, QPointF d) {
    canva = c;
    delta = d;
}

void PanCmd::execute() {
}  // 同样的只记录不执行同时注意视图的移动与鼠标的移动是反向的

void PanCmd::undo() { canva->viewTf->translate(delta); }

void PanCmd::redo() { canva->viewTf->translate(-delta); }

CpsFigCmd::CpsFigCmd(Canva* c, std::vector<Figure*> f) {
    canva = c;
    cpsFig = std::make_shared<CpsFig>(f);
    figList = f;
    cpsFig->setVTf(canva->viewTf);
    type = Cps;
}

void CpsFigCmd::execute() {
    for (auto it : figList) {
        canva->removeFromList(it);
        canva->removeFromSelList(it);
        it->inCps = true;
    }
    canva->ctrlPtList.clear();
    canva->addToList(cpsFig.get());
    canva->addToSelList(cpsFig.get());
}

void CpsFigCmd::undo() {
    canva->removeFromList(cpsFig.get());
    canva->removeFromSelList(cpsFig.get());
    canva->ctrlPtList.clear();
    for (auto it : figList) {
        canva->addToList(it);
        canva->addToSelList(it);
        it->inCps = false;
    }
}

void CpsFigCmd::redo() { execute(); }

CancelCpsCmd::CancelCpsCmd(Canva* c, Figure* f) {
    canva = c;
    if (f->getType() != Figure::Cps) {
        qDebug() << "不是组合图形";
        return;
    }
    fig = dynamic_cast<CpsFig*>(f);
}

void CancelCpsCmd::execute() {
    canva->removeFromList(fig);
    canva->removeFromSelList(fig);
    canva->ctrlPtList.clear();
    for (auto it : fig->List()) {
        canva->addToList(it);
        it->inCps = false;
        // canva->addToSelList(it);
    }
}

void CancelCpsCmd::undo() {
    canva->addToList(fig);
    canva->addToSelList(fig);
    canva->ctrlPtList.clear();
    for (auto it : fig->List()) {
        canva->removeFromList(it);
        it->inCps = true;
        // canva->removeFromSelList(it);
    }
}

void CancelCpsCmd::redo() { execute(); }

AdjustCmd::AdjustCmd(Canva* c, std::vector<QPointF> pre, Figure* f) {
    canva = c;
    if (f == nullptr) {
        qDebug() << "无效的指针";
        return;
    }
    fig = f;
    preList = pre;
    newList = f->getCtrlPoint();
    qDebug() << pre.size() << newList.size();
    type = Adjust;
}

void AdjustCmd::execute() {}  // 只执行，不记录

void AdjustCmd::undo() { fig->adjust(preList); }

void AdjustCmd::redo() { fig->adjust(newList); }

SetPenCmd::SetPenCmd(Figure* f, QPen pp, QPen np) {
    fig = f;
    prePen = pp;
    newPen = np;
}

void SetPenCmd::execute() { fig->setPen(newPen); }

void SetPenCmd::undo() { fig->setPen(prePen); }

void SetPenCmd::redo() { execute(); }

SetBrushCmd::SetBrushCmd(Figure* f, QBrush pb, QBrush nb) {
    fig = f;
    preBrush = pb;
    newBrush = nb;
}

void SetBrushCmd::execute() { fig->setBrush(newBrush); }

void SetBrushCmd::undo() { fig->setBrush(preBrush); }

void SetBrushCmd::redo() { execute(); }
