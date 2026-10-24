#include "mainwindow.h"
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //Подгоняем ширину столбцов под таблицу
    ui.statsTableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );
}

MainWindow::~MainWindow()
{}

