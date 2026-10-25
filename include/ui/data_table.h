#pragma once
#include <FL/Fl_Table_Row.H>
#include <string>
#include <vector>
#include <map>
#include <functional>

class DataTable : public Fl_Table_Row {
public:
    DataTable(int X, int Y, int W, int H);

    void setup(std::vector<std::string> headers, std::vector<int> weights);
    void setRows(const std::vector<std::vector<std::string>>& data,
        const std::vector<long>& ids);
    void setBadgeColumn(int col, const std::map<std::string, Fl_Color>& colors);
    long selectedId();
    void onActivate(std::function<void(long)> cb) {
        onActivate_ = std::move(cb);
    }

    void resize(int X, int Y, int W, int H) override;

protected:
    void draw() override;
    void draw_cell(TableContext ctx, int R, int C, int X, int Y, int W, int H) override;
    int handle(int e) override;

private:
    std::vector<std::string> headers_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<long> ids_;
    std::vector<int> weights_;
    int sumWeight_ = 0;

    int hoverRow_ = -1;
    int badgeCol_ = -1;
    std::map<std::string, Fl_Color> badgeColors_;
    std::function<void(long)> onActivate_;

    void applyWidths();
    void drawBadge(const std::string& text, int X, int Y, int W, int H);
};
