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

void DataTable::setup(std::vector<std::string> h, std::vector<int> weights) {
    headers_ = std::move(h);
    weights_ = std::move(weights);
    sumWeight_ = 0;
    for (int wgt : weights_) {
        sumWeight_ += wgt;
    }
    if (sumWeight_ <= 0) {
        sumWeight_ = 1;
    }

    cols((int)headers_.size());
    applyWidths();
    end();
}

void DataTable::applyWidths() {
    if (sumWeight_ <= 0 || weights_.empty()) {
        return;
    }

    int avail = w() - 4;
    if (avail < (int)weights_.size() * 40) {
        avail = (int)weights_.size() * 40;
    }

    int used = 0;
    for (int i = 0; i < (int)weights_.size(); ++i) {
        int cw;
        if (i == (int)weights_.size() - 1) {
            cw = avail - used;
        } else {
            cw = (int)((long long)avail * weights_[i] / sumWeight_);
        }
        if (cw < 40) {
            cw = 40;
        }
        col_width(i, cw);
        used += cw;
    }
}

void DataTable::resize(int X, int Y, int W, int H) {
    Fl_Table_Row::resize(X, Y, W, H);
    applyWidths();
    redraw();
}

void DataTable::setRows(const std::vector<std::vector<std::string>>& data,
const std::vector<long>& ids) {
    rows_ = data;
    ids_  = ids;
    rows((int)rows_.size());
    redraw();
}

void DataTable::setBadgeColumn(int col, const std::map<std::string, Fl_Color>& c) {
    badgeCol_ = col;
    badgeColors_ = c;
}

long DataTable::selectedId() {
    for (int r = 0; r < rows(); ++r) {
        if (row_selected(r) && r < (int)ids_.size()) {
            return ids_[r];
        }
    }
    return -1;
}

void DataTable::draw() {
    Fl_Table_Row::draw();
    if (rows() == 0) {
        fl_color(Theme::textMuted);
        fl_font(Theme::font, 14);
        fl_draw("Здесь пока пусто — нажмите «+ Добавить»",
            x(), y() + col_header_height() + 40, w(), 40, FL_ALIGN_CENTER);
    }
}

void DataTable::drawBadge(const std::string& text, int X, int Y, int W, int H) {
    auto it = badgeColors_.find(text);
    Fl_Color c = (it != badgeColors_.end()) ? it->second : Theme::textMuted;
    int bw = W - 24, bh = 24;
    int bx = X + 12, by = Y + (H - bh) / 2;
    fl_draw_box(FL_RFLAT_BOX, bx, by, bw, bh, c);
    fl_color(FL_WHITE);
    fl_font(Theme::fontBold, 11);
    fl_draw(text.c_str(), bx, by, bw, bh, FL_ALIGN_CENTER);
}

void DataTable::draw_cell(TableContext ctx, int R, int C, int X, int Y, int W, int H) {
    switch (ctx) {
    case CONTEXT_COL_HEADER:
        fl_draw_box(FL_FLAT_BOX, X, Y, W, H, Theme::surface);
        fl_color(Theme::textMuted);
        fl_font(Theme::fontBold, 11);
        if (C >= 0 && C < (int)headers_.size()) {
            fl_draw(headers_[C].c_str(), X + 16, Y, W - 32, H,
                FL_ALIGN_LEFT | FL_ALIGN_CLIP);
        }
        fl_color(Theme::border);
        fl_line(X, Y + H - 1, X + W - 1, Y + H - 1);
        return;

    case CONTEXT_CELL: {
        if (R < 0 || R >= (int)rows_.size()) {
            return;
        }
        if (C < 0 || C >= (int)rows_[R].size()) {
            return;
        }

        bool sel = row_selected(R);
        bool hov = (R == hoverRow_);
        Fl_Color bg = sel ? Theme::rowSelected
        : hov ? Theme::rowHover
        : (R % 2 ? Theme::rowAlt : Theme::surface);

        fl_draw_box(FL_FLAT_BOX, X, Y, W, H, bg);
        fl_color(Theme::border);
        fl_line(X, Y + H - 1, X + W - 1, Y + H - 1);

        const auto& txt = rows_[R][C];
        if (C == badgeCol_) {
            drawBadge(txt, X, Y, W, H);
        } else {
            fl_color(Theme::textPrimary);
            fl_font(Theme::font, 13);
            fl_draw(txt.c_str(), X + 16, Y, W - 32, H,
                FL_ALIGN_LEFT | FL_ALIGN_CLIP);
        }
        return;
    }
    default:
        Fl_Table_Row::draw_cell(ctx, R, C, X, Y, W, H);
    }
}

int DataTable::handle(int e) {
    if (e == FL_PUSH && Fl::event_clicks() && Fl::event_button() == 1) {
        int R = -1, C = -1;
        ResizeFlag rf;
        TableContext cx = cursor2rowcol(R, C, rf);
        if (cx == CONTEXT_CELL && R >= 0 && R < (int)ids_.size() && onActivate_) {
            onActivate_(ids_[R]);
        }
    }
    int ret = Fl_Table_Row::handle(e);
    int R = -1, C = -1;
    ResizeFlag rf;
    TableContext cx = cursor2rowcol(R, C, rf);
    int newHover = (cx == CONTEXT_CELL) ? R : -1;
    if (newHover != hoverRow_) {
        hoverRow_ = newHover;
        redraw();
    }
    return ret;
}
