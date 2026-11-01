#include "ui/employees_tab.h"
#include "ui/dialogs.h"
#include "ui/theme.h"

#include <FL/fl_ask.H>

#include <cctype>
#include <exception>
#include <string>
#include <vector>

namespace {

    template <typename F>
    void runSafely(F&& function) {
        try {
            function();
        } catch (std::exception& error) {
            fl_alert("Ошибка: %s", error.what());
        }
    }

}

EmployeesTab::EmployeesTab(int X, int Y, int W, int H, Service& svc) : Fl_Group(X, Y, W, H), svc_(svc) {
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
    table_->setBadgeColumn(3, {{"В штате", Theme::success}, {"Уволен", Theme::textMuted}});
    table_->onActivate([this](long id) { doEdit(id); });

    end();
    layout();
    reload();
}

void EmployeesTab::layout() {
    const int pad = Theme::PAD;
    const int gap = Theme::GAP;
    const int buttonHeight = Theme::BTN_H;
    const int buttonWidth = Theme::BTN_W;

    int rightEdge = x() + w() - pad;
    addBtn_->resize(rightEdge - buttonWidth, y() + pad, buttonWidth, buttonHeight);
    editBtn_->resize(rightEdge - buttonWidth - gap - buttonWidth, y() + pad, buttonWidth, buttonHeight);

    int searchX = x() + pad + 60 + 8;
    int searchWidth = editBtn_->x() - gap - searchX;

    if (searchWidth < 120) {
        searchWidth = 120;
    }

    searchLabel_->resize(x() + pad, y() + pad, 60, buttonHeight);
    search_->resize(searchX, y() + pad, searchWidth, buttonHeight);

    int tableY = y() + pad + buttonHeight + 16;
    int tableHeight = h() - (tableY - y()) - pad;

    if (tableHeight < 80) {
        tableHeight = 80;
    }

    table_->resize(x() + pad, tableY, w() - 2 * pad, tableHeight);
}

void EmployeesTab::resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X, Y, W, H);
    layout();
}

void EmployeesTab::reload() {
    runSafely([&] {
        std::string filter = search_->value();
        std::vector<std::vector<std::string>> data;
        std::vector<long> ids;

        for (const auto& employee : svc_.employees()) {
            if (!filter.empty()) {
                auto lower = [](std::string value) {
                    for (auto& character : value) {
                        character = (char)tolower(character);
                    }

                    return value;
                };

                if (lower(employee.fullName()).find(lower(filter)) == std::string::npos
                    && lower(employee.email()).find(lower(filter)) == std::string::npos) {
                    continue;
                }
            }

            data.push_back(
                    {std::to_string(employee.id()),
                     employee.fullName(),
                     employee.email(),
                     employee.isActive() ? "В штате" : "Уволен"});
            ids.push_back(employee.id());
        }

        table_->setRows(data, ids);
    });
}

void EmployeesTab::doAdd() {
    auto* dialog = new EmployeeDialog([this](auto name, auto email, auto) {
        runSafely([&] {
            svc_.addEmployee(name, email);
            reload();
        });
    });

    dialog->show();

    while (dialog->shown()) {
        Fl::wait();
    }

    delete dialog;
}

void EmployeesTab::doEdit(long id) {
    runSafely([&] {
        for (const auto& employee : svc_.employees()) {
            if (employee.id() != id) {
                continue;
            }

            auto* dialog = new EmployeeDialog(
                    [this, id](auto name, auto email, auto active) {
                        runSafely([&] {
                            svc_.updateEmployee(id, name, email, active);
                            reload();
                        });
                    },
                    id,
                    employee.fullName(),
                    employee.email(),
                    employee.isActive());

            dialog->show();

            while (dialog->shown()) {
                Fl::wait();
            }

            delete dialog;
            return;
        }
    });
}

void EmployeesTab::searchCb(Fl_Widget*, void* v) {
    static_cast<EmployeesTab*>(v)->reload();
}

void EmployeesTab::addCb(Fl_Widget*, void* v) {
    static_cast<EmployeesTab*>(v)->doAdd();
}

void EmployeesTab::editCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<EmployeesTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }

    tab->doEdit(id);
}
