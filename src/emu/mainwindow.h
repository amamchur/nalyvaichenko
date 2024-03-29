#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <src/message.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_ccwButton_clicked();

    void on_cwButton_clicked();

    void on_pressButton_clicked();

    void on_renderButton_clicked();

private:
    Ui::MainWindow *ui;
    static void process_event(::event event1);
    void process_command(::command cmd);
    void process_events();
};
#endif // MAINWINDOW_H
