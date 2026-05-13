#include "route_panel.h"
#include "domain/graph.h"
#include "domain/combined_cost.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMouseEvent>
#include <QScrollArea>
#include <QTime>

RoutePanel::RoutePanel(QWidget* parent)
    : QWidget(parent)
{
    setFixedWidth(380);

    // Единая вертикальная колонка
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Скроллящаяся область
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* container = new QWidget();
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    // Заголовок
    auto* titleLabel = new QLabel("Navigator");
    titleLabel->setObjectName("panelTitle");
    layout->addWidget(titleLabel);

    // Разделитель
    auto* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setStyleSheet("color: #313244;");
    layout->addWidget(sep1);

    // Старт
    auto* startLabel = new QLabel("Старт");
    startLabel->setObjectName("sectionLabel");
    layout->addWidget(startLabel);

    startCombo_ = new QComboBox();
    startCombo_->setObjectName("startCombo");
    startCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    layout->addWidget(startCombo_);

    // Финиш
    auto* goalLabel = new QLabel("Финиш");
    goalLabel->setObjectName("sectionLabel");
    layout->addWidget(goalLabel);

    goalCombo_ = new QComboBox();
    goalCombo_->setObjectName("goalCombo");
    goalCombo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    layout->addWidget(goalCombo_);

    // Время отправления
    auto* timeLabel = new QLabel("Время отправления");
    timeLabel->setObjectName("sectionLabel");
    layout->addWidget(timeLabel);

    // Галочка "Текущее время" (по умолчанию включена)
    nowCheckbox_ = new QCheckBox("Текущее время");
    nowCheckbox_->setChecked(true);
    nowCheckbox_->setObjectName("nowCheckbox");
    layout->addWidget(nowCheckbox_);

    // Поле ввода времени (скрыто когда галочка включена)
    timeEdit_ = new QLineEdit();
    timeEdit_->setObjectName("timeEdit");
    timeEdit_->setPlaceholderText("ЧЧ:ММ (например 14:30)");
    timeEdit_->setEnabled(false);  // Отключено пока галочка включена
    layout->addWidget(timeEdit_);

    connect(nowCheckbox_, &QCheckBox::toggled,
            this, &RoutePanel::onNowCheckboxToggled);

    connect(timeEdit_, &QLineEdit::textChanged,
            this, &RoutePanel::onTimeEditTextChanged);

    // Кнопки: Найти / Сброс / Центр
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(6);

    findButton_ = new QPushButton("Найти");
    findButton_->setObjectName("actionButton");
    btnRow->addWidget(findButton_);

    resetButton_ = new QPushButton("Сброс");
    resetButton_->setObjectName("actionButton");
    btnRow->addWidget(resetButton_);

    centerButton_ = new QPushButton("Центр");
    centerButton_->setObjectName("actionButton");
    centerButton_->setToolTip("Центрировать карту");
    btnRow->addWidget(centerButton_);

    layout->addLayout(btnRow);

    connect(findButton_, &QPushButton::clicked,
            this, &RoutePanel::onFindClicked);
    connect(resetButton_, &QPushButton::clicked,
            this, &RoutePanel::onResetClicked);
    connect(centerButton_, &QPushButton::clicked,
            this, &RoutePanel::onCenterClicked);

    // Разделитель
    auto* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #313244;");
    layout->addWidget(sep2);

    // Результаты (3 карточки)
    auto* resLabel = new QLabel("Маршруты");
    resLabel->setObjectName("sectionLabel");
    layout->addWidget(resLabel);

    resultsContainer_ = new QWidget();
    resultsLayout_ = new QVBoxLayout(resultsContainer_);
    resultsLayout_->setContentsMargins(0, 0, 0, 0);
    resultsLayout_->setSpacing(6);
    resultsLayout_->addStretch();

    layout->addWidget(resultsContainer_);

    // Разделитель
    auto* sep3 = new QFrame();
    sep3->setFrameShape(QFrame::HLine);
    sep3->setStyleSheet("color: #313244;");
    layout->addWidget(sep3);

    // Детали маршрута
    auto* detLabel = new QLabel("Детали маршрута");
    detLabel->setObjectName("sectionLabel");
    layout->addWidget(detLabel);

    detailsBrowser_ = new QTextBrowser();
    detailsBrowser_->setObjectName("routeDetails");
    detailsBrowser_->setOpenExternalLinks(false);
    detailsBrowser_->setOpenLinks(false);
    detailsBrowser_->setReadOnly(true);
    detailsBrowser_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    detailsBrowser_->setMaximumHeight(350);
    layout->addWidget(detailsBrowser_);

    layout->addStretch();

    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);
}

void RoutePanel::populateNodes(const std::unordered_map<domain::NodeId, domain::Node>& nodes)
{
    startCombo_->clear();
    goalCombo_->clear();

    // Сортируем узлы по ID для предсказуемого порядка
    std::vector<std::pair<domain::NodeId, std::string>> sorted;
    for (const auto& [id, node] : nodes) {
        std::string name = node.name.empty() ?
            ("Узел " + std::to_string(id)) : node.name;
        sorted.emplace_back(id, name);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [id, name] : sorted) {
        QString text = QString("%1 — %2").arg(id).arg(QString::fromStdString(name));
        startCombo_->addItem(text, id);
        goalCombo_->addItem(text, id);
    }
}

void RoutePanel::setStartNode(domain::NodeId id)
{
    int idx = startCombo_->findData(id);
    if (idx >= 0) {
        startCombo_->setCurrentIndex(idx);
    }
}

void RoutePanel::setGoalNode(domain::NodeId id)
{
    int idx = goalCombo_->findData(id);
    if (idx >= 0) {
        goalCombo_->setCurrentIndex(idx);
    }
}

void RoutePanel::displayResults(const std::vector<application::RouteResult>& results,
                                const domain::Graph& graph)
{
    currentResults_ = results;
    currentGraph_ = &graph;

    // Очищаем старые карточки
    QLayoutItem* item;
    while ((item = resultsLayout_->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    if (results.empty()) {
        auto* noResult = new QLabel("Маршрут не найден");
        noResult->setStyleSheet("color: #F38BA8; padding: 8px; font-style: italic;");
        resultsLayout_->addWidget(noResult);
        resultsLayout_->addStretch();
        detailsBrowser_->clear();
        return;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        auto* card = createRouteCard(results[i], graph, static_cast<int>(i));
        resultsLayout_->addWidget(card);
    }
    resultsLayout_->addStretch();

    // Показываем детали первого маршрута
    if (!results.empty()) {
        showRouteDetails(0);
    }
}

void RoutePanel::clearResults()
{
    currentResults_.clear();
    currentGraph_ = nullptr;
    detailsBrowser_->clear();

    QLayoutItem* item;
    while ((item = resultsLayout_->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    resultsLayout_->addStretch();
}

void RoutePanel::onNowCheckboxToggled(bool checked)
{
    timeEdit_->setEnabled(!checked);
    if (checked) {
        timeEdit_->clear();
    }
}

void RoutePanel::onTimeEditTextChanged(const QString& text)
{
    // Автоформатирование: вставляем ':' после 2 цифр
    QString digits;
    for (int i = 0; i < text.size(); ++i) {
        if (text[i].isDigit()) {
            digits += text[i];
        }
    }

    // Ограничиваем 4 цифрами (ЧЧММ)
    if (digits.size() > 4) {
        digits = digits.left(4);
    }

    QString formatted;
    if (digits.size() > 2) {
        formatted = digits.left(2) + ":" + digits.mid(2);
    } else {
        formatted = digits;
    }

    // Обновляем только если текст изменился
    if (formatted != text) {
        timeEdit_->blockSignals(true);
        int cursorPos = timeEdit_->cursorPosition();
        timeEdit_->setText(formatted);
        // Корректируем позицию курсора
        if (cursorPos >= 2 && formatted.size() > text.size()) {
            cursorPos++;  // Двоеточие сдвинуло курсор
        }
        timeEdit_->setCursorPosition(qMin(cursorPos, formatted.size()));
        timeEdit_->blockSignals(false);
    }
}

void RoutePanel::onFindClicked()
{
    domain::NodeId startId = startCombo_->currentData().toInt();
    domain::NodeId goalId = goalCombo_->currentData().toInt();

    if (startId == 0 || goalId == 0) return;

    // Определяем время отправления
    std::chrono::system_clock::time_point departure;

    if (nowCheckbox_->isChecked()) {
        departure = std::chrono::system_clock::now();
    } else {
        // Парсим время из QLineEdit
        QString timeText = timeEdit_->text().trimmed();
        QTime t = QTime::fromString(timeText, "HH:mm");
        if (!t.isValid()) {
            t = QTime::fromString(timeText, "H:mm");
        }
        if (!t.isValid()) {
            // Пробуем без ведущего нуля
            bool ok = false;
            int sepIdx = timeText.indexOf(':');
            if (sepIdx > 0) {
                int h = timeText.left(sepIdx).toInt(&ok);
                int m = timeText.mid(sepIdx + 1).toInt(&ok);
                if (ok && h >= 0 && h < 24 && m >= 0 && m < 60) {
                    t = QTime(h, m);
                }
            }
        }
        if (!t.isValid()) {
            timeEdit_->setStyleSheet("border: 1px solid #F38BA8;");
            return;
        }
        timeEdit_->setStyleSheet("");

        auto now = std::chrono::system_clock::now();
        auto now_tt = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_tt);
        now_tm->tm_hour = t.hour();
        now_tm->tm_min = t.minute();
        now_tm->tm_sec = 0;
        departure = std::chrono::system_clock::from_time_t(std::mktime(now_tm));
    }

    emit routeRequested(startId, goalId, departure);
}

void RoutePanel::onResetClicked()
{
    // Очищаем результаты и детали
    clearResults();
    startCombo_->setCurrentIndex(0);
    goalCombo_->setCurrentIndex(0);
    // Сигнал для очистки карты
    emit routeReset();
}

void RoutePanel::onCenterClicked()
{
    emit centerMapRequested();
}

QWidget* RoutePanel::createRouteCard(const application::RouteResult& result,
                                     const domain::Graph& graph, int index)
{
    auto* card = new QFrame();
    card->setObjectName("routeCard");
    card->setStyleSheet(
        "QFrame#routeCard {"
        "  background-color: #313244;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "}"
        "QFrame#routeCard:hover {"
        "  background-color: #3B3B54;"
        "}"
    );
    card->setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    // Название стратегии
    QString stratName;
    QString stratEmoji;
    QColor stratColor;
    if (result.strategy_name == "fastest") {
        stratName = "Самый быстрый";
        stratEmoji = "⚡";
        stratColor = QColor("#42A5F5");
    } else if (result.strategy_name == "cheapest") {
        stratName = "Самый дешёвый";
        stratEmoji = "💰";
        stratColor = QColor("#66BB6A");
    } else if (result.strategy_name == "convenient") {
        stratName = "Самый удобный";
        stratEmoji = "🛋";
        stratColor = QColor("#AB47BC");
    }

    auto* stratLabel = new QLabel(QString("%1 %2").arg(stratEmoji).arg(stratName));
    stratLabel->setStyleSheet(
        QString("font-weight: bold; font-size: 14px; color: %1;")
            .arg(stratColor.name()));
    layout->addWidget(stratLabel);

    // Статистика
    const auto& cost = result.total_cost;
    QString stats = QString("Время: %1  Стоимость: %2р  Пересадки: %3")
        .arg(formatDuration(cost.time_seconds))
        .arg(cost.money_rub, 0, 'f', 0)
        .arg(cost.transfers);
    auto* statsLabel = new QLabel(stats);
    statsLabel->setStyleSheet("color: #A6ADC8; font-size: 12px;");
    layout->addWidget(statsLabel);

    // Клик по карточке
    card->installEventFilter(this);

    // Сохраняем индекс в property
    card->setProperty("routeIndex", index);

    return card;
}

void RoutePanel::showRouteDetails(int index)
{
    if (index < 0 || index >= static_cast<int>(currentResults_.size()) || !currentGraph_) return;

    QString html;
    const auto& route = currentResults_[index];
    for (size_t i = 0; i < route.edges.size(); ++i) {
        // Фактический тип транспорта (Car->Taxi при наличии билета)
        domain::TransportType actual = (i < route.actual_transports.size())
            ? route.actual_transports[i]
            : domain::TransportType::Walk;

        // Имя ребра берём из графа
        QString edgeName;
        const auto& edges = currentGraph_->getEdgesFrom(route.nodes[i]);
        for (const auto& e : edges) {
            if (e.id == route.edges[i]) {
                edgeName = QString::fromStdString(e.name);
                break;
            }
        }

        QString colorName;
        switch (actual) {
            case domain::TransportType::Walk:  colorName = "#66BB6A"; break;
            case domain::TransportType::Car:   colorName = "#757575"; break;
            case domain::TransportType::Taxi:  colorName = "#FDD835"; break;
            case domain::TransportType::Bus:   colorName = "#1E88E5"; break;
            case domain::TransportType::Metro: colorName = "#E53935"; break;
        }
        QString step = QString("%1. %2 — %3")
            .arg(i + 1)
            .arg(transportName(actual))
            .arg(edgeName);
        html += QString("<div style='color:%1; padding:4px 0; border-bottom:1px solid #313244;'>%2</div>")
            .arg(colorName)
            .arg(step.toHtmlEscaped());
    }
    detailsBrowser_->setHtml(html);
}

QString RoutePanel::formatDuration(double seconds)
{
    int total = static_cast<int>(seconds);
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;
    if (h > 0) {
        return QString("%1ч %2мин").arg(h).arg(m);
    }
    if (m > 0) {
        return QString("%1мин %2с").arg(m).arg(s);
    }
    return QString("%1с").arg(s);
}

QString RoutePanel::transportName(domain::TransportType t)
{
    switch (t) {
        case domain::TransportType::Walk:  return "Пешком";
        case domain::TransportType::Car:   return "Авто";
        case domain::TransportType::Taxi:  return "Такси";
        case domain::TransportType::Bus:   return "Автобус";
        case domain::TransportType::Metro: return "Метро";
    }
    return "❓";
}

bool RoutePanel::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto* card = qobject_cast<QFrame*>(watched);
        if (card && card->objectName() == "routeCard") {
            int idx = card->property("routeIndex").toInt();
            emit routeSelected(idx);
            showRouteDetails(idx);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
