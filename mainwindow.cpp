#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <openssl/rand.h>
#include <QClipboard>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    qDebug() << "Current working directory:" << QDir::currentPath();

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginScreen();
    setupDataScreen();
    setupErrorScreen();

    stackedWidget->addWidget(loginScreen);
    stackedWidget->addWidget(dataScreen);
    stackedWidget->addWidget(errorScreen);

    connect(dataTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::handleCellDoubleClick);
}

MainWindow::~MainWindow() {
    // Очищаем чувствительные данные
    for (auto& cred : memoryStorage) {
        cred.login.fill('*');
        cred.password.fill('*');
    }
    memoryStorage.clear();
}

void MainWindow::setupLoginScreen() {
    loginScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(loginScreen);

    QLabel *titleLabel = new QLabel("Enter Master PIN", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px;");

    // Добавляем QLabel для отображения ошибок
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->hide(); // Скрываем по умолчанию

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Enter PIN");
    passwordField->setEchoMode(QLineEdit::Password);

    QPushButton *loginButton = new QPushButton("Login", this);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::checkPassword);

    layout->addStretch(1);
    layout->addWidget(titleLabel);
    // Добавляем errorLabel перед passwordField
    layout->addWidget(errorLabel);
    layout->addWidget(passwordField);
    layout->addWidget(loginButton);
    layout->addStretch(1);

    loginScreen->setLayout(layout);
}


void MainWindow::setupDataScreen() {
    dataScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(dataScreen);

    QLabel *titleLabel = new QLabel("Credentials Storage", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    // Добавляем поле фильтрации
    filterField = new QLineEdit(this);
    filterField->setPlaceholderText("Filter by site...");
    connect(filterField, &QLineEdit::textChanged, this, &MainWindow::filterTable);

    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(3);
    dataTable->setHorizontalHeaderLabels({"Website", "Login", "Password"});
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Добавляем элементы в layout
    layout->addWidget(titleLabel);
    layout->addWidget(filterField);  // Поле фильтрации
    layout->addWidget(dataTable);

    dataScreen->setLayout(layout);
}

void MainWindow::filterTable(const QString &text) {
    for(int i = 0; i < dataTable->rowCount(); ++i) {
        QTableWidgetItem *siteItem = dataTable->item(i, 0);
        bool match = siteItem->text().contains(text, Qt::CaseInsensitive);
        dataTable->setRowHidden(i, !match);
    }
}

void MainWindow::setupErrorScreen() {
    errorScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(errorScreen);

    errorLabelErrorScreen = new QLabel("Invalid PIN!", this);
    errorLabelErrorScreen->setStyleSheet("color: red; font-size: 18px;");
    errorLabelErrorScreen->setAlignment(Qt::AlignCenter);

    QPushButton *backButton = new QPushButton("Back", this);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::returnToLogin);

    layout->addWidget(errorLabelErrorScreen);
    layout->addWidget(backButton);
    errorScreen->setLayout(layout);
}


void MainWindow::checkPassword() {
    QString pin = passwordField->text();
    passwordField->clear();

    if (pin.isEmpty()) {
        errorLabel->setText("PIN cannot be empty!");
        errorLabel->show();
        return;
    } else {
        errorLabel->hide(); // Скрываем сообщение об ошибке, если поле PIN не пустое
    }

    // Генерация ключа
    QByteArray key = QCryptographicHash::hash(
        pin.toUtf8(),
        QCryptographicHash::Sha3_256
        );
    qDebug() << "Generated key:" << key.toHex();

    // Если файла нет - создаем
    QFile file("credentialsAES.json");
    if (!file.exists()) {
        qDebug() << "File not found. Creating new...";
        createEncryptedFile(key);
    }

    // Дешифровка
    if (decryptFile(key)) {
        loadDataToTable();
        stackedWidget->setCurrentWidget(dataScreen);
        errorLabel->hide(); // Скрываем метку ошибки при успешной дешифровке
    } else {
        errorLabel->setText("Invalid PIN or corrupted file!");
        errorLabel->show();
        stackedWidget->setCurrentWidget(errorScreen); // Альтернатива - переключение на errorScreen
    }

    key.fill(0);
}

void MainWindow::createEncryptedFile(const QByteArray &key) {
    QFile jsonFile("credentials.json"); // Используем credentials.json
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open credentials.json:" << jsonFile.errorString();
        errorLabelErrorScreen->setText("Default credentials file not found!");
        stackedWidget->setCurrentWidget(errorScreen);
        return;
    }

    QByteArray jsonData = jsonFile.readAll(); // Считываем JSON данные
    jsonFile.close();

    // Шифруем в hex виде
    QByteArray encryptedData;
    if (!do_crypt(jsonData, encryptedData, key, true)) {
        qDebug() << "Encryption failed!";
        errorLabelErrorScreen->setText("Encryption Failed!");
        stackedWidget->setCurrentWidget(errorScreen);
        return;
    }

    // Преобразуем зашифрованные данные в hex-строку
    QByteArray hexEncryptedData = encryptedData.toHex();

    QFile encFile("credentialsAES.json");
    if (!encFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to create enc file:" << encFile.errorString();
        errorLabelErrorScreen->setText("Failed to create encrypted file!");
        stackedWidget->setCurrentWidget(errorScreen);
        return;
    }

    encFile.write(hexEncryptedData);
    encFile.close();
    qDebug() << "New encrypted file created successfully";
}


bool MainWindow::decryptFile(const QByteArray &key) {
    QFile file("credentialsAES.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Open error:" << file.errorString();
        errorLabelErrorScreen->setText("Failed to open encrypted file!");
        stackedWidget->setCurrentWidget(errorScreen);
        return false;
    }

    QByteArray hexEncryptedData = file.readAll();
    file.close();

    // Преобразуем hex-строку обратно в зашифрованные данные
    QByteArray encryptedData = QByteArray::fromHex(hexEncryptedData);

    QByteArray decryptedData;
    if (!do_crypt(encryptedData, decryptedData, key, false)) {
        qDebug() << "Decryption failed!";
        errorLabelErrorScreen->setText("Decryption failed!");
        stackedWidget->setCurrentWidget(errorScreen);
        decryptedData.fill(0);
        return false;
    }

    // Парсинг JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedData);
    if (!jsonDoc.isObject()) {
        qDebug() << "Invalid JSON format";
        errorLabelErrorScreen->setText("Invalid JSON format in encrypted file!");
        stackedWidget->setCurrentWidget(errorScreen);
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray credentialsArray = jsonObj["credentials"].toArray();
    memoryStorage.clear();

    for (const QJsonValue &value : credentialsArray) {
        if (value.isObject()) {
            QJsonObject credential = value.toObject();
            if (credential.contains("hostname") && credential.contains("loginpassword")) {
                QString hostname = credential["hostname"].toString();
                QJsonObject loginpassword = credential["loginpassword"].toObject();

                if (loginpassword.contains("login") && loginpassword.contains("password")) {
                    QString login = loginpassword["login"].toString();
                    QString password = loginpassword["password"].toString();
                    memoryStorage.append({hostname, login, password});
                } else {
                    qDebug() << "Missing login or password in loginpassword";
                }
            } else {
                qDebug() << "Missing hostname or loginpassword";
            }
        } else {
            qDebug() << "Value is not an object";
        }
    }

    decryptedData.fill(0); // Очищаем расшифрованные данные
    return true;
}



bool MainWindow::do_crypt(const QByteArray &in, QByteArray &out, const QByteArray &key, bool encrypt) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "Failed to create EVP context";
        return false;
    }

    unsigned char iv[EVP_MAX_IV_LENGTH];
    if (encrypt) {
        if (RAND_bytes(iv, EVP_MAX_IV_LENGTH) != 1) {
            qDebug() << "IV generation failed";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*>(key.data()), iv)) {
            qDebug() << "EncryptInit failed";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    } else {
        if (in.size() < EVP_MAX_IV_LENGTH) {
            qDebug() << "Invalid encrypted data size. Data size:" << in.size() << ", IV size required:" << EVP_MAX_IV_LENGTH;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        memcpy(iv, in.constData(), EVP_MAX_IV_LENGTH);
        if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*>(key.data()), iv)) {
            qDebug() << "DecryptInit failed";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    }

    // Размер буфера для выходных данных
    out.resize(in.size() + EVP_MAX_BLOCK_LENGTH);
    int out_len = 0;

    // Обработка данных по частям
    const int chunkSize = 1024; // Размер блока для обработки
    int processed = 0;

    while (processed < in.size()) {
        int bytesToProcess = qMin(chunkSize, in.size() - processed);
        int tmp_len = 0;

        if (encrypt) {
            if (!EVP_EncryptUpdate(ctx,
                                   reinterpret_cast<unsigned char*>(out.data()) + out_len, &tmp_len,
                                   reinterpret_cast<const unsigned char*>(in.constData() + processed), bytesToProcess)) {
                qDebug() << "EncryptUpdate failed";
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
        } else {
            // Для дешифровки пропускаем IV (первые EVP_MAX_IV_LENGTH байт)
            int offset = (processed == 0) ? EVP_MAX_IV_LENGTH : 0;
            if (!EVP_DecryptUpdate(ctx,
                                   reinterpret_cast<unsigned char*>(out.data()) + out_len, &tmp_len,
                                   reinterpret_cast<const unsigned char*>(in.constData() + processed + offset),
                                   bytesToProcess - offset)) {
                qDebug() << "DecryptUpdate failed";
                EVP_CIPHER_CTX_free(ctx);
                return false;
            }
        }

        out_len += tmp_len;
        processed += bytesToProcess;
    }

    // Финализация
    int final_len = 0;
    if (encrypt) {
        if (!EVP_EncryptFinal_ex(ctx,
                                 reinterpret_cast<unsigned char*>(out.data()) + out_len, &final_len)) {
            qDebug() << "EncryptFinal failed";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    } else {
        if (!EVP_DecryptFinal_ex(ctx,
                                 reinterpret_cast<unsigned char*>(out.data()) + out_len, &final_len)) {
            qDebug() << "DecryptFinal failed";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
    }

    out.resize(out_len + final_len);

    // Добавляем IV в начало при шифровании
    if (encrypt) {
        out.prepend(reinterpret_cast<const char*>(iv), EVP_MAX_IV_LENGTH);
    }

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void MainWindow::loadDataToTable() {
    dataTable->setRowCount(0);

    for (const CredentialEntry& entry : memoryStorage) {
        int row = dataTable->rowCount();
        dataTable->insertRow(row);

        dataTable->setItem(row, 0, new QTableWidgetItem(entry.hostname));
        dataTable->setItem(row, 1, new QTableWidgetItem("********"));
        dataTable->setItem(row, 2, new QTableWidgetItem("********"));
    }
}

void MainWindow::handleCellDoubleClick(int row, int column) {
    if (row < 0 || row >= memoryStorage.size()) {
        errorLabelErrorScreen->setText("Invalid cell selected!");
        stackedWidget->setCurrentWidget(errorScreen);
        return;
    }

    const CredentialEntry& entry = memoryStorage[row];
    QString textToCopy;
    QString message;

    if (column == 1) { // Копируем логин
        textToCopy = entry.login;
        message = "Login copied to clipboard!"; // Сообщение для логина
    } else if (column == 2) { // Копируем пароль
        textToCopy = entry.password;
        message = "Password copied to clipboard!"; // Сообщение для пароля
    } else {
        return; // Не копируем, если кликнули не на логин или пароль
    }

    // Копируем в буфер обмена
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textToCopy);

    // Уведомляем пользователя
    errorLabelErrorScreen->setText(message); // Используем подготовленное сообщение
    stackedWidget->setCurrentWidget(errorScreen);
}

void MainWindow::returnToLogin() {
    memoryStorage.clear();
    dataTable->setRowCount(0);
    filterField->clear(); // Очищаем поле фильтра
    stackedWidget->setCurrentWidget(loginScreen);
}
