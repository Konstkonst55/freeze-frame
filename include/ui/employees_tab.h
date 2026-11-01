#pragma once
#include "service.h"
#include "ui/data_table.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

class EmployeesTab : public Fl_Group {
public:
    EmployeesTab(int X, int Y, int W, int H, Service& svc);
    void reload();
    void resize(int X, int Y, int W, int H) override;

private:
    Service& svc_;
    Fl_Box* searchLabel_;
    Fl_Input* search_;
    Fl_Button *addBtn_, *editBtn_;
    DataTable* table_;

    void layout();
    void doAdd();
    void doEdit(long id);

    static void searchCb(Fl_Widget*, void*);
    static void addCb(Fl_Widget*, void*);
    static void editCb(Fl_Widget*, void*);
};
