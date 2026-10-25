#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QSignalBlocker>

//Конструктор главного окна
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Растягиваем столбцы таблицы на всю доступную ширину.
    ui.statsTableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );

    // Подключаем обработчики действий пользователя.
    connect(ui.browseButton, &QPushButton::clicked,             // Кнопка выбора каталога.
        this, &MainWindow::selectDirectory);

    connect(ui.upButton, &QPushButton::clicked,                 // Кнопка перехода в родительский каталог.
        this, &MainWindow::navigateToParentDirectory);

    connect(ui.subdirectoryComboBox,                            // Комбобокс перехода в выбранный подкаталог.
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index)                                     // Лямбда-функция для перехода в выбранный подкаталог.
        {
            if (index >= 0)
                openSelectedSubdirectory();
        });

    // При запуске показываем домашний каталог пользователя.
    updateDirectory(QDir::homePath());
}

// Деструктор главного окна
MainWindow::~MainWindow()
{
}

// Открывает диалог выбора каталога и обновляет текущий каталог.
void MainWindow::selectDirectory()
{
    // Открываем диалог, начиная с текущего каталога.
    const QString selectedPath = QFileDialog::getExistingDirectory(
        this,                                                           // Родительское окно диалога.
        QStringLiteral("Выберите каталог"),                             // Заголовок диалога.
        currentPath,                                                    // Начальный каталог. Свойство класса.
        QFileDialog::ShowDirsOnly                                       // Показываем только каталоги, без файлов.
    );

    // Пустой путь означает, что пользователь отменил выбор.
    if (selectedPath.isEmpty())
        return;

    updateDirectory(selectedPath);
}

// Обновляет текущий каталог и список его непосредственных подкаталогов.
void MainWindow::updateDirectory(const QString& path)
{
    //Изменение текущего каталога на указанный путь.
    QDir directory(path);                                               // Создаем объект QDir для указанного пути.

    if (!directory.exists())                                            // Проверяем существование каталога.
    {
        QMessageBox::warning(                                               // Выводим предупреждение, если каталог не существует.
            this,
            QStringLiteral("Ошибка"),
            QStringLiteral("Выбранный каталог не существует или недоступен.")
        );

        return;
    }

    currentPath = directory.absolutePath();                             // Обновляем свойство текущего пути на абсолютный путь выбранного каталога.

    // Отображение нового каталога как текущего
    ui.pathLineEdit->setText(currentPath);                              // Отображаем полный путь к текущему каталогу.


    // Обновление списка подкаталогов в комбобоксе.
    const QSignalBlocker blocker(ui.subdirectoryComboBox);              // Блокируем сигналы для комбобокса подкаталогов, чтобы избежать вызова слота при обновлении списка.

    ui.subdirectoryComboBox->clear();                                   // Очищаем список подкаталогов в чекербоксе перед его обновлением.

    const QFileInfoList subdirectories = directory.entryInfoList(       // Формируем список подкаталогов текущего каталога.
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable,                 // Исключаем файлы, специальные каталоги "." и ".." - сылки на этот и родительский каталоги,
        QDir::Name | QDir::IgnoreCase                                       // Недоступные для чтения каталоги. Сортируем по имени в алфавитном порядке, игноририруя регистр букв.
    );


    for (const QFileInfo& subdirectory : subdirectories)                // Заполняем комбобокс подкаталогов.
    {
        ui.subdirectoryComboBox->addItem(                               // Добавляем подкаталог в комбобокс.
            subdirectory.fileName(),                                        // В названии показываем имя.
            subdirectory.absoluteFilePath()                                 // В данных сохраняем полный путь.
        );
    }


    ui.upButton->setEnabled(directory.cdUp());                          // Переключаем кнопку перехода в родительский каталог, в соответствии с его доступностью.

    if (subdirectories.isEmpty())                                       //Соообщаем пользователю об отсутствии доступных подкаталогов.
    {
        ui.statusLineEdit->setText(
            QStringLiteral("В каталоге нет доступных подкаталогов")
        );
    }
    else                                                                // Сообщаем пользователю о количестве доступных подкаталогов.
    {
        ui.statusLineEdit->setText(
            QStringLiteral("Каталог загружен. Подкаталогов: %1")            //%1 — позиционный заполнитель. Метод arg() заменяет его переданным значением.
            .arg(subdirectories.size())
        );
    }
}

// Переходит в выбранный подкаталог.
void MainWindow::openSelectedSubdirectory()
{
    const int index = ui.subdirectoryComboBox->currentIndex();      // Получаем индекс выбранного подкаталога.

    if (index < 0)                                                  // Если индекс отрицательный, значит подкаталог не выбран.
        return;

    const QString selectedPath =                                    // Получаем полный путь к выбранному подкаталогу из данных комбобокса.
        ui.subdirectoryComboBox->currentData().toString();

    updateDirectory(selectedPath);                                  // Обновляем текущий каталог на выбранный подкаталог.
}

// Переходит в родительский каталог.
void MainWindow::navigateToParentDirectory()
{
    QDir directory(currentPath);                                    // Создаем объект QDir для текущего каталога.

    if (!directory.cdUp())                                          // Проверяем наличие родительского каталога.
        return;

    updateDirectory(directory.absolutePath());                      // Обновляем текущий каталог на родительский.
}
