#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include "figure.h"  //;

class Canva;
class Command {
public:
    enum CmdType {
        Cps,
        Add,
        Sel,
        Recover,
        Zoom,
        Paste,
        Move,
        Del,
        CancelCps,
        Pan,
        Adjust
    };
    Canva* canva;
    Command();
    ~Command();
    virtual void execute() = 0;
    virtual void redo() = 0;
    virtual void undo() = 0;
    CmdType type;

protected:
    // bool executed = false;
};

class ComposeCmd : public Command {
private:
    std::vector<std::shared_ptr<Command>> list;

public:
    ComposeCmd();
    ~ComposeCmd();
    void addCmd(std::shared_ptr<Command> cmd);
    void execute();
    void undo();
    void redo();
};

class CmdStack {
private:
    std::vector<std::shared_ptr<Command>> stack;
    size_t curPt;

public:
    CmdStack() : curPt(0) {};
    void executeCommand(std::shared_ptr<Command> cmd);
    void undoCommand();
    void redoCommand();
    bool canUndo();
    bool canRedo();
};

class AddFigCmd : public Command {
public:
    AddFigCmd(Canva* c, Figure::FigType type, QPointF p);
    ~AddFigCmd();
    void execute();
    void undo();
    void redo();

private:
    std::shared_ptr<Figure> fig;
};

class DelFigCmd : public Command {
public:
    DelFigCmd(Canva* c, std::vector<Figure*> f);
    void execute();
    void undo();
    void redo();

private:
    std::vector<Figure*> figList;
};

// 将视图缩放比例恢复
class RecoverCmd : public Command {
public:
    RecoverCmd(Canva* c, double scale);
    ~RecoverCmd();
    void execute();
    void undo();
    void redo();

private:
    double preScale;
};

class ZoomCmd : public Command {
public:
    ZoomCmd(Canva* c, double ps, double ns);
    ~ZoomCmd();
    void execute();
    void undo();
    void redo();

private:
    double pastScale;
    double newScale;
};

class PasteCmd : public Command {
public:
    PasteCmd(Canva* c, Figure* f, QPointF p);
    ~PasteCmd();
    void execute();
    void undo();
    void redo();

private:
    Figure* fig;
};

class SelCmd : public Command {  // 不只选中，取消选中也采用该命令
public:
    SelCmd(Canva* c, std::vector<Figure*> preL, std::vector<Figure*> newL);
    void execute();
    void undo();
    void redo();

private:
    std::vector<Figure*> preSelList;
    std::vector<Figure*> newSelList;
};

class MoveCmd : public Command {
public:
    MoveCmd(Canva* c, std::vector<Figure*> selL, QPointF preP, QPointF newP);
    void execute();
    void undo();
    void redo();

private:
    std::vector<Figure*> selList;
    QPointF prePoint;
    QPointF newPoint;
};

class PanCmd : public Command {
public:
    PanCmd(Canva* c, QPointF delta);
    void execute();
    void undo();
    void redo();

private:
    QPointF delta;
};

class CpsFigCmd : public Command {
public:
    CpsFigCmd(Canva* c, std::vector<Figure*> f);
    void execute();
    void undo();
    void redo();

private:
    std::shared_ptr<Figure> cpsFig;
    std::vector<Figure*> figList;
};

class CancelCpsCmd : public Command {
public:
    CancelCpsCmd(Canva* c, Figure* fig);
    void execute();
    void undo();
    void redo();

private:
    CpsFig* fig;
};

class AdjustCmd : public Command {
public:
    AdjustCmd(Canva* c, std::vector<QPointF> pre, Figure* f);
    void execute();
    void undo();
    void redo();

private:
    std::vector<QPointF> preList;
    std::vector<QPointF> newList;
    Figure* fig;
};

#endif  // COMMAND_H
