#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <QString>

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

    // Обновляет текущий каталог и список его непосредственных подкаталогов.
    void updateDirectory(const QString& path);

    // Открывает диалог выбора каталога.
    void selectDirectory();

    // Переходит в выбранный подкаталог.
    void openSelectedSubdirectory();

    // Переходит в родительский каталог.
    void navigateToParentDirectory();
};
