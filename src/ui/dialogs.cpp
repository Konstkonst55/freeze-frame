#include "ui/dialogs.h"
#include "ui/theme.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cstring>

namespace {

    void header(Fl_Window* w, const char* title) {
        const int HDR = 56;
        auto h = new Fl_Box(0, 0, w->w(), HDR);
        h->box(FL_FLAT_BOX);
        h->color(Theme::surface);

        auto t = new Fl_Box(24, 0, w->w() - 48, HDR, title);
        t->box(FL_NO_BOX);
        t->labelcolor(Theme::textPrimary);
        t->labelfont(Theme::fontBold);
        t->labelsize(16);
        t->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        auto sep = new Fl_Box(0, HDR, w->w(), 1);
        sep->box(FL_FLAT_BOX);
        sep->color(Theme::border);
    }

    void footer(Fl_Window* w, Fl_Callback* ok, void* self) {
        const int H = 60;
        int y = w->h() - H;
        auto sep = new Fl_Box(0, y, w->w(), 1);
        sep->box(FL_FLAT_BOX);
        sep->color(Theme::border);

        auto cancel = Theme::secondary(
            w->w() - 24 - Theme::BTN_W - Theme::GAP - Theme::BTN_W,
            y + 11, Theme::BTN_W, Theme::BTN_H, "Отмена");
        cancel->callback([](Fl_Widget*, void* v) {
            static_cast<Fl_Window*>(v)->hide();
        }, w);

        auto okBtn = Theme::returnButton(
            w->w() - 24 - Theme::BTN_W,
            y + 11, Theme::BTN_W, Theme::BTN_H, "Сохранить");
        okBtn->callback(ok, self);
        w->end();
    }

    void lockSize(Fl_Window* w) {
        w->size_range(w->w(), w->h(), w->w(), w->h());
        w->resizable(nullptr);
    }

} // namespace

EmployeeDialog::EmployeeDialog(OnSave cb, long id,
    const std::string& name,
    const std::string& email,
    bool active)
    : Fl_Window(440, 380, id == 0 ? "Новый фотограф" : "Редактирование"),
      onSave_(std::move(cb)), id_(id) {
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
        activeChk_ = new Fl_Check_Button(x, y, w, 28,
            "В штате (снятие уволит фотографа)");
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

AssetDialog::AssetDialog(OnSave cb, const std::string& inv,
    const std::string& model, const std::string& category)
    : Fl_Window(440, 400, inv.empty() ? "Новая техника" : "Редактирование"),
      onSave_(std::move(cb)) {
    set_modal();
    color(Theme::bg);

    header(this, inv.empty() ? "Новая техника" : "Редактирование техники");

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
    d->onSave_(d->inv_->value(), d->model_->value(),
        d->cat_->text(d->cat_->value()));
    d->hide();
}

IssueDialog::IssueDialog(OnSave cb,
    const std::vector<std::pair<long, std::string>>& emps,
    const std::vector<std::pair<long, std::string>>& astr)
    : Fl_Window(480, 340, "Выдача техники"),
      onSave_(std::move(cb)), employees_(emps), assets_(astr) {
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
    d->onSave_(d->employees_[d->emp_->value()].first,
        d->assets_[d->asset_->value()].first);
    d->hide();
}
