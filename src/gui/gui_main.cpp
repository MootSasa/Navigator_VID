#include "application/navigation_facade.h"
#include "infrastructure/graph_builder.h"
#include "main_window.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

/// Ищет папку с данными, проверяя несколько возможных расположений.
static QString findDataDir(const QString& arg)
{
    // 1. Если передан аргумент командной строки — используем его
    if (!arg.isEmpty()) {
        if (QDir(arg).exists())
            return arg;
    }

    // 2. Относительно текущей рабочей директории
    if (QDir("data").exists())
        return QString("data");

    // 3. Относительно расположения исполняемого файла (на уровень вверх из build/)
    QString exeDir = QCoreApplication::applicationDirPath();
    QDir dir(exeDir);
    if (dir.exists("data"))
        return dir.absoluteFilePath("data");

    // 4. На уровень выше от расположения исполняемого файла
    dir.cdUp();
    if (dir.exists("data"))
        return dir.absoluteFilePath("data");

    // 5. Ещё на уровень выше (для вложенных build/ директорий)
    dir.cdUp();
    if (dir.exists("data"))
        return dir.absoluteFilePath("data");

    // Не найдено — вернём значение по умолчанию
    return QString("data");
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Transport Navigator");
    app.setApplicationVersion("1.0");

    // Загрузка стилей
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // Поиск папки с данными
    QString dataDir = findDataDir(argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString());

    std::string nodesPath    = (dataDir + "/nodes.csv").toStdString();
    std::string edgesPath    = (dataDir + "/edges.csv").toStdString();
    std::string paramsPath   = (dataDir + "/transport_params.csv").toStdString();
    std::string schedulesPath = (dataDir + "/schedules.csv").toStdString();

    // Загрузка данных
    try {
        infrastructure::GraphBuilder builder;
        builder.loadNodes(nodesPath)
               .loadEdges(edgesPath)
               .loadTransportParams(paramsPath)
               .loadSchedules(schedulesPath);

        auto graph = builder.build();
        auto params = builder.buildTransportParams();

        auto facade = std::make_unique<application::NavigationFacade>(
            std::move(graph), std::move(params));

        MainWindow window(std::move(facade));
        window.show();

        return app.exec();

    } catch (const std::exception& e) {
        // Если не удалось загрузить данные, показываем ошибку
        QMessageBox::critical(nullptr, "Ошибка загрузки",
            QString("Не удалось загрузить данные:\n%1\n\n"
                    "Папка данных: %2\n"
                    "Убедитесь, что CSV файлы находятся в папке data/")
                .arg(e.what())
                .arg(QDir(dataDir).absolutePath()));
        return 1;
    }
}
