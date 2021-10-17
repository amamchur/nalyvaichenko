#ifndef OLEDSCRENN_H
#define OLEDSCRENN_H

#include <QWidget>
#include <QPainter>

class OledScreen : public QWidget
{
    Q_OBJECT
public:
    explicit OledScreen(QWidget *parent = nullptr);
protected:
    void drawPixel(QPainter &qp, int x, int y);
    void paintEvent(QPaintEvent *event);
    void drawLogo(QPainter &qp);
signals:

};

#endif // OLEDSCRENN_H
