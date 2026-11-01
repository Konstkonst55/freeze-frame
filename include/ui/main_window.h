#pragma once
#include "service.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>

class EmployeesTab;
class AssetsTab;

class MainWindow : public Fl_Double_Window {
public:
    explicit MainWindow(Service& svc);
    void resize(int X, int Y, int W, int H) override;

private:
    Service& svc_;
    Fl_Box *header_, *brand_, *status_;
    Fl_Button *tabEmp_, *tabAst_;
    EmployeesTab* empTab_;
    AssetsTab* astTab_;

    void layout();
    void switchTab(int idx);
    static void tabEmpCb(Fl_Widget*, void*);
    static void tabAstCb(Fl_Widget*, void*);
};
