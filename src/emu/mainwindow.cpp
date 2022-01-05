#include "mainwindow.h"

#include "../gui.hpp"
#include "./ui_mainwindow.h"

#include <QTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    user_interface.render();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_ccwButton_clicked() {
    send_event(event_type::encoder_ccw);
    process_events();
}

void MainWindow::on_cwButton_clicked() {
    send_event(event_type::encoder_cw);
    process_events();
}

void MainWindow::on_pressButton_clicked() {
    send_event(event_type::encoder_press);
    process_events();
}

void MainWindow::process_events() {
    message msg{};
    while (pop_message(msg)) {
        switch (msg.type) {
        case message_type::event:
            process_event(msg.e);
            break;
        case message_type::command:
            process_command(msg.c);
            break;
        }
    }
}

void MainWindow::on_renderButton_clicked() {
    user_interface.render();
    ui->oledScreen->repaint();
}

void MainWindow::process_command(command cmd) {
    switch (cmd.type) {
    case command_type::logo:
        user_interface.push_screen(&user_interface.logo_screen_);
        break;
    case command_type::request_render_screen:
    case command_type::render_screen:
        user_interface.render();
        ui->oledScreen->repaint();
        break;
    default:
        break;
    }
}

void MainWindow::process_event(::event event1) {
    user_interface.process_event(event1);
}

