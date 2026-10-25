#include "ui/assets_tab.h"
#include "ui/theme.h"
#include "ui/dialogs.h"
#include <FL/fl_ask.H>
#include <algorithm>

namespace {
    template<typename F> void runSafely(F&& f) {
        try {
            f();
        } catch (std::exception& e) {
            fl_alert("Ошибка: %s", e.what());
        }
    }
}

AssetsTab::AssetsTab(int X, int Y, int W, int H, Service& svc)
    : Fl_Group(X, Y, W, H), svc_(svc) {
    box(FL_FLAT_BOX);
    color(Theme::bg);

    filter_ = new Fl_Choice(0, 0, 160, Theme::BTN_H);
    filter_->add("Все|В стойке|На съёмке");
    filter_->value(0);
    filter_->callback(filterCb, this);

    issueBtn_  = Theme::primary(0, 0, Theme::BTN_W, Theme::BTN_H, "Выдать");
    returnBtn_ = Theme::secondary(0, 0, Theme::BTN_W, Theme::BTN_H, "Принять");
    issueBtn_->callback(issueCb, this);
    returnBtn_->callback(returnCb, this);

    addBtn_  = Theme::primary(0, 0, Theme::BTN_W, Theme::BTN_H, "+ Техника");
    editBtn_ = Theme::secondary(0, 0, Theme::BTN_W, Theme::BTN_H, "Изменить");
    addBtn_->callback(addCb, this);
    editBtn_->callback(editCb, this);

    table_ = new DataTable(0, 0, 100, 100);
    table_->setup({"ID", "Инв.№", "Модель", "Категория", "Статус", "У кого"},
                  {1, 2, 5, 3, 2, 4});
    table_->setBadgeColumn(4, {
        {"В стойке",  Theme::success},
        {"На съёмке", Theme::accent}
    });
    table_->onActivate([this](long id) {
        doEdit(id);
    });

    end();
    layout();
    reload();
}

void AssetsTab::layout() {
    const int pad = Theme::PAD;
    const int gap = Theme::GAP;
    const int bh  = Theme::BTN_H;
    const int bw  = Theme::BTN_W;

    int rightEdge = x() + w() - pad;
    addBtn_->resize(rightEdge - bw, y() + pad, bw, bh);
    editBtn_->resize(rightEdge - bw - gap - bw, y() + pad, bw, bh);

    int actionY = y() + pad + bh + 12;
    int actionX = x() + pad;
    issueBtn_->resize(actionX, actionY, bw, bh);
    returnBtn_->resize(actionX + bw + gap, actionY, bw, bh);

    filter_->resize(x() + pad, y() + pad, 160, bh);

    int tableY = actionY + bh + 16;
    int tableH = h() - (tableY - y()) - pad;
    if (tableH < 80) {
        tableH = 80;
    }
    table_->resize(x() + pad, tableY, w() - 2 * pad, tableH);
}

void AssetsTab::resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X, Y, W, H);
    layout();
}

void AssetsTab::reload() {
    runSafely([&] {
        current_ = svc_.assets();
        std::string f = filter_->text(filter_->value());
        std::vector<std::vector<std::string>> data;
        std::vector<long> ids;
        for (const auto& r : current_) {
            bool assigned = r.asset.isAssigned();
            if (f == "В стойке"  && assigned) {
                continue;
            }
            if (f == "На съёмке" && !assigned) {
                continue;
            }
            data.push_back({
                std::to_string(r.asset.id()),
                r.asset.inventoryNumber(),
                r.asset.model(),
                r.asset.category(),
                assigned ? "На съёмке" : "В стойке",
                r.holder.empty() ? "—" : r.holder
            });
            ids.push_back(r.asset.id());
        }
        table_->setRows(data, ids);
        updateButtons();
    });
}

void AssetsTab::updateButtons() {
    long id = table_->selectedId();
    auto it = std::find_if(current_.begin(), current_.end(),
        [id](const AssetRow& r) {
            return r.asset.id() == id;
        });
    bool assigned = (it != current_.end()) && it->asset.isAssigned();
    if (assigned) {
        issueBtn_->deactivate();
        returnBtn_->activate();
    } else {
        issueBtn_->activate();
        returnBtn_->deactivate();
    }
}

void AssetsTab::doAdd() {
    auto* d = new AssetDialog([this](auto i, auto m, auto c) {
        runSafely([&] {
            svc_.addAsset(i, m, c);
            reload();
        });
    });
    d->show();
    while (d->shown()) {
        Fl::wait();
    }
    delete d;
}

void AssetsTab::doEdit(long id) {
    runSafely([&] {
        for (const auto& r : current_) {
            if (r.asset.id() != id) {
                continue;
            }
            auto* d = new AssetDialog(
                [this](auto, auto, auto) {},
                r.asset.inventoryNumber(), r.asset.model(), r.asset.category());
            d->show();
            while (d->shown()) {
                Fl::wait();
            }
            delete d;
            return;
        }
    });
}

void AssetsTab::doIssue(long id) {
    runSafely([&] {
        auto emps = svc_.activeEmployees();
        auto free = svc_.freeAssets();
        std::vector<std::pair<long, std::string>> e, a;
        for (const auto& emp : emps) {
            e.push_back({emp.id(), emp.fullName()});
        }
        for (const auto& ast : free) {
            a.push_back({ast.id(), ast.inventoryNumber() + " — " + ast.model()});
        }
        auto* d = new IssueDialog([this](long eid, long aid) {
            runSafely([&] {
                svc_.issue(aid, eid);
                reload();
            });
        }, e, a);
        d->show();
        while (d->shown()) {
            Fl::wait();
        }
        delete d;
    });
}

void AssetsTab::doReturn(long id) {
    if (fl_choice("Принять технику обратно в стойку?", "Отмена", "Принять", nullptr) != 1) {
        return;
    }
    runSafely([&] {
        svc_.giveBack(id);
        reload();
    });
}

void AssetsTab::filterCb(Fl_Widget*, void* v) {
    ((AssetsTab*)v)->reload();
}
void AssetsTab::addCb(Fl_Widget*, void* v) {
    ((AssetsTab*)v)->doAdd();
}
void AssetsTab::editCb(Fl_Widget*, void* v) {
    auto* t = (AssetsTab*)v;
    long id = t->table_->selectedId();
    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }
    t->doEdit(id);
}
void AssetsTab::issueCb(Fl_Widget*, void* v) {
    auto* t = (AssetsTab*)v;
    long id = t->table_->selectedId();
    if (id < 0) {
        fl_alert("Выберите технику");
        return;
    }
    t->doIssue(id);
}
void AssetsTab::returnCb(Fl_Widget*, void* v) {
    auto* t = (AssetsTab*)v;
    long id = t->table_->selectedId();
    if (id < 0) {
        fl_alert("Выберите технику");
        return;
    }
    t->doReturn(id);
}
