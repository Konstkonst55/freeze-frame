#include "ui/assets_tab.h"
#include "ui/dialogs.h"
#include "ui/theme.h"

#include <FL/fl_ask.H>

#include <algorithm>
#include <cstdio>
#include <exception>
#include <string>
#include <utility>
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

AssetsTab::AssetsTab(int X, int Y, int W, int H, Service& svc) : Fl_Group(X, Y, W, H), svc_(svc) {
    box(FL_FLAT_BOX);
    color(Theme::bg);

    filter_ = new Fl_Choice(0, 0, 140, Theme::BTN_H);
    filter_->add("Все|В стойке|На съёмке");
    filter_->value(0);
    filter_->callback(filterCb, this);

    searchBtn_ = Theme::secondary(0, 0, 100, Theme::BTN_H, "Поиск");
    historyBtn_ = Theme::secondary(0, 0, 100, Theme::BTN_H, "История");
    searchBtn_->callback(searchCb, this);
    historyBtn_->callback(historyCb, this);

    prevPageBtn_ = Theme::secondary(0, 0, 40, Theme::BTN_H, "◀");
    nextPageBtn_ = Theme::secondary(0, 0, 40, Theme::BTN_H, "▶");
    prevPageBtn_->callback(prevCb, this);
    nextPageBtn_->callback(nextCb, this);

    pageLabel_ = new Fl_Box(0, 0, 80, Theme::BTN_H);
    pageLabel_->box(FL_NO_BOX);
    pageLabel_->labelcolor(Theme::textMuted);
    pageLabel_->labelfont(Theme::font);
    pageLabel_->labelsize(12);
    pageLabel_->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
    pageLabel_->copy_label("Стр. 1");

    addBtn_ = Theme::primary(0, 0, 120, Theme::BTN_H, "+ Техника");
    editBtn_ = Theme::secondary(0, 0, 120, Theme::BTN_H, "Изменить");
    copyBtn_ = Theme::secondary(0, 0, 120, Theme::BTN_H, "Копировать");
    deleteBtn_ = Theme::dangerBtn(0, 0, 100, Theme::BTN_H, "Удалить");
    addBtn_->callback(addCb, this);
    editBtn_->callback(editCb, this);
    copyBtn_->callback(copyCb, this);
    deleteBtn_->callback(deleteCb, this);

    issueBtn_ = Theme::primary(0, 0, 110, Theme::BTN_H, "Выдать");
    bulkIssueBtn_ = Theme::secondary(0, 0, 160, Theme::BTN_H, "Выдать несколько");
    reassignBtn_ = Theme::secondary(0, 0, 120, Theme::BTN_H, "Сменить");
    returnBtn_ = Theme::secondary(0, 0, 110, Theme::BTN_H, "Принять");
    issueBtn_->callback(issueCb, this);
    bulkIssueBtn_->callback(bulkIssueCb, this);
    reassignBtn_->callback(reassignCb, this);
    returnBtn_->callback(returnCb, this);

    table_ = new DataTable(0, 0, 100, 100);
    table_->setup({"ID", "Инв.№", "Модель", "Категория", "Статус", "У кого"}, {1, 2, 5, 3, 2, 4});
    table_->setBadgeColumn(4, {{"В стойке", Theme::success}, {"На съёмке", Theme::accent}});
    table_->onActivate([this](long id) { doEdit(id); });
    table_->onSelect([this] { updateButtons(); });

    end();
    layout();
    reload();
}

void AssetsTab::layout() {
    const int pad = Theme::PAD;
    const int gap = Theme::GAP;
    const int buttonHeight = Theme::BTN_H;

    int xPosition = x() + pad;
    int yPosition = y() + pad;

    filter_->resize(xPosition, yPosition, 140, buttonHeight);
    xPosition += 140 + gap;
    searchBtn_->resize(xPosition, yPosition, 100, buttonHeight);
    xPosition += 100 + gap;
    historyBtn_->resize(xPosition, yPosition, 100, buttonHeight);
    xPosition += 100 + gap;
    prevPageBtn_->resize(xPosition, yPosition, 40, buttonHeight);
    xPosition += 40 + 4;
    pageLabel_->resize(xPosition, yPosition, 80, buttonHeight);
    xPosition += 80 + 4;
    nextPageBtn_->resize(xPosition, yPosition, 40, buttonHeight);

    yPosition += buttonHeight + 12;
    int rightEdge = x() + w() - 2 * pad;
    deleteBtn_->resize(rightEdge - deleteBtn_->w(), yPosition, deleteBtn_->w(), buttonHeight);
    copyBtn_->resize(deleteBtn_->x() - gap - copyBtn_->w(), yPosition, copyBtn_->w(), buttonHeight);
    editBtn_->resize(copyBtn_->x() - gap - editBtn_->w(), yPosition, editBtn_->w(), buttonHeight);
    addBtn_->resize(editBtn_->x() - gap - addBtn_->w(), yPosition, addBtn_->w(), buttonHeight);

    yPosition += buttonHeight + 12;
    xPosition = x() + pad;
    issueBtn_->resize(xPosition, yPosition, issueBtn_->w(), buttonHeight);
    xPosition += issueBtn_->w() + gap;
    bulkIssueBtn_->resize(xPosition, yPosition, bulkIssueBtn_->w(), buttonHeight);
    xPosition += bulkIssueBtn_->w() + gap;
    reassignBtn_->resize(xPosition, yPosition, reassignBtn_->w(), buttonHeight);
    xPosition += reassignBtn_->w() + gap;
    returnBtn_->resize(xPosition, yPosition, returnBtn_->w(), buttonHeight);

    int tableY = yPosition + buttonHeight + 16;
    int tableHeight = h() - (tableY - y()) - pad;

    if (tableHeight < 80) {
        tableHeight = 80;
    }

    table_->resize(x() + pad, tableY, w() - 2 * pad, tableHeight);
}

void AssetsTab::resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X, Y, W, H);
    layout();
}

void AssetsTab::reload() {
    runSafely([&] {
        long keepId = table_->selectedId();
        std::string filter = filter_->text(filter_->value());
        auto paged = svc_.listAssetsPaged(currentPage_, PAGE_SIZE, filter);

        if (paged.items.empty() && currentPage_ > 0) {
            currentPage_ = 0;
            paged = svc_.listAssetsPaged(0, PAGE_SIZE, filter);
        }

        hasNext_ = paged.hasNext;
        current_ = paged.items;

        std::vector<std::vector<std::string>> data;
        std::vector<long> ids;

        for (const auto& row : current_) {
            bool assigned = row.asset.isAssigned();
            data.push_back(
                    {std::to_string(row.asset.id()),
                     row.asset.inventoryNumber(),
                     row.asset.model(),
                     row.asset.category(),
                     assigned ? "На съёмке" : "В стойке",
                     row.holder.empty() ? "—" : row.holder});
            ids.push_back(row.asset.id());
        }

        table_->setRows(data, ids);

        if (keepId > 0) {
            table_->selectById(keepId);
        }

        updateButtons();
        updatePagination();
    });
}

void AssetsTab::updateButtons() {
    long id = table_->selectedId();
    auto it = std::find_if(
            current_.begin(), current_.end(), [id](const AssetRow& row) { return row.asset.id() == id; });

    if (it == current_.end()) {
        issueBtn_->deactivate();
        returnBtn_->deactivate();
        reassignBtn_->deactivate();
        return;
    }

    bool assigned = it->asset.isAssigned();

    if (assigned) {
        issueBtn_->deactivate();
        returnBtn_->activate();
        reassignBtn_->activate();
    } else {
        issueBtn_->activate();
        returnBtn_->deactivate();
        reassignBtn_->deactivate();
    }
}

void AssetsTab::updatePagination() {
    std::string text = "Стр. " + std::to_string(currentPage_ + 1);
    pageLabel_->copy_label(text.c_str());
    pageLabel_->redraw();

    if (currentPage_ == 0) {
        prevPageBtn_->deactivate();
    } else {
        prevPageBtn_->activate();
    }

    if (!hasNext_) {
        nextPageBtn_->deactivate();
    } else {
        nextPageBtn_->activate();
    }
}

void AssetsTab::doAdd() {
    auto* dialog = new AssetDialog([this](auto inventoryNumber, auto model, auto category) {
        runSafely([&] {
            svc_.addAsset(inventoryNumber, model, category);
            currentPage_ = 0;
            reload();
        });
    });

    dialog->show();

    while (dialog->shown()) {
        Fl::wait();
    }

    delete dialog;
}

void AssetsTab::doEdit(long id) {
    runSafely([&] {
        for (const auto& row : current_) {
            if (row.asset.id() != id) {
                continue;
            }

            auto* dialog = new AssetDialog(
                    [this](auto, auto, auto) {}, row.asset.inventoryNumber(), row.asset.model(), row.asset.category());

            dialog->show();

            while (dialog->shown()) {
                Fl::wait();
            }

            delete dialog;
            return;
        }
    });
}

void AssetsTab::doCopy(long id) {
    runSafely([&] {
        for (const auto& row : current_) {
            if (row.asset.id() != id) {
                continue;
            }

            auto* dialog = new AssetDialog(
                    [this, id](auto newInventoryNumber, auto, auto) {
                        runSafely([&] {
                            svc_.duplicateAsset(id, newInventoryNumber);
                            currentPage_ = 0;
                            reload();
                        });
                    },
                    "",
                    row.asset.model(),
                    row.asset.category(),
                    true);

            dialog->show();

            while (dialog->shown()) {
                Fl::wait();
            }

            delete dialog;
            return;
        }
    });
}

void AssetsTab::doDelete(long id) {
    if (fl_choice("Удалить технику и всю историю её выдач?", "Отмена", "Удалить", nullptr) != 1) {
        return;
    }

    runSafely([&] {
        svc_.deleteAssetCascade(id);
        reload();
    });
}

void AssetsTab::doIssue(long) {
    runSafely([&] {
        auto employees = svc_.activeEmployees();
        auto freeAssets = svc_.freeAssets();
        std::vector<std::pair<long, std::string>> employeeItems;
        std::vector<std::pair<long, std::string>> assetItems;

        for (const auto& employee : employees) {
            employeeItems.push_back({employee.id(), employee.fullName()});
        }

        for (const auto& asset : freeAssets) {
            assetItems.push_back({asset.id(), asset.inventoryNumber() + " — " + asset.model()});
        }

        auto* dialog = new IssueDialog(
                [this](long employeeId, long assetId) {
                    runSafely([&] {
                        svc_.issue(assetId, employeeId);
                        reload();
                    });
                },
                employeeItems,
                assetItems);

        dialog->show();

        while (dialog->shown()) {
            Fl::wait();
        }

        delete dialog;
    });
}

void AssetsTab::doBulkIssue() {
    runSafely([&] {
        auto employees = svc_.activeEmployees();
        auto freeAssets = svc_.freeAssets();
        std::vector<std::pair<long, std::string>> employeeItems;
        std::vector<std::pair<long, std::string>> assetItems;

        for (const auto& employee : employees) {
            employeeItems.push_back({employee.id(), employee.fullName()});
        }

        for (const auto& asset : freeAssets) {
            assetItems.push_back(
                    {asset.id(), asset.inventoryNumber() + " — " + asset.model() + " (" + asset.category() + ")"});
        }

        auto* dialog = new BulkIssueDialog(
                [this](const std::vector<long>& ids, long employeeId) {
                    runSafely([&] {
                        svc_.issueMultiple(ids, employeeId);
                        reload();
                    });
                },
                employeeItems,
                assetItems);

        dialog->show();

        while (dialog->shown()) {
            Fl::wait();
        }

        delete dialog;
    });
}

void AssetsTab::doReassign(long id) {
    runSafely([&] {
        for (const auto& row : current_) {
            if (row.asset.id() != id) {
                continue;
            }

            if (!row.asset.isAssigned()) {
                fl_alert("Техника не выдана");
                return;
            }

            auto employees = svc_.activeEmployees();
            std::vector<std::pair<long, std::string>> employeeItems;

            for (const auto& employee : employees) {
                employeeItems.push_back({employee.id(), employee.fullName()});
            }

            std::string label = row.asset.inventoryNumber() + " — " + row.asset.model();
            auto* dialog = new ReassignDialog(
                    [this, id](long newEmployeeId) {
                        runSafely([&] {
                            svc_.reassignAsset(id, newEmployeeId);
                            reload();
                        });
                    },
                    label,
                    row.holder,
                    employeeItems);

            dialog->show();

            while (dialog->shown()) {
                Fl::wait();
            }

            delete dialog;
            return;
        }
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

void AssetsTab::doSearch() {
    auto* dialog = new SearchDialog(svc_);
    dialog->callback([](Fl_Widget* widget, void*) { widget->hide(); });
    dialog->show();
}

void AssetsTab::doHistory(long id) {
    runSafely([&] {
        for (const auto& row : current_) {
            if (row.asset.id() != id) {
                continue;
            }

            auto history = svc_.assetHistory(id);
            std::string label = row.asset.inventoryNumber() + " — " + row.asset.model();
            auto* dialog = new HistoryDialog(label, history);
            dialog->callback([](Fl_Widget* widget, void*) {
                widget->hide();
                delete widget;
            });
            dialog->show();
            return;
        }
    });
}

void AssetsTab::filterCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    tab->currentPage_ = 0;
    tab->reload();
}

void AssetsTab::searchCb(Fl_Widget*, void* v) {
    static_cast<AssetsTab*>(v)->doSearch();
}

void AssetsTab::historyCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }

    tab->doHistory(id);
}

void AssetsTab::prevCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);

    if (tab->currentPage_ > 0) {
        tab->currentPage_--;
        tab->reload();
    }
}

void AssetsTab::nextCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);

    if (tab->hasNext_) {
        tab->currentPage_++;
        tab->reload();
    }
}

void AssetsTab::addCb(Fl_Widget*, void* v) {
    static_cast<AssetsTab*>(v)->doAdd();
}

void AssetsTab::editCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }

    tab->doEdit(id);
}

void AssetsTab::copyCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }

    tab->doCopy(id);
}

void AssetsTab::deleteCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите строку");
        return;
    }

    tab->doDelete(id);
}

void AssetsTab::issueCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите технику");
        return;
    }

    tab->doIssue(id);
}

void AssetsTab::bulkIssueCb(Fl_Widget*, void* v) {
    static_cast<AssetsTab*>(v)->doBulkIssue();
}

void AssetsTab::reassignCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите технику");
        return;
    }

    tab->doReassign(id);
}

void AssetsTab::returnCb(Fl_Widget*, void* v) {
    auto* tab = static_cast<AssetsTab*>(v);
    long id = tab->table_->selectedId();

    if (id < 0) {
        fl_alert("Выберите технику");
        return;
    }

    tab->doReturn(id);
}
