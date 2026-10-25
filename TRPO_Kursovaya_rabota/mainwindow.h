
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

#include <QString>
#include <QMap>
#include <QtGlobal>
#include <QtCharts/QChartView>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindowClass ui;

    // Путь к каталогу, содержимое которого сейчас отображается.
    QString currentPath;

    // Общий размер всех найденных файлов в байтах.
    qint64 totalSize = 0;

    // Суммарные размеры файлов по непосредственным подкаталогам.
    QMap<QString, qint64> directorySizes;

    // Суммарные размеры файлов по категориям.
    QMap<QString, qint64> categorySizes;

    // Виджет для отображения круговой диаграммы.
    QtCharts::QChartView* chartView = nullptr;

    // Обновляет текущий каталог и список его непосредственных подкаталогов.
    void updateDirectory(const QString& path);

    // Открывает диалог выбора каталога.
    void selectDirectory();

    // Переходит в выбранный подкаталог.
    void openSelectedSubdirectory();

    // Переходит в родительский каталог.
    void navigateToParentDirectory();

    // Выполняет один обход выбранного каталога и собирает статистику.
    void analyzeCurrentDirectory();

    // Рекурсивно обрабатывает содержимое каталога.
    void scanDirectory(
        const QString& path,
        const QString& topLevelBucket,
        bool isRoot
    );

    // Определяет категорию файла по его расширению.
    QString fileCategory(const QString& extension);

    // Преобразует размер в байтах в удобный для чтения формат.
    QString formatSize(qint64 bytes);

    // Выводит в таблицу статистику выбранного режима.
    void updateStatisticsTable();

    // Обновляет круговую диаграмму для выбранного режима статистики.
    void updateChart();
};
