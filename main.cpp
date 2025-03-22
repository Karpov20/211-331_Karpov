#include <QApplication>
#include <QMessageBox>
#include <QCryptographicHash> // Для хеширования
#include <windows.h>         // Для Windows API
#include <psapi.h>           // Для GetModuleInformation
#include <cstdint>           // Для uint64_t
#include "mainwindow.h"      // Заголовочный файл вашего главного окна

// Функция для проверки целостности
void CheckIntegrity() {
    // 1) Определить, где в памяти начало сегмента .text
    uint64_t imageBase = (uint64_t)GetModuleHandle(NULL);
    uint64_t baseOfCode = 0x1000;
    uint64_t textBase = imageBase + baseOfCode;

    // 2) Определить, какой он длины
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(imageBase);
    PIMAGE_NT_HEADERS peHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
        imageBase + dosHeader->e_lfanew);
    uint64_t textSize = peHeader->OptionalHeader.SizeOfCode;

    // 3) От бинарного блока в диапазоне textBase...(textBase+textSize) посчитать хеш
    QByteArray textSegmentContents = QByteArray((char*)textBase, textSize);
    QByteArray calculatedTextHash = QCryptographicHash::hash(
        textSegmentContents, QCryptographicHash::Sha256);
    QByteArray calculatedTextHashBase64 = calculatedTextHash.toBase64();

    // 4) Сравнить полученный хеш с заранее рассчитанным
    const QByteArray referenceTextHashBase64 =
        QByteArray("DlGyIf4PUkvO1pJdmbgOgujtbmhKlj6iJmq3P17mIok=");

    qDebug() << "textBase = " << Qt::hex << textBase;
    qDebug() << "textSize = " << textSize;
    qDebug() << "textSegmentContents = " << Qt::hex << textSegmentContents.first(100);
    qDebug() << "calculatedTextHashBase64 = " << calculatedTextHashBase64;

    bool checkresult = (calculatedTextHashBase64 == referenceTextHashBase64);
    qDebug() << "checkresult = " << checkresult;

    // 5) Реакция на расхождение хешей
    if (!checkresult) {
        QMessageBox::critical(nullptr, "Ошибка целостности", "Обнаружена модификация приложения!");
        exit(1); // Завершаем приложение
    }
}


int main(int argc, char *argv[]) {
    // Создание объекта QApplication
    QApplication a(argc, argv);

    // Проверка на наличие отладчика
   /* if (IsDebuggerPresent()) {
        qDebug() << "Debugger detected! Exiting...";
        QMessageBox::critical(nullptr, "Debugger Detected", "A debugger has been detected. The application will now exit.");
        return 1;
    }*/

    qDebug() << "No debugger detected. Starting application...";

    // Проверка целостности приложения
    CheckIntegrity();

    // Создание и отображение главного окна
    MainWindow w;
    w.show();

    return a.exec();
}



