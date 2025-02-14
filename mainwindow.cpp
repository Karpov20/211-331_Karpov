#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>

#define MASTER_PASSWORD "1234"  // Пример мастер-пароля

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginScreen();
    setupDataScreen();
    setupErrorScreen();

    stackedWidget->addWidget(loginScreen);
    stackedWidget->addWidget(dataScreen);
    stackedWidget->addWidget(errorScreen);

    stackedWidget->setCurrentWidget(loginScreen);
}

MainWindow::~MainWindow() {}

// --- Экран входа ---
void MainWindow::setupLoginScreen() {
    loginScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(loginScreen);

    QLabel *titleLabel = new QLabel("Введите мастер-пароль", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px;");

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Введите мастер-пароль");
    passwordField->setEchoMode(QLineEdit::Password);

    QPushButton *loginButton = new QPushButton("Войти", this);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::checkPassword);

    // Добавляем верхний отступ, чтобы текст не был прижат к верху окна
    layout->addStretch(1);

    layout->addWidget(titleLabel, 0, Qt::AlignCenter);  // Центрируем надпись
    layout->addWidget(passwordField, 0, Qt::AlignCenter);
    layout->addWidget(loginButton, 0, Qt::AlignCenter);

    // Добавляем нижний отступ, чтобы поле ввода не прилипало к низу
    layout->addStretch(2);

}

// --- Экран ошибки ---
void MainWindow::setupErrorScreen() {
    errorScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(errorScreen);

    QLabel *errorLabel = new QLabel("Ошибка: неправильный пароль", this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setStyleSheet("color: red; font-size: 18px;");

    QPushButton *backButton = new QPushButton("Назад", this);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::returnToLogin);

    layout->addWidget(errorLabel);
    layout->addWidget(backButton);

    errorScreen->setLayout(layout);
}

// --- Экран таблицы с учётными данными ---
void MainWindow::setupDataScreen() {
    dataScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(dataScreen);

    QLabel *titleLabel = new QLabel("Хранилище учётных данных", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(3);
    dataTable->setHorizontalHeaderLabels({"Сайт", "Логин", "Пароль"});
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    loadCredentials();

    layout->addWidget(titleLabel);
    layout->addWidget(dataTable);

    dataScreen->setLayout(layout);
}

// --- Проверка пароля и смена экрана ---
void MainWindow::checkPassword() {
    if (passwordField->text() == MASTER_PASSWORD) {
        stackedWidget->setCurrentWidget(dataScreen);
    } else {
        stackedWidget->setCurrentWidget(errorScreen);
    }
}

// --- Кнопка "Назад" на экране ошибки ---
void MainWindow::returnToLogin() {
    stackedWidget->setCurrentWidget(loginScreen);
}

QString maskString(const QString& input) {
    return QString(input.length(), QChar(0x2022));  // Маскируем строку символами '•'
}

// --- Загрузка учётных данных из файла ---
void MainWindow::loadCredentials() {
    QFile file("credentials.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');

        if (parts.size() == 3) {
            int row = dataTable->rowCount();
            dataTable->insertRow(row);

            dataTable->setItem(row, 0, new QTableWidgetItem(parts[0]));  // Сайт
            dataTable->setItem(row, 1, new QTableWidgetItem(maskString(parts[1])));  // Логин (маскирован)
            dataTable->setItem(row, 2, new QTableWidgetItem(maskString(parts[2])));  // Пароль (маскирован)
        }
    }
    file.close();
}
