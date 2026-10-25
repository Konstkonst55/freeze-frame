#include "ui/main_window.h"
#include "ui/theme.h"
#include "ui/employees_tab.h"
#include "ui/assets_tab.h"

MainWindow::MainWindow(Service& svc)
    : Fl_Double_Window(1200, 760, "Стоп-кадр — учёт техники студии"), svc_(svc) {
    color(Theme::bg);
    size_range(900, 600);

    header_ = new Fl_Box(0, 0, w(), Theme::HEADER);
    header_->box(FL_FLAT_BOX);
    header_->color(Theme::headerBg);

    brand_ = new Fl_Box(24, 0, 260, Theme::HEADER, "Стоп-кадр");
    brand_->box(FL_NO_BOX);
    brand_->labelcolor(FL_WHITE);
    brand_->labelfont(Theme::fontBold);
    brand_->labelsize(20);
    brand_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    tabEmp_ = new Fl_Button(0, 0, 160, 38, "Фотографы");
    tabEmp_->box(FL_RFLAT_BOX);
    tabEmp_->color(Theme::accent);
    tabEmp_->selection_color(Theme::accentHover);
    tabEmp_->labelcolor(FL_WHITE);
    tabEmp_->labelfont(Theme::fontBold);
    tabEmp_->labelsize(13);
    tabEmp_->callback(tabEmpCb, this);

    tabAst_ = new Fl_Button(0, 0, 160, 38, "Техника");
    tabAst_->box(FL_RFLAT_BOX);
    tabAst_->color(Theme::tabInactive);
    tabAst_->selection_color(Theme::accentHover);
    tabAst_->labelcolor(FL_WHITE);
    tabAst_->labelfont(Theme::fontBold);
    tabAst_->labelsize(13);
    tabAst_->callback(tabAstCb, this);

    int contentY = Theme::HEADER;
    int contentH = h() - Theme::HEADER - Theme::STATUS;

    empTab_ = new EmployeesTab(0, contentY, w(), contentH, svc_);
    astTab_ = new AssetsTab   (0, contentY, w(), contentH, svc_);
    astTab_->hide();

    status_ = new Fl_Box(0, h() - Theme::STATUS, w(), Theme::STATUS,
        "Готово   ·   PostgreSQL");
    status_->box(FL_FLAT_BOX);
    status_->color(Theme::surface);
    status_->labelcolor(Theme::textMuted);
    status_->labelfont(Theme::font);
    status_->labelsize(12);
    status_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    resizable(this);
    end();
    layout();
}

void MainWindow::layout() {
    const int H = Theme::HEADER;
    const int S = Theme::STATUS;

    header_->resize(0, 0, w(), H);
    brand_->resize(24, 0, 260, H);

    int tabY = (H - 38) / 2;
    tabEmp_->resize(300, tabY, 160, 38);
    tabAst_->resize(468, tabY, 160, 38);

    int contentY = H;
    int contentH = h() - H - S;
    empTab_->resize(0, contentY, w(), contentH);
    astTab_->resize(0, contentY, w(), contentH);

    status_->resize(0, h() - S, w(), S);
}

void MainWindow::resize(int X, int Y, int W, int H) {
    Fl_Double_Window::resize(X, Y, W, H);
    layout();
}

void MainWindow::switchTab(int idx) {
    if (idx == 0) {
        empTab_->show();
        astTab_->hide();
        tabEmp_->color(Theme::accent);
        tabAst_->color(Theme::tabInactive);
        empTab_->reload();
    } else {
        empTab_->hide();
        astTab_->show();
        tabAst_->color(Theme::accent);
        tabEmp_->color(Theme::tabInactive);
        astTab_->reload();
    }
    redraw();
}

void MainWindow::tabEmpCb(Fl_Widget*, void* v) {
    ((MainWindow*)v)->switchTab(0);
}
void MainWindow::tabAstCb(Fl_Widget*, void* v) {
    ((MainWindow*)v)->switchTab(1);
}
