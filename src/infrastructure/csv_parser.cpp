#include "infrastructure/csv_parser.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace infrastructure {

// ==================== Приватные вспомогательные методы ====================

std::string CsvParser::trim(const std::string& str) const {
    size_t start = 0;
    size_t end = str.length();

    // Пропускаем начальные пробелы
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    // Пропускаем конечные пробелы
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }

    std::string result = str.substr(start, end - start);

    // Удаляем кавычки, если они есть
    if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.size() - 2);
    }

    return result;
}

std::vector<std::string> CsvParser::splitLine(const std::string& line, char delimiter) const {
    std::vector<std::string> cells;
    bool inQuotes = false;
    std::string currentCell;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (c == '"') {
            inQuotes = !inQuotes;
            currentCell += c;
        } else if (c == delimiter && !inQuotes) {
            cells.push_back(trim(currentCell));
            currentCell.clear();
        } else {
            currentCell += c;
        }
    }

    // Добавляем последнюю ячейку
    cells.push_back(trim(currentCell));

    return cells;
}

double CsvParser::parseDouble(const std::string& str) const {
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return 0.0;
    }
}

int CsvParser::parseInt(const std::string& str) const {
    try {
        return std::stoi(str);
    } catch (const std::exception&) {
        return 0;
    }
}

// ==================== Публичные методы ====================

std::vector<std::vector<std::string>> CsvParser::parse(std::istream& input) const {
    std::vector<std::vector<std::string>> table;
    std::string line;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        table.push_back(splitLine(line));
    }

    return table;
}

std::vector<ParsedNode> CsvParser::parseNodes(std::istream& input) const {
    std::vector<ParsedNode> nodes;
    std::string line;
    bool isFirstLine = true;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Пропускаем заголовок (первая строка, если содержит "node_id")
        if (isFirstLine) {
            isFirstLine = false;
            if (line.find("node_id") != std::string::npos) {
                continue;
            }
        }

        std::vector<std::string> cells = splitLine(line);

        // Пропускаем некорректные строки
        if (cells.size() < 5) {
            continue;
        }

        ParsedNode node;
        node.id = parseInt(cells[0]);
        node.type = cells[1];
        node.name = cells[2];
        node.x = parseDouble(cells[3]);
        node.y = parseDouble(cells[4]);

        nodes.push_back(node);
    }

    return nodes;
}

std::vector<ParsedEdge> CsvParser::parseEdges(std::istream& input) const {
    std::vector<ParsedEdge> edges;
    std::string line;
    bool isFirstLine = true;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Пропускаем заголовок (первая строка, если содержит "edge_id")
        if (isFirstLine) {
            isFirstLine = false;
            if (line.find("edge_id") != std::string::npos) {
                continue;
            }
        }

        std::vector<std::string> cells = splitLine(line);

        // Пропускаем некорректные строки
        if (cells.size() < 6) {
            continue;
        }

        ParsedEdge edge;
        edge.id = parseInt(cells[0]);
        edge.from_node = parseInt(cells[1]);
        edge.to_node = parseInt(cells[2]);
        edge.transport_type = cells[3];
        edge.length_meters = parseDouble(cells[4]);
        edge.name = cells[5];

        edges.push_back(edge);
    }

    return edges;
}

std::vector<ParsedTransportParam> CsvParser::parseTransportParams(std::istream& input) const {
    std::vector<ParsedTransportParam> params;
    std::string line;
    bool isFirstLine = true;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Пропускаем заголовок (первая строка, если содержит "transport_type")
        if (isFirstLine) {
            isFirstLine = false;
            if (line.find("transport_type") != std::string::npos) {
                continue;
            }
        }

        std::vector<std::string> cells = splitLine(line);

        // Пропускаем некорректные строки
        if (cells.size() < 3) {
            continue;
        }

        ParsedTransportParam param;
        param.transport_type = cells[0];
        param.param_name = cells[1];
        param.param_value = cells[2];

        params.push_back(param);
    }

    return params;
}

std::vector<ParsedSchedule> CsvParser::parseSchedules(std::istream& input) const {
    std::vector<ParsedSchedule> schedules;
    std::string line;
    bool isFirstLine = true;

    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Пропускаем заголовок (первая строка, если содержит "edge_id")
        if (isFirstLine) {
            isFirstLine = false;
            if (line.find("edge_id") != std::string::npos) {
                continue;
            }
        }

        std::vector<std::string> cells = splitLine(line);

        // Пропускаем некорректные строки
        if (cells.size() < 3) {
            continue;
        }

        ParsedSchedule schedule;
        schedule.edge_id = parseInt(cells[0]);
        schedule.schedule_type = cells[1];
        schedule.params = cells[2];

        schedules.push_back(schedule);
    }

    return schedules;
}

} // namespace infrastructure
