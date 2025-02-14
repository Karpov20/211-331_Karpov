#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    QWidget *loginScreen, *dataScreen, *errorScreen;
    QLineEdit *passwordField;
    QTableWidget *dataTable;

    void setupLoginScreen();
    void setupDataScreen();
    void setupErrorScreen();

    void loadCredentials();

private slots:
    void checkPassword();
    void returnToLogin();
};

#endif // MAINWINDOW_H
