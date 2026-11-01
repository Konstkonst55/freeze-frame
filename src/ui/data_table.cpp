#include "ui/data_table.h"
#include "ui/theme.h"

#include <FL/fl_draw.H>

DataTable::DataTable(int X, int Y, int W, int H) : Fl_Table_Row(X, Y, W, H) {
    col_header(true);
    col_header_height(44);
    row_height(48);
    box(FL_FLAT_BOX);
    color(Theme::surface);
    type(SELECT_SINGLE);
    end();
}

void DataTable::setup(std::vector<std::string> headers, std::vector<int> weights) {
    headers_ = std::move(headers);
    weights_ = std::move(weights);
    sumWeight_ = 0;

    for (int weight : weights_) {
        sumWeight_ += weight;
    }

    if (sumWeight_ <= 0) {
        sumWeight_ = 1;
    }

    cols(static_cast<int>(headers_.size()));
    applyWidths();
    end();
}

void DataTable::applyWidths() {
    if (sumWeight_ <= 0 || weights_.empty()) {
        return;
    }

    int available = w() - 4;

    if (available < static_cast<int>(weights_.size()) * 40) {
        available = static_cast<int>(weights_.size()) * 40;
    }

    int used = 0;

    for (int index = 0; index < static_cast<int>(weights_.size()); ++index) {
        int columnWidth;

        if (index == static_cast<int>(weights_.size()) - 1) {
            columnWidth = available - used;
        } else {
            columnWidth = static_cast<int>(static_cast<long long>(available) * weights_[index] / sumWeight_);
        }

        if (columnWidth < 40) {
            columnWidth = 40;
        }

        col_width(index, columnWidth);
        used += columnWidth;
    }
}

void DataTable::resize(int X, int Y, int W, int H) {
    Fl_Table_Row::resize(X, Y, W, H);
    applyWidths();
    redraw();
}

void DataTable::setRows(const std::vector<std::vector<std::string>>& data, const std::vector<long>& ids) {
    rows_ = data;
    ids_ = ids;
    rows(static_cast<int>(rows_.size()));
    redraw();
}

void DataTable::setBadgeColumn(int column, const std::map<std::string, Fl_Color>& colors) {
    badgeCol_ = column;
    badgeColors_ = colors;
}

long DataTable::selectedId() {
    for (int row = 0; row < rows(); ++row) {
        if (row_selected(row) && row < static_cast<int>(ids_.size())) {
            return ids_[row];
        }
    }

    return -1;
}

void DataTable::selectById(long id) {
    for (int row = 0; row < static_cast<int>(ids_.size()); ++row) {
        if (ids_[row] == id) {
            select_row(row, 1);
            redraw();
            return;
        }
    }
}

void DataTable::draw() {
    Fl_Table_Row::draw();

    if (rows() == 0) {
        fl_color(Theme::textMuted);
        fl_font(Theme::font, 14);
        fl_draw("Здесь пока пусто — нажмите «+ Добавить»",
                x(),
                y() + col_header_height() + 40,
                w(),
                40,
                FL_ALIGN_CENTER);
    }
}

void DataTable::drawBadge(const std::string& text, int X, int Y, int W, int H) {
    auto it = badgeColors_.find(text);
    Fl_Color color = (it != badgeColors_.end()) ? it->second : Theme::textMuted;
    int badgeWidth = W - 24;
    int badgeHeight = 24;
    int badgeX = X + 12;
    int badgeY = Y + (H - badgeHeight) / 2;

    fl_draw_box(FL_RFLAT_BOX, badgeX, badgeY, badgeWidth, badgeHeight, color);
    fl_color(FL_WHITE);
    fl_font(Theme::fontBold, 11);
    fl_draw(text.c_str(), badgeX, badgeY, badgeWidth, badgeHeight, FL_ALIGN_CENTER);
}

void DataTable::draw_cell(TableContext context, int row, int column, int X, int Y, int W, int H) {
    switch (context) {
    case CONTEXT_COL_HEADER:
        fl_draw_box(FL_FLAT_BOX, X, Y, W, H, Theme::surface);
        fl_color(Theme::textMuted);
        fl_font(Theme::fontBold, 11);

        if (column >= 0 && column < static_cast<int>(headers_.size())) {
            fl_draw(headers_[column].c_str(), X + 16, Y, W - 32, H, FL_ALIGN_LEFT | FL_ALIGN_CLIP);
        }

        fl_color(Theme::border);
        fl_line(X, Y + H - 1, X + W - 1, Y + H - 1);
        return;

    case CONTEXT_CELL: {
        if (row < 0 || row >= static_cast<int>(rows_.size())) {
            return;
        }

        if (column < 0 || column >= static_cast<int>(rows_[row].size())) {
            return;
        }

        bool selected = row_selected(row);
        bool hovered = row == hoverRow_;
        Fl_Color background = selected ? Theme::rowSelected
                : hovered              ? Theme::rowHover
                                       : (row % 2 ? Theme::rowAlt : Theme::surface);

        fl_draw_box(FL_FLAT_BOX, X, Y, W, H, background);
        fl_color(Theme::border);
        fl_line(X, Y + H - 1, X + W - 1, Y + H - 1);

        const auto& text = rows_[row][column];

        if (column == badgeCol_) {
            drawBadge(text, X, Y, W, H);
        } else {
            fl_color(Theme::textPrimary);
            fl_font(Theme::font, 13);
            fl_draw(text.c_str(), X + 16, Y, W - 32, H, FL_ALIGN_LEFT | FL_ALIGN_CLIP);
        }

        return;
    }

    default:
        Fl_Table_Row::draw_cell(context, row, column, X, Y, W, H);
    }
}

int DataTable::handle(int event) {
    if (event == FL_PUSH && Fl::event_clicks() && Fl::event_button() == 1) {
        int row = -1;
        int column = -1;
        ResizeFlag resizeFlag;
        TableContext context = cursor2rowcol(row, column, resizeFlag);

        if (context == CONTEXT_CELL && row >= 0 && row < static_cast<int>(ids_.size()) && onActivate_) {
            onActivate_(ids_[row]);
        }
    }

    int result = Fl_Table_Row::handle(event);

    if (event == FL_PUSH && onSelect_) {
        onSelect_();
    }

    int row = -1;
    int column = -1;
    ResizeFlag resizeFlag;
    TableContext context = cursor2rowcol(row, column, resizeFlag);
    int newHover = context == CONTEXT_CELL ? row : -1;

    if (newHover != hoverRow_) {
        hoverRow_ = newHover;
    }

    if (event == FL_PUSH || event == FL_RELEASE || event == FL_DRAG) {
        redraw();
    }

    return result;
}
