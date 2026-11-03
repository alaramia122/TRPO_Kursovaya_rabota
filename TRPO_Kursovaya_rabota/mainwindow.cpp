#include "mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QSignalBlocker>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPainter>

//Конструктор главного окна
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Настраиваем таблицу
    ui.statsTableWidget->horizontalHeader()->setSectionResizeMode(// Растягиваем столбцы таблицы на всю доступную ширину.
        QHeaderView::Stretch
    );
    ui.statsTableWidget->setMinimumSize(300, 250);              // Задаём минимальный размер области таблицы.


    // Создаём виджет для диаграммы
	chartView = new QtCharts::QChartView(ui.chartWidget);       // Создаём специализированный виджет для отображения диаграммы и прикрепляем его к контейнеру, созданному в qt-designer.

    chartView->setMinimumSize(600, 250);                        // Задаём минимальный размер области отображения диаграммы.

    QVBoxLayout* chartLayout = new QVBoxLayout(ui.chartWidget); // Создаём компоновку для размещения диаграммы внутри контейнера.

    chartLayout->addWidget(chartView);                          // Добавляем виджет диаграммы в компоновку контейнера.

	// Создаём объект диаграммы и прикрепляем его к виджету диаграммы.
	series = new QtCharts::QPieSeries();                        // Создаём серию данных для диаграммы.

	chart = new QtCharts::QChart();                             // Создаём объект диаграммы.

	chart->addSeries(series);                                   // Добавляем серию данных в диаграмму.

	chart->setTitle(QStringLiteral("Распределение размера файлов"));    // Устанавливаем заголовок диаграммы.

	chart->legend()->show();                                    // Отображаем легенду диаграммы.

	chartView->setChart(chart);                                 // Устанавливаем диаграмму в виджет диаграммы.

	chartView->setRenderHint(QPainter::Antialiasing);           // Включаем сглаживание для улучшения качества отображения диаграммы.


    // Подключаем обработчики действий пользователя.
    connect(ui.browseButton, &QPushButton::clicked,             // Кнопка выбора каталога.
        this, &MainWindow::selectDirectory);

    connect(ui.upButton, &QPushButton::clicked,                 // Кнопка перехода в родительский каталог.
        this, &MainWindow::navigateToParentDirectory);

    connect(ui.subdirectoryComboBox,                            // Комбобокс перехода в выбранный подкаталог.
        QOverload<int>::of(&QComboBox::activated),
        this, &MainWindow::openSelectedSubdirectory);

	connect(ui.analyzeButton, &QPushButton::clicked,            // Кнопка запуска анализа текущего каталога.
        this, &MainWindow::analyzeCurrentDirectory);

	connect(ui.statsModeComboBox,                               // Комбобокс выбора режима отображения статистики.
        QOverload<int>::of(&QComboBox::activated),
		this, &MainWindow::updateChartAndTable);

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
void MainWindow::updateDirectory(const QString& path)                   // путь к новому каталогу, который нужно открыть
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
    
    ui.subdirectoryComboBox->clear();                                   // Очищаем список подкаталогов в чекербоксе перед его обновлением.

    const QFileInfoList subdirectories = directory.entryInfoList(       // Формируем список подкаталогов текущего каталога.
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::NoSymLinks,// Исключаем файлы, специальные каталоги "." и ".." - сылки на этот и родительский каталоги,
        QDir::Name | QDir::IgnoreCase                                       // Недоступные для чтения элементы и символические ссылки. Сортируем по имени в алфавитном порядке, игноририруя регистр букв.
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

// Выполняет один обход выбранного каталога и собирает статистику.
void MainWindow::analyzeCurrentDirectory()
{
                                                                // Сбрасываем результаты предыдущего анализа.
	totalSize = 0;                                                  // Общий размер файлов в каталоге.
	directorySizes.clear();                                         // Суммарные размеры файлов по подкаталогам.
	categorySizes.clear();                                          // Суммарные размеры файлов по категориям.

    scanDirectory(currentPath, QString(), true);                    // Собираем размеры файлов и обе статистики.

    updateChartAndTable();                                          // Выводим собранную статистику

	ui.statusLineEdit->setText(                                     // Сообщаем пользователю о завершении анализа и общем размере файлов.
        QStringLiteral("Анализ завершён. Общий размер: %1")
        .arg(formatSize(totalSize))
    );
}

// Рекурсивно обрабатывает содержимое каталога, собирает статистику по размерам файлов.
void MainWindow::scanDirectory(
	const QString& path,                                        // Путь к каталогу, который нужно обработать.
	const QString& topLevelBucket,                              // Имя непосредственного подкаталога верхнего уровня, к которому относятся файлы.
	bool isRoot)                                                // Флаг, указывающий, что текущий каталог является корневым для анализа.
{
	QDir directory(path);                                           // Создаем объект QDir для указанного пути.

    
    const QFileInfoList entries = directory.entryInfoList(          // Получаем содержимое каталога одним списком.
        QDir::AllEntries | QDir::NoDotAndDotDot
        | QDir::Readable | QDir::NoSymLinks
    );

	for (const QFileInfo& entry : entries)                          // Обрабатываем каждый элемент каталога. entry - подкаталоги
    {   
        QString bucket;                                                 // Имя подкаталога верхнего уровня, к которому относятся файлы.

		// Рекурсивный обход подкаталогов.
		if (entry.isDir())                                              // Если элемент является каталогом
        {
			
			if (isRoot)                                                     // Если текущий каталог является корневым
            {
				bucket = entry.fileName();                                      // Имя подкаталога верхнего уровня — это имя текущего подкаталога.
            }
            else
            {
				bucket = topLevelBucket;                                        // Имя подкаталога верхнего уровня передаётся рекурсивно.
            }

			scanDirectory(                                                  // Рекурсивно обрабатываем содержимое подкаталога.
				entry.absoluteFilePath(),                                       // Путь к подкаталогу.
				bucket,                                                         // Имя подкаталога верхнего уровня, к которому относятся файлы.
				false                                                           // Флаг, указывающий, что текущий каталог не является корневым для анализа.
            );
        }
		else if (entry.isFile()) {                                      // Если элемент является файлом

            qint64 fileSize = entry.size();                                 // Получаем размер файла в байтах.

            totalSize += fileSize;                                          // Увеличиваем общий размер файлов в каталоге.
			categorySizes[fileCategory(entry.suffix())] += fileSize;        // Увеличиваем суммарный размер файлов в соответствующей категории. Создается новый элемент в categorySizes, если категория ещё не встречалась.

            if (isRoot)                                                     // Если текущий каталог является корневым для анализа
            {
                bucket = QStringLiteral("Файлы в корне");                       // Имя подкаталога верхнего уровня для файлов в корне каталога.
            }
            else
            {
                bucket = topLevelBucket;                                        // Имя подкаталога верхнего уровня передаётся рекурсивно.
            }

			directorySizes[bucket] += fileSize;                             // Увеличиваем суммарный размер файлов в соответствующем подкаталоге верхнего уровня. Создается новый элемент в directorySizes, если подкаталог ещё не встречался.
        }
    }
}

// Определяет категорию файла по его расширению.
QString MainWindow::fileCategory(const QString& extension)          // Принимает расширение файла
{
	const QString suffix = extension.toLower();                         // Приводим расширение к нижнему регистру

    if (   suffix == QStringLiteral("jpg")                              // Категория "Изображения"
        || suffix == QStringLiteral("jpeg")
        || suffix == QStringLiteral("png")
		|| suffix == QStringLiteral("gif")                                  // QStringLiteral - макрос из библиотеки Qt, создающий
        || suffix == QStringLiteral("bmp")                                  // объекты QString из строковых литералов на этапе компиляции
        || suffix == QStringLiteral("webp")
        || suffix == QStringLiteral("svg")
        || suffix == QStringLiteral("tif")
        || suffix == QStringLiteral("tiff")
        || suffix == QStringLiteral("ico"))
    {
        return QStringLiteral("Изображения");
    }

    if (   suffix == QStringLiteral("mp4")                              // Категория "Видео"
        || suffix == QStringLiteral("mkv")
        || suffix == QStringLiteral("avi")
        || suffix == QStringLiteral("mov")
        || suffix == QStringLiteral("wmv")
        || suffix == QStringLiteral("flv")
        || suffix == QStringLiteral("webm")
        || suffix == QStringLiteral("m4v"))
    {
        return QStringLiteral("Видео");
    }

    if (   suffix == QStringLiteral("pdf")                              // Категория "Документы"
        || suffix == QStringLiteral("doc")
        || suffix == QStringLiteral("docx")
        || suffix == QStringLiteral("txt")
        || suffix == QStringLiteral("rtf")
        || suffix == QStringLiteral("odt")
        || suffix == QStringLiteral("xls")
        || suffix == QStringLiteral("xlsx")
        || suffix == QStringLiteral("ppt")
        || suffix == QStringLiteral("pptx")
        || suffix == QStringLiteral("csv"))
    {
        return QStringLiteral("Документы");
    }

    if (   suffix == QStringLiteral("zip")                              // Категория "Архивы"
        || suffix == QStringLiteral("rar")
        || suffix == QStringLiteral("7z")
        || suffix == QStringLiteral("tar")
        || suffix == QStringLiteral("gz")
        || suffix == QStringLiteral("bz2")
        || suffix == QStringLiteral("xz"))
    {
        return QStringLiteral("Архивы");
    }

	return QStringLiteral("Прочие файлы");                              // Категория "Прочие файлы" для всех остальных расширений.
}

// Преобразует размер в байтах в удобный для чтения формат.
QString MainWindow::formatSize(qint64 bytes)                        // Принимает размер в байтах
{
	const double size = static_cast<double>(bytes);                     // Преобразуем размер в байтах в тип double для последующих вычислений.

	if (bytes < 1024)                                                       // Если размер меньше 1024 байт, возвращаем его в байтах.
        return QStringLiteral("%1 Б").arg(bytes);

	if (bytes < 1024 * 1024)                                                // Если размер меньше 1 МБ, возвращаем его в килобайтах с одним знаком после запятой.
        return QStringLiteral("%1 КБ")
        .arg(size / 1024.0, 0, 'f', 1);

	if (bytes < 1024 * 1024 * 1024)                                         // Если размер меньше 1 ГБ, возвращаем его в мегабайтах с одним знаком после запятой.
        return QStringLiteral("%1 МБ")
        .arg(size / (1024.0 * 1024.0), 0, 'f', 1);

	return QStringLiteral("%1 ГБ")                                          // Если размер больше или равен 1 ГБ, возвращаем его в гигабайтах с одним знаком после запятой.
        .arg(size / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

// Вызывает updateStatisticsTable() и updateChart().
void MainWindow::updateChartAndTable() {
    updateStatisticsTable();                                                // Выводим собранную статистику
    updateChart();                                                          // Обновляем диаграмму на основе той же статистики.
}

// возвращает словарь значений под выбранный режим отображения
QMap<QString, qint64>* MainWindow::selectedView() {
	if (ui.statsModeComboBox->currentIndex() == 0)                          // Если выбран режим отображения по категориям
    {
        return &directorySizes;                                                 // Указываем на статистику по подкаталогам.
    }
    else                                                                    // Если выбран режим "По категориям"
    {
        return &categorySizes;                                                  // Указываем на статистику по категориям.
    }
}

// Выводит в таблицу статистику выбранного режима.
void MainWindow::updateStatisticsTable()
{
	ui.statsTableWidget->setRowCount(0);                                    // Очищаем таблицу перед заполнением новыми данными.

	const QMap<QString, qint64>* statistics;                                // Указатель на выбранную статистику (по подкаталогам или по категориям).

	statistics = selectedView();                                            // Получаем указатель на выбранную статистику.

	for (auto it = statistics->cbegin(); it != statistics->cend(); ++it)    // Проходим по всем элементам выбранной статистики.
    {
		QString name = it.key();                                                // Получаем имя подкаталога или категории.
		qint64 size = it.value();                                               // Получаем суммарный размер файлов в подкаталоге или категории.

		double share = 0.0;                                                     // Переменная для хранения доли размера в процентах.

		if (totalSize > 0)                                                      // Если общий размер файлов больше нуля
        {
			share = static_cast<double>(size) * 100.0                               // Вычисляем долю размера в процентах.
                / static_cast<double>(totalSize);
        }

		// Добавляем новую строку в таблицу статистики.
		int row = ui.statsTableWidget->rowCount();                              // Получаем текущее количество строк в таблице.
		ui.statsTableWidget->insertRow(row);                                    // Вставляем новую строку в конец таблицы.
        
        // Заполняем ячейки новой строки данными.
		ui.statsTableWidget->setItem(                                           // Столбец 0: Имя подкаталога или категории.
			row, 0, new QTableWidgetItem(name)                                      // номер строки, номер столбца, указатель на новый элемент таблицы с именем подкаталога или категории.
        );

		ui.statsTableWidget->setItem(                                           // Столбец 1: Размер.
			row, 1, new QTableWidgetItem(formatSize(size))                          // номер строки, номер столбца, указатель на новый элемент таблицы с отформатированным размером.
        );

		ui.statsTableWidget->setItem(                                           // Столбец 2: Доля в процентах.
			row, 2,                                                                 // номер строки, номер столбца, указатель на новый элемент таблицы с долей размера в процентах.
			new QTableWidgetItem(QString::number(share, 'f', 1))                        // преобразуем число в строку с одним знаком после запятой.
        );
    }
}

// Обновляет круговую диаграмму для выбранного режима статистики.
void MainWindow::updateChart()
{
    // Выбираем статистику, соответствующую текущему режиму.
	const QMap<QString, qint64>* statistics;                                // Указатель на выбранную статистику.

    statistics = selectedView();                                            // Получаем указатель на выбранную статистику.

	// Очищаем старые сектора диаграммы.
    series->clear();

    // Добавляем в диаграмму элементы статистики с ненулевым размером.
    qint64 otherSize = 0;                                                   // Храним общий размер файлов, попавших в категорию «Другое».

    for (auto it = statistics->cbegin(); it != statistics->cend(); ++it)    // Проходим по всем элементам выбранной статистики.
    {
        QString name = it.key();                                                // Получаем имя подкаталога или категории.
        qint64 size = it.value();                                               // Получаем суммарный размер файлов в подкаталоге или категории.

        if (size <= 0)                                                          // Пропускаем элементы с нулевым размером, чтобы не отображать их на диаграмме.
        {
            continue;
        }

		if (ui.statsModeComboBox->currentIndex() == 0)                          // Если выбран режим "По подкаталогам"
        {
            // Вычисляем долю элемента от общего размера файлов.
            double share = 0.0;                                                     // Переменная для хранения доли размера в процентах.

            if (totalSize > 0)                                                      // Если общий размер файлов больше нуля
            {
                share = static_cast<double>(size) * 100.0                         // Вычисляем долю размера в процентах.
                    / static_cast<double>(totalSize);
            }

            // Объединяем элементы с долей меньше 2% в категорию «Другое».
            if (share < 2.0)                                                        // Если доля элемента меньше 2%
            {
                otherSize += size;                                                      // Увеличиваем суммарный размер файлов, попавших в категорию «Другое».
                continue;                                                               // Пропускаем добавление этого элемента в диаграмму.
            }
        }

		series->append(name, size);                                             // Добавляем сектор диаграммы с именем и размером.
    }

    // Добавляем сектор «Другое», если в него попали файлы.
	if (otherSize > 0)                                                      // Если суммарный размер файлов в категории «Другое» больше нуля
    {
        series->append(QStringLiteral("Другое"), otherSize);                    // Добавляем сектор диаграммы с именем «Другое» и суммарным размером.
    }

    // Обновляем заголовок
	if (series->count() == 0)                                               // Если в диаграмме нет секторов
		chart->setTitle(QStringLiteral("Нет данных для отображения"));          // Устанавливаем заголовок диаграммы как "Нет данных для отображения".
	else                                                                    // Если в диаграмме есть хотя бы один сектор
		chart->setTitle(QStringLiteral("Распределение размера файлов"));        // Устанавливаем заголовок диаграммы как "Распределение размера файлов".
}