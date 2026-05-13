#pragma once

#include "application/navigation_facade.h"

#include <QMainWindow>
#include <QFutureWatcher>
#include <QtConcurrent>

class MapWidget;
class MapScene;
class RoutePanel;

/**
 * @brief Главное окно приложения Navigator.
 *
 * Содержит карту (MapWidget) и боковую панель (RoutePanel),
 * связывает их через NavigationFacade.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /// Конструктор с готовым фасадом навигации.
    explicit MainWindow(std::unique_ptr<application::NavigationFacade> facade,
                        QWidget* parent = nullptr);

private slots:
    /// Обработка выбора узла на карте.
    void onNodeSelected(domain::NodeId id);

    /// Обработка запроса на поиск маршрута (все стратегии).
    void onRouteRequested(domain::NodeId start, domain::NodeId goal,
                          std::chrono::system_clock::time_point departure);

    /// Обработка завершения асинхронного поиска маршрута.
    void onRouteFinished();

    /// Обработка выбора маршрута из списка.
    void onRouteSelected(int index);

    /// Обработка сброса маршрута.
    void onRouteReset();

    /// Обработка центрирования карты.
    void onCenterMap();

private:
    std::unique_ptr<application::NavigationFacade> facade_;
    MapScene* scene_;
    MapWidget* mapWidget_;
    RoutePanel* routePanel_;

    std::vector<application::RouteResult> currentResults_;

    /// Watcher для отслеживания завершения асинхронного поиска.
    QFutureWatcher<std::vector<application::RouteResult>>* routeWatcher_ = nullptr;

    /// Режим выбора: что выбирает следующий клик по узлу.
    enum class SelectionMode { Start, Goal };
    SelectionMode selectionMode_ = SelectionMode::Start;
};
