#include "oledscreen.h"

#include "../hardware_atmega2560.hpp"

#include <QPainter>

const uint8_t ecafe_logo[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xc0, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0xfc, 0xff, 0xff, 0xff, 0x3f, 0x0f, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x70, 0xe0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0xc0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0x7f,
    0x3f, 0x1f, 0x0f, 0x0f, 0x07, 0x07, 0x03, 0x03, 0x03, 0x83, 0x83, 0xc3, 0xe7, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf8, 0xfe, 0xff, 0xff, 0x7f, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x81, 0x81, 0xc3, 0xc3, 0xc7, 0xc7, 0xc7, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xee, 0xce, 0xce,
    0x8e, 0x8e, 0x8e, 0x0f, 0x07, 0x07, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xf8,
    0xf8, 0x7c, 0x3c, 0x1c, 0x1c, 0x18, 0xb8, 0xf0, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0x78, 0x70, 0x60, 0xe0, 0xf0, 0xf8, 0xf8, 0xf0,
    0xe0, 0x00, 0x00, 0x10, 0x18, 0x18, 0x98, 0xf8, 0xfc, 0xff, 0xff, 0xff, 0x1f, 0x1f, 0x19, 0x18, 0x18, 0x08, 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0x7c,
    0x3c, 0x38, 0xf0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0xe0, 0xf8, 0xfc, 0xfe, 0xfe, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xfe, 0xff, 0x7f, 0x1f, 0x0f, 0x03, 0x01, 0x00, 0x0c,
    0x0c, 0x0c, 0x06, 0x07, 0x03, 0xe1, 0xf0, 0xfc, 0xfe, 0xff, 0x3f, 0x0f, 0x07, 0x83, 0xc1, 0x70, 0x38, 0xfc, 0xff, 0xff, 0xff, 0x7f, 0x1f, 0x07, 0x01, 0x00,
    0x80, 0xe0, 0xf8, 0xfe, 0xff, 0x3f, 0x0f, 0xff, 0xff, 0x3f, 0x30, 0x60, 0xe0, 0xf0, 0xfc, 0xfe, 0xff, 0x7f, 0x6f, 0x67, 0x63, 0x31, 0x30, 0x18, 0x0c, 0x07,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x7f, 0xff,
    0xff, 0xff, 0xff, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0,
    0x60, 0x30, 0x18, 0x0c, 0x00, 0x00, 0x30, 0x78, 0xfc, 0x78, 0x30, 0x00, 0x00, 0x07, 0x1f, 0x3f, 0x3f, 0x7c, 0x70, 0x60, 0x60, 0x60, 0x70, 0x30, 0x18, 0x0c,
    0x0c, 0x06, 0x0f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3c, 0x1c, 0x0e, 0x07, 0x01, 0x00, 0x00, 0x1f, 0x3f, 0x7f, 0x7f, 0x23, 0x30, 0x18, 0xcc, 0xf6, 0xff, 0xff, 0xff,
    0x3f, 0x0f, 0x01, 0xe0, 0xfe, 0x7f, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0x3f, 0x7f, 0x70, 0x70, 0x60, 0x60, 0x60, 0x30, 0x30, 0x18, 0x0c, 0x07, 0x00, 0x00,
    0xc0, 0xfc, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03,
    0x07, 0x0f, 0x0f, 0x0f, 0x1e, 0x1e, 0x1c, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1c, 0x1c, 0x0c, 0x0e, 0x0e, 0x06, 0x07, 0x03, 0x01, 0x81, 0x80, 0xc0, 0xc0,
    0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0x78, 0x3c, 0x3c, 0x3c, 0x1c, 0x1e, 0x1e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x06, 0x06, 0x06, 0x06, 0xfe, 0xff, 0xff, 0xdf, 0x9f, 0x1b, 0x98, 0xf8, 0xf8,
    0x7e, 0x3f, 0x33, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf8, 0x78, 0x78, 0x7c, 0x3c, 0x3e, 0x1f, 0x1f, 0x0f,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x0f, 0x0f, 0x07, 0x07, 0x03,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

OledScreen::OledScreen(QWidget *parent)
    : QWidget(parent) {}

void OledScreen::drawPixel(QPainter &qp, int x, int y) {
    QColor color(0x00FF00);
    QBrush b(color);
    int size = 10;
    int px = x * size;
    int py = y * size;
    qp.fillRect(px, py, size, size, b);
}

void OledScreen::paintEvent(QPaintEvent *event) {
    QPainter qp(this);
    QColor color(Qt::GlobalColor::black);
    qp.setBrush(QBrush(color));
    qp.drawRect(0, 0, 1280, 640);

    QPen pen(Qt::green, 1, Qt::SolidLine);
    qp.setPen(pen);
    drawScreen(qp);
}

void OledScreen::drawScreen(QPainter &qp) {
    auto canvas = ::screen.buffer.canvas;
    int size = sizeof(ecafe_logo);
    for (int i = 0; i < size; i++) {
        auto b = canvas[i];
        int x = i % 128;
        int page = i / 128;
        int yp = page << 3;
        for (int j = 0; j < 8; j++) {
            bool v = (b & (1 << j)) != 0;
            if (v) {
                drawPixel(qp, x, yp + j);
            }
        }
    }
}
