#include "ui/employees_tab.h"
#include "ui/theme.h"
#include "ui/dialogs.h"
#include <FL/fl_ask.H>

namespace {
    template<typename F> void runSafely(F&& f) {
        try {
            f();
        } catch (std::exception& e) {
            fl_alert("Ошибка: %s", e.what());
        }
    }
}

EmployeesTab::EmployeesTab(int X, int Y, int W, int H, Service& svc)
    : Fl_Group(X, Y, W, H), svc_(svc) {
    box(FL_FLAT_BOX);
    color(Theme::bg);

    searchLabel_ = new Fl_Box(0, 0, 60, Theme::BTN_H, "Поиск:");
    searchLabel_->box(FL_NO_BOX);
    searchLabel_->labelcolor(Theme::textMuted);
    searchLabel_->labelfont(Theme::fontBold);
    searchLabel_->labelsize(12);
    searchLabel_->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

    search_ = Theme::input(0, 0, 100, Theme::BTN_H);
    search_->callback(searchCb, this);
    search_->when(FL_WHEN_CHANGED);

    addBtn_ = Theme::primary(0, 0, Theme::BTN_W, Theme::BTN_H, "+ Фотограф");
    addBtn_->callback(addCb, this);

    editBtn_ = Theme::secondary(0, 0, Theme::BTN_W, Theme::BTN_H, "Изменить");
    editBtn_->callback(editCb, this);

    table_ = new DataTable(0, 0, 100, 100);
    table_->setup({"ID", "ФИО", "Email", "Статус"}, {1, 5, 5, 2});
    table_->setBadgeColumn(3, {
        {"В штате", Theme::success},
        {"Уволен",  Theme::textMuted}
    });
    table_->onActivate([this](long id) {
        doEdit(id);
    });

    end();
    layout();
    reload();
}

void EmployeesTab::layout() {
    const int pad = Theme::PAD;
    const int gap = Theme::GAP;
    const int bh  = Theme::BTN_H;
    const int bw  = Theme::BTN_W;

    int rightEdge = x() + w() - pad;
    addBtn_->resize(rightEdge - bw, y() + pad, bw, bh);
    editBtn_->resize(rightEdge - bw - gap - bw, y() + pad, bw, bh);

    int searchX = x() + pad + 60 + 8;
    int searchW = editBtn_->x() - gap - searchX;
    if (searchW < 120) {
        searchW = 120;
    }
    searchLabel_->resize(x() + pad, y() + pad, 60, bh);
    search_->resize(searchX, y() + pad, searchW, bh);

    int tableY = y() + pad + bh + 16;
    int tableH = h() - (tableY - y()) - pad;
    if (tableH < 80) {
        tableH = 80;
    }
    table_->resize(x() + pad, tableY, w() - 2 * pad, tableH);
}

void EmployeesTab::resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X, Y, W, H);
    layout();
}

void EmployeesTab::reload() {
    runSafely([&] {
        std::string f = search_->value();
        std::vector<std::vector<std::string>> data;
        std::vector<long> ids;
        for (const auto& e : svc_.employees()) {
            if (!f.empty()) {
                auto lower = [](std::string s) {
                    for (auto& c : s) {
                        c = (char)tolower(c);
                    }
                    return s;
                };
                if (lower(e.fullName()).find(lower(f)) == std::string::npos &&
                lower(e.email()).find(lower(f)) == std::string::npos)
                continue;
            }
            data.push_back({
                std::to_string(e.id()),
                e.fullName(),
                e.email(),
                e.isActive() ? "В штате" : "Уволен"
            });
            ids.push_back(e.id());
        }
        table_->setRows(data, ids);
    });
}

void EmployeesTab::doAdd() {
    auto* d = new EmployeeDialog([this](auto name, auto email, auto) {
        runSafely([&] {
            svc_.addEmployee(name, email);
            reload();
        });
    });
    d->show();
    while (d->shown()) {
        Fl::wait();
    }
    delete d;
}

void EmployeesTab::doEdit(long id) {
    runSafely([&] {
        for (const auto& e : svc_.employees()) {
            if (e.id() != id) {
                continue;
            }
            auto* d = new EmployeeDialog(
            [this, id](auto name, auto email, auto active) {
                runSafely([&] {
                    svc_.updateEmployee(id, name, email, active);
                    reload();
                });
            },
            id, e.fullName(), e.email(), e.isActive());
            d->show();
            while (d->shown()) {
                Fl::wait();
            }
            delete d;
            return;
        }
    });
}

void EmployeesTab::searchCb(Fl_Widget*, void* v) {
    ((EmployeesTab*)v)->reload();
}
void EmployeesTab::addCb(Fl_Widget*, void* v) {
    ((EmployeesTab*)v)->doAdd();
}
void EmployeesTab::editCb(Fl_Widget*, void* v) {
    auto* t = (EmployeesTab*)v;
    long id = t->table_->selectedId();
    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }
    t->doEdit(id);
}
