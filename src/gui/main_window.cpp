#include "main_window.h"
#include "map_scene.h"
#include "map_widget.h"
#include "route_panel.h"
#include "node_item.h"

#include <QHBoxLayout>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(std::unique_ptr<application::NavigationFacade> facade,
                       QWidget* parent)
    : QMainWindow(parent)
    , facade_(std::move(facade))
{
    setWindowTitle("Transport Navigator");
    setMinimumSize(900, 600);
    resize(1200, 800);

    // Сцена и виджет карты
    scene_ = new MapScene(this);
    scene_->loadGraph(facade_->getGraph());

    mapWidget_ = new MapWidget(scene_, this);
    mapWidget_->setFocus();

    // Боковая панель
    routePanel_ = new RoutePanel(this);
    routePanel_->populateNodes(facade_->getGraph().getAllNodes());

    // Основной layout
    auto* central = new QWidget();
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(routePanel_);
    layout->addWidget(mapWidget_, 1);
    setCentralWidget(central);

    // Статусбар
    statusBar()->showMessage(
        QString("Загружено узлов: %1")
            .arg(facade_->getGraph().nodeCount()));

    // Связи сигналов/слотов
    connect(scene_, &MapScene::nodeSelected,
            this, &MainWindow::onNodeSelected);

    connect(routePanel_, &RoutePanel::routeRequested,
            this, &MainWindow::onRouteRequested);

    connect(routePanel_, &RoutePanel::routeSelected,
            this, &MainWindow::onRouteSelected);

    connect(routePanel_, &RoutePanel::routeReset,
            this, &MainWindow::onRouteReset);

    connect(routePanel_, &RoutePanel::centerMapRequested,
            this, &MainWindow::onCenterMap);
}

void MainWindow::onNodeSelected(domain::NodeId id)
{
    // Чередуем выбор: старт -> финиш -> старт -> ...
    if (selectionMode_ == SelectionMode::Start) {
        scene_->clearNodeHighlights();
        scene_->clearRoute();
        routePanel_->clearResults();
        currentResults_.clear();

        scene_->setNodeHighlight(id, NodeItem::HighlightRole::Start);
        routePanel_->setStartNode(id);
        selectionMode_ = SelectionMode::Goal;

        const auto& node = facade_->getGraph().getNode(id);
        QString name = node.name.empty() ?
            QString("Узел %1").arg(id) :
            QString::fromStdString(node.name);
        statusBar()->showMessage(
            QString("Старт: %1 — выберите конечную точку").arg(name));
    } else {
        scene_->setNodeHighlight(id, NodeItem::HighlightRole::Goal);
        routePanel_->setGoalNode(id);
        selectionMode_ = SelectionMode::Start;

        const auto& node = facade_->getGraph().getNode(id);
        QString name = node.name.empty() ?
            QString("Узел %1").arg(id) :
            QString::fromStdString(node.name);
        statusBar()->showMessage(
            QString("Финиш: %1 — нажмите «Найти маршрут»").arg(name));
    }
}

void MainWindow::onRouteRequested(domain::NodeId start, domain::NodeId goal,
                                  std::chrono::system_clock::time_point departure)
{
    if (!facade_->hasNode(start) || !facade_->hasNode(goal)) {
        statusBar()->showMessage("Ошибка: узел не найден");
        return;
    }

    if (start == goal) {
        statusBar()->showMessage("Старт и финиш совпадают");
        return;
    }

    // Если предыдущий поиск ещё не завершён - игнорируем новый запрос
    if (routeWatcher_ && routeWatcher_->isRunning()) {
        statusBar()->showMessage("Подождите, идёт поиск маршрута...");
        return;
    }

    statusBar()->showMessage("Поиск маршрута...");

    // Запускаем поиск в пуле потоков
    auto future = QtConcurrent::run([this, start, goal, departure]() {
        return facade_->findAllRoutes(start, goal, departure);
    });

    routeWatcher_ = new QFutureWatcher<std::vector<application::RouteResult>>(this);
    connect(routeWatcher_, &QFutureWatcherBase::finished,
            this, &MainWindow::onRouteFinished);
    routeWatcher_->setFuture(future);
}

void MainWindow::onRouteFinished()
{
    auto results = routeWatcher_->result();
    routeWatcher_->deleteLater();
    routeWatcher_ = nullptr;

    currentResults_ = std::move(results);

    if (currentResults_.empty()) {
        statusBar()->showMessage("Маршрут не найден");
        routePanel_->displayResults({}, facade_->getGraph());
        scene_->clearRoute();
        return;
    }

    // Отображаем результаты в панели
    routePanel_->displayResults(currentResults_, facade_->getGraph());

    // Подсвечиваем первый маршрут на карте
    scene_->highlightRoute(currentResults_[0], facade_->getGraph());

    QString stratInfo;
    for (const auto& r : currentResults_) {
        int mins = static_cast<int>(r.total_cost.time_seconds) / 60;
        stratInfo += QString("%1: %2мин  ").arg(
            QString::fromStdString(r.strategy_name)).arg(mins);
    }
    statusBar()->showMessage(
        QString("Найдено маршрутов: %1 | %2")
            .arg(currentResults_.size()).arg(stratInfo));
}

void MainWindow::onRouteSelected(int index)
{
    if (index < 0 || index >= static_cast<int>(currentResults_.size())) return;

    scene_->highlightRoute(currentResults_[index], facade_->getGraph());

    const auto& route = currentResults_[index];
    statusBar()->showMessage(
        QString("Маршрут: %1 -- %2мин, %3р, %4 пер.")
            .arg(QString::fromStdString(route.strategy_name))
            .arg(static_cast<int>(route.total_cost.time_seconds) / 60)
            .arg(route.total_cost.money_rub, 0, 'f', 0)
            .arg(route.total_cost.transfers));
}

void MainWindow::onRouteReset()
{
    scene_->clearNodeHighlights();
    scene_->clearRoute();
    currentResults_.clear();
    selectionMode_ = SelectionMode::Start;
    statusBar()->showMessage("Маршрут сброшен — выберите стартовую точку");
}

void MainWindow::onCenterMap()
{
    mapWidget_->fitToView();
    statusBar()->showMessage("Карта центрирована");
}
