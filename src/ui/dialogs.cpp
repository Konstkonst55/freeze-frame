#include "ui/dialogs.h"
#include "ui/theme.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <utility>

namespace {

    void header(Fl_Window* w, const char* title) {
        const int HDR = 56;
        auto h = new Fl_Box(0, 0, w->w(), HDR);
        h->box(FL_FLAT_BOX);
        h->color(Theme::surface);

        auto t = new Fl_Box(24, 0, w->w() - 48, HDR);
        t->box(FL_NO_BOX);
        t->copy_label(title);
        t->labelcolor(Theme::textPrimary);
        t->labelfont(Theme::fontBold);
        t->labelsize(16);
        t->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        auto sep = new Fl_Box(0, HDR, w->w(), 1);
        sep->box(FL_FLAT_BOX);
        sep->color(Theme::border);
    }

    void footer(Fl_Window* w, Fl_Callback* ok, void* self, const char* okLabel = "Сохранить") {
        const int H = 60;
        int y = w->h() - H;
        auto sep = new Fl_Box(0, y, w->w(), 1);
        sep->box(FL_FLAT_BOX);
        sep->color(Theme::border);

        auto cancel = Theme::secondary(
                w->w() - 24 - Theme::BTN_W - Theme::GAP - Theme::BTN_W, y + 11, Theme::BTN_W, Theme::BTN_H, "Отмена");
        cancel->callback([](Fl_Widget*, void* v) { static_cast<Fl_Window*>(v)->hide(); }, w);

        auto okBtn = Theme::returnButton(w->w() - 24 - Theme::BTN_W, y + 11, Theme::BTN_W, Theme::BTN_H, okLabel);
        okBtn->callback(ok, self);
        w->end();
    }

    void lockSize(Fl_Window* w) {
        w->size_range(w->w(), w->h(), w->w(), w->h());
        w->resizable(nullptr);
    }

    std::string formatDate(std::time_t t) {
        if (t == 0) {
            return "—";
        }
        char buf[16];
        std::strftime(buf, sizeof(buf), "%d.%m.%Y", std::localtime(&t));
        return buf;
    }

} // namespace

EmployeeDialog::EmployeeDialog(OnSave cb, long id, const std::string& name, const std::string& email, bool active)
    : Fl_Window(440, 380, id == 0 ? "Новый фотограф" : "Редактирование"), onSave_(std::move(cb)), id_(id) {
    set_modal();
    color(Theme::bg);
    header(this, id == 0 ? "Новый фотограф" : "Редактирование фотографа");

    int x = 24, w = 440 - 48, y = 84;
    Theme::fieldLabel(x, y, w, "ФИО");
    y += 20;
    name_ = Theme::input(x, y, w, 40, name.c_str());
    y += 40 + 16;

    Theme::fieldLabel(x, y, w, "Email");
    y += 20;
    email_ = Theme::input(x, y, w, 40, email.c_str());
    y += 40 + 20;

    if (id != 0) {
        activeChk_ = new Fl_Check_Button(x, y, w, 28, "В штате (снятие уволит фотографа)");
        activeChk_->value(active ? 1 : 0);
        activeChk_->labelcolor(Theme::textPrimary);
        activeChk_->labelfont(Theme::font);
        activeChk_->labelsize(13);
    }

    footer(this, okCb, this);
    lockSize(this);
}

void EmployeeDialog::okCb(Fl_Widget*, void* v) {
    auto* d = static_cast<EmployeeDialog*>(v);
    if (!d->name_->value()[0]) {
        fl_alert("Введите ФИО");
        return;
    }
    if (!d->email_->value()[0] || !std::strchr(d->email_->value(), '@')) {
        fl_alert("Введите корректный email");
        return;
    }
    bool active = d->activeChk_ ? (d->activeChk_->value() == 1) : true;
    d->onSave_(d->name_->value(), d->email_->value(), active);
    d->hide();
}

AssetDialog::AssetDialog(
        OnSave cb, const std::string& inv, const std::string& model, const std::string& category, bool readOnlyDetails)
    : Fl_Window(440, 400, readOnlyDetails ? "Копирование техники" : (inv.empty() ? "Новая техника" : "Редактирование")),
      onSave_(std::move(cb)) {
    set_modal();
    color(Theme::bg);
    header(this, readOnlyDetails ? "Копирование техники" : (inv.empty() ? "Новая техника" : "Редактирование техники"));

    int x = 24, w = 440 - 48, y = 84;
    Theme::fieldLabel(x, y, w, "Инвентарный номер");
    y += 20;
    inv_ = Theme::input(x, y, w, 40, inv.c_str());
    y += 40 + 16;

    Theme::fieldLabel(x, y, w, "Модель");
    y += 20;
    model_ = Theme::input(x, y, w, 40, model.c_str());
    y += 40 + 16;

    Theme::fieldLabel(x, y, w, "Категория");
    y += 20;
    cat_ = new Fl_Choice(x, y, w, 40);
    cat_->add("Камера|Объектив|Вспышка|Штатив|Свет|Другое");
    cat_->value(0);
    if (!category.empty()) {
        for (int i = 0; i < cat_->size(); ++i) {
            if (category == cat_->text(i)) {
                cat_->value(i);
                break;
            }
        }
    }

    if (readOnlyDetails) {
        model_->deactivate();
        cat_->deactivate();
    }

    footer(this, okCb, this);
    lockSize(this);
}

void AssetDialog::okCb(Fl_Widget*, void* v) {
    auto* d = static_cast<AssetDialog*>(v);
    if (!d->inv_->value()[0]) {
        fl_alert("Введите инв. номер");
        return;
    }
    if (!d->model_->value()[0]) {
        fl_alert("Введите модель");
        return;
    }
    d->onSave_(d->inv_->value(), d->model_->value(), d->cat_->text(d->cat_->value()));
    d->hide();
}

IssueDialog::IssueDialog(
        OnSave cb,
        const std::vector<std::pair<long, std::string>>& emps,
        const std::vector<std::pair<long, std::string>>& astr)
    : Fl_Window(480, 340, "Выдача техники"), onSave_(std::move(cb)), employees_(emps), assets_(astr) {
    set_modal();
    color(Theme::bg);
    header(this, "Выдача техники");

    int x = 24, w = 480 - 48, y = 84;
    Theme::fieldLabel(x, y, w, "Фотограф");
    y += 20;
    emp_ = new Fl_Choice(x, y, w, 40);
    for (auto& e : employees_) {
        emp_->add(e.second.c_str());
    }
    if (emp_->size() > 0) {
        emp_->value(0);
    }
    y += 40 + 16;

    Theme::fieldLabel(x, y, w, "Техника");
    y += 20;
    asset_ = new Fl_Choice(x, y, w, 40);
    for (auto& a : assets_) {
        asset_->add(a.second.c_str());
    }
    if (asset_->size() > 0) {
        asset_->value(0);
    }

    footer(this, okCb, this);
    lockSize(this);
}

void IssueDialog::okCb(Fl_Widget*, void* v) {
    auto* d = static_cast<IssueDialog*>(v);
    if (d->emp_->size() == 0) {
        fl_alert("Нет фотографов в штате");
        return;
    }
    if (d->asset_->size() == 0) {
        fl_alert("Вся техника занята");
        return;
    }
    d->onSave_(d->employees_[d->emp_->value()].first, d->assets_[d->asset_->value()].first);
    d->hide();
}

BulkIssueDialog::BulkIssueDialog(
        OnSave cb,
        const std::vector<std::pair<long, std::string>>& emps,
        const std::vector<std::pair<long, std::string>>& astr)
    : Fl_Window(560, 540, "Массовая выдача"), onSave_(std::move(cb)), employees_(emps), assets_(astr) {
    set_modal();
    color(Theme::bg);
    header(this, "Массовая выдача техники");

    int x = 24, w = 560 - 48, y = 84;

    Theme::fieldLabel(x, y, w, "Фотограф");
    y += 20;
    emp_ = new Fl_Choice(x, y, w, 40);
    for (auto& e : employees_) {
        emp_->add(e.second.c_str());
    }
    if (emp_->size() > 0) {
        emp_->value(0);
    }
    y += 40 + 16;

    Theme::fieldLabel(x, y, w, "Техника (Ctrl+Click — выбрать несколько)");
    y += 20;
    list_ = new Fl_Multi_Browser(x, y, w, 260);
    for (auto& a : assets_) {
        list_->add(a.second.c_str());
    }
    list_->callback(listCb, this);
    y += 260 + 12;

    counter_ = new Fl_Box(x, y, w, 24);
    counter_->box(FL_NO_BOX);
    counter_->labelcolor(Theme::textMuted);
    counter_->labelfont(Theme::font);
    counter_->labelsize(13);
    counter_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    counter_->copy_label("Выбрано: 0 из 0");

    footer(this, okCb, this, "Выдать");
    lockSize(this);

    listCb(list_, this);
}

void BulkIssueDialog::listCb(Fl_Widget*, void* v) {
    auto* d = static_cast<BulkIssueDialog*>(v);
    int selected = 0;
    for (int i = 1; i <= d->list_->size(); ++i) {
        if (d->list_->selected(i)) {
            ++selected;
        }
    }

    char buf[64];
    std::snprintf(buf, sizeof(buf), "Выбрано: %d из %d", selected, d->list_->size());
    d->counter_->copy_label(buf);
    d->counter_->redraw();
}

void BulkIssueDialog::okCb(Fl_Widget*, void* v) {
    auto* d = static_cast<BulkIssueDialog*>(v);
    if (d->emp_->size() == 0) {
        fl_alert("Нет фотографов в штате");
        return;
    }

    std::vector<long> selectedIds;
    for (int i = 1; i <= d->list_->size(); ++i) {
        if (d->list_->selected(i)) {
            selectedIds.push_back(d->assets_[i - 1].first);
        }
    }

    if (selectedIds.empty()) {
        fl_alert("Выберите хотя бы одну единицу техники");
        return;
    }

    long empId = d->employees_[d->emp_->value()].first;
    d->onSave_(selectedIds, empId);
    d->hide();
}

SearchDialog::SearchDialog(Service& svc) : Fl_Window(900, 600, "Поиск техники"), svc_(svc) {
    set_modal();
    color(Theme::bg);
    header(this, "Поиск техники");

    int x = 24, w = 900 - 48, y = 76;

    Theme::fieldLabel(x, y, 150, "Категория");
    Theme::fieldLabel(x + 160, y, 200, "Только свободные");
    Theme::fieldLabel(x + 380, y, w - 380, "Модель или инв. номер");
    y += 20;

    category_ = new Fl_Choice(x, y, 150, 40);
    category_->add("Все|Камера|Объектив|Вспышка|Штатив|Свет|Другое");
    category_->value(0);

    onlyFree_ = new Fl_Check_Button(x + 160, y, 200, 40, "");
    onlyFree_->value(0);

    text_ = Theme::input(x + 380, y, w - 380 - 140, 40);

    searchBtn_ = Theme::primary(x + w - 130, y, 130, 40, "Найти");
    searchBtn_->callback(searchCb, this);
    y += 40 + 16;

    hint_ = new Fl_Box(x, y, w, 40);
    hint_->box(FL_FLAT_BOX);
    hint_->color(Theme::surface);
    hint_->labelcolor(Theme::textMuted);
    hint_->labelfont(Theme::font);
    hint_->labelsize(14);
    hint_->copy_label("Заполните фильтры и нажмите «Найти»");

    int tableY = y + 48;
    int tableH = h() - tableY - 76;
    table_ = new DataTable(x, tableY, w, tableH);
    table_->hide();

    auto closeBtn = Theme::secondary(w - 24 - 130, h() - 60, 130, 40, "Закрыть");
    closeBtn->callback([](Fl_Widget*, void* v) { static_cast<Fl_Window*>(v)->hide(); }, this);

    end();
    lockSize(this);
}

void SearchDialog::doSearch() {
    std::string cat = category_->value() == 0 ? "" : category_->text(category_->value());
    bool onlyFree = onlyFree_->value() == 1;
    std::string text = text_->value();

    std::vector<Asset> results;
    try {
        results = svc_.searchAssets(cat, onlyFree, text);
    } catch (const std::exception& e) {
        fl_alert("Ошибка: %s", e.what());
        return;
    }

    if (results.empty()) {
        hint_->copy_label("Ничего не найдено");
        hint_->show();
        table_->hide();
        redraw();
        return;
    }

    hint_->hide();
    table_->setup({"ID", "Инв.№", "Модель", "Категория", "Статус"}, {1, 2, 5, 3, 2});
    table_->setBadgeColumn(4, {{"В стойке", Theme::success}, {"На съёмке", Theme::accent}});

    std::vector<std::vector<std::string>> data;
    std::vector<long> ids;
    for (const auto& a : results) {
        data.push_back(
                {std::to_string(a.id()),
                 a.inventoryNumber(),
                 a.model(),
                 a.category(),
                 a.isAssigned() ? "На съёмке" : "В стойке"});
        ids.push_back(a.id());
    }
    table_->setRows(data, ids);
    table_->show();
    redraw();
}

void SearchDialog::searchCb(Fl_Widget*, void* v) {
    static_cast<SearchDialog*>(v)->doSearch();
}

HistoryDialog::HistoryDialog(const std::string& assetLabel, const std::vector<HistoryEntry>& history)
    : Fl_Window(700, 540, "История выдач") {
    set_modal();
    color(Theme::bg);
    header(this, "История выдач");

    int x = 24, y = 76, w = 700 - 48;

    auto label = new Fl_Box(x, y, w, 28);
    label->box(FL_NO_BOX);
    label->labelcolor(Theme::textPrimary);
    label->labelfont(Theme::fontBold);
    label->labelsize(14);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->copy_label(assetLabel.c_str());
    y += 36;

    auto table = new DataTable(x, y, w, h() - y - 76);
    table->setup({"Фотограф", "Выдано", "Возвращено", "Статус"}, {4, 2, 2, 2});
    table->setBadgeColumn(3, {{"Активна", Theme::accent}, {"Возвращена", Theme::textMuted}});

    std::vector<std::vector<std::string>> data;
    std::vector<long> ids;
    for (const auto& e : history) {
        data.push_back(
                {e.employeeName,
                 formatDate(e.assignment.issuedAt()),
                 formatDate(e.assignment.returnedAt()),
                 e.assignment.isActive() ? "Активна" : "Возвращена"});
        ids.push_back(e.assignment.id());
    }
    table->setRows(data, ids);

    auto closeBtn = Theme::secondary(w - 130, h() - 60, 130, 40, "Закрыть");
    closeBtn->callback([](Fl_Widget*, void* v) { static_cast<Fl_Window*>(v)->hide(); }, this);

    end();
    lockSize(this);
}

ReassignDialog::ReassignDialog(
        OnSave cb,
        const std::string& assetLabel,
        const std::string& currentEmployee,
        const std::vector<std::pair<long, std::string>>& employees)
    : Fl_Window(480, 300, "Смена фотографа"), onSave_(std::move(cb)), employees_(employees) {
    set_modal();
    color(Theme::bg);
    header(this, "Смена фотографа");

    int x = 24, w = 480 - 48, y = 84;

    auto label = new Fl_Box(x, y, w, 24);
    label->box(FL_NO_BOX);
    label->labelcolor(Theme::textPrimary);
    label->labelfont(Theme::fontBold);
    label->labelsize(14);
    label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    label->copy_label(assetLabel.c_str());
    y += 32;

    Theme::fieldLabel(x, y, w, "Сейчас у фотографа");
    y += 20;
    auto cur = new Fl_Box(x, y, w, 32);
    cur->box(FL_FLAT_BOX);
    cur->color(Theme::surface);
    cur->labelcolor(Theme::textMuted);
    cur->labelfont(Theme::font);
    cur->labelsize(13);
    cur->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    cur->copy_label(currentEmployee.c_str());
    y += 40;

    Theme::fieldLabel(x, y, w, "Передать фотографу");
    y += 20;
    emp_ = new Fl_Choice(x, y, w, 40);
    for (auto& e : employees_) {
        emp_->add(e.second.c_str());
    }
    if (emp_->size() > 0) {
        emp_->value(0);
    }

    footer(this, okCb, this, "Передать");
    lockSize(this);
}

void ReassignDialog::okCb(Fl_Widget*, void* v) {
    auto* d = static_cast<ReassignDialog*>(v);
    if (d->emp_->size() == 0) {
        fl_alert("Нет активных фотографов");
        return;
    }
    d->onSave_(d->employees_[d->emp_->value()].first);
    d->hide();
}
