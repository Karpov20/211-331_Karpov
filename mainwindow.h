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
    QByteArray encryptedLogin;
    QByteArray encryptedPassword;
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
    QString masterPassword;


    void setupLoginScreen();
    void setupDataScreen();
    void setupErrorScreen();
    void setupSecondPasswordScreen();  // Метод для настройки нового экрана
    void handleSecondPasswordSubmit();  // Метод для обработки ввода второго пароля

    bool do_crypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool encrypt);
    bool decryptFile(const QByteArray &key);
    void loadDataToTable();
    void secureClear(QByteArray &data);
    void createEncryptedFile(const QByteArray &key);
    QWidget *secondPasswordScreen;  // Новый экран для ввода второго пароля
    QLineEdit *secondPasswordField;  // Поле для ввода второго пароля
    QLabel *secondPasswordErrorLabel;  // Метка для ошибок
    int currentRow;  // Текущая строка
    int currentColumn;

    // Новые методы для второго слоя шифрования
    QByteArray generateSecondaryKey(const QString &masterKey, const QString &uniqueIdentifier);
    QByteArray encryptData(const QByteArray &data, const QByteArray &key);
    QByteArray decryptData(const QByteArray &data, const QByteArray &key);

private slots:
    void handleCellDoubleClick(int row, int column);  // Слот для обработки двойного клика
    void checkPassword();
    void returnToLogin();
    void filterTable(const QString &text);
};

#endif // MAINWINDOW_H
