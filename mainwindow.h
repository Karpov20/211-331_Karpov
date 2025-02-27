#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QVector>
#include <QSortFilterProxyModel>
#include <openssl/evp.h>

struct CredentialEntry {
    QString hostname;
    QString login;
    QString password;
};


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    QWidget *loginScreen, *dataScreen, *errorScreen;
    QLineEdit *passwordField;
    QLineEdit *filterField;
    QLabel *errorLabel;
    QTableWidget *dataTable;
    QVector<CredentialEntry> memoryStorage;
    QSortFilterProxyModel *proxyModel;
    QLabel *errorLabelErrorScreen;


    void setupLoginScreen();
    void setupDataScreen();
    void setupErrorScreen();

    bool do_crypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool encrypt);
    bool decryptFile(const QByteArray &key);
    void loadDataToTable();
    void secureClear(QByteArray &data);
    void createEncryptedFile(const QByteArray &key);

private slots:
    void checkPassword();
    void returnToLogin();
    void handleCellDoubleClick(int row, int column);
    void filterTable(const QString &text);
};

#endif // MAINWINDOW_H
