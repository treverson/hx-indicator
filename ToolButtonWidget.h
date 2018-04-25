#ifndef TOOLBUTTONWIDGET_H
#define TOOLBUTTONWIDGET_H

#include <QWidget>

namespace Ui {
class ToolButtonWidget;
}

class ToolButtonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ToolButtonWidget(QWidget *parent = 0);
    ~ToolButtonWidget();
signals:
    void clicked(bool checked=false);
public:
    void setText(const QString &text);
    void setInitGray(bool gray = false);
private:
    void InitWidget();
    void InitStyle();
private:
    Ui::ToolButtonWidget *ui;
};

#endif // TOOLBUTTONWIDGET_H
