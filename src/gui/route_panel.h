#pragma once

#include "application/a_star_solver.h"
#include "domain/types.h"

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>

class QVBoxLayout;

/**
 * @brief Боковая панель маршрутов.
 *
 * Вертикальная колонка: выбор старт/финиш, время, кнопки,
 * карточки результатов, пошаговые детали маршрута.
 */
class RoutePanel : public QWidget {
    Q_OBJECT

public:
    explicit RoutePanel(QWidget* parent = nullptr);

    /// Заполняет комбобоксы узлами из графа.
    void populateNodes(const std::unordered_map<domain::NodeId, domain::Node>& nodes);

    /// Устанавливает стартовый узел.
    void setStartNode(domain::NodeId id);

    /// Устанавливает конечный узел.
    void setGoalNode(domain::NodeId id);

    /// Отображает результаты поиска маршрутов.
    void displayResults(const std::vector<application::RouteResult>& results,
                        const domain::Graph& graph);

    /// Очищает результаты.
    void clearResults();

signals:
    /// Сигнал запроса на поиск маршрута (все стратегии).
    void routeRequested(domain::NodeId start, domain::NodeId goal,
                        std::chrono::system_clock::time_point departure);

    /// Сигнал выбора маршрута из списка.
    void routeSelected(int index);

    /// Сигнал сброса маршрута (очистка выделений и результатов).
    void routeReset();

    /// Сигнал центрирования карты (fit-to-view).
    void centerMapRequested();

private slots:
    void onFindClicked();
    void onResetClicked();
    void onCenterClicked();
    void onNowCheckboxToggled(bool checked);
    void onTimeEditTextChanged(const QString& text);

private:
    /// Создаёт карточку маршрута для списка результатов.
    QWidget* createRouteCard(const application::RouteResult& result,
                             const domain::Graph& graph, int index);

    /// Форматирует время в пути.
    static QString formatDuration(double seconds);

    /// Возвращает название типа транспорта.
    static QString transportName(domain::TransportType t);

    /// Показывает детали маршрута по индексу.
    void showRouteDetails(int index);

    /// Фильтр событий для кликов по карточкам маршрутов.
    bool eventFilter(QObject* watched, QEvent* event) override;

    // Панель выбора маршрута
    QComboBox* startCombo_;
    QComboBox* goalCombo_;
    QCheckBox* nowCheckbox_;
    QLineEdit* timeEdit_;
    QPushButton* findButton_;
    QPushButton* resetButton_;
    QPushButton* centerButton_;

    QWidget* resultsContainer_;
    QVBoxLayout* resultsLayout_;

    // Детали
    QTextBrowser* detailsBrowser_;

    // Храним текущие результаты для отображения деталей
    std::vector<application::RouteResult> currentResults_;
    const domain::Graph* currentGraph_ = nullptr;
};
