#pragma once
#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include "ui/data_table.h"
#include "service.h"

class AssetsTab : public Fl_Group {
public:
    AssetsTab(int X, int Y, int W, int H, Service& svc);
    void reload();
    void resize(int X, int Y, int W, int H) override;

private:
    Service& svc_;
    Fl_Choice* filter_;
    Fl_Button *addBtn_, *editBtn_, *issueBtn_, *returnBtn_;
    DataTable* table_;
    std::vector<AssetRow> current_;

    void layout();
    void updateButtons();
    void doAdd();
    void doEdit(long id);
    void doIssue(long id);
    void doReturn(long id);

    static void filterCb(Fl_Widget*, void*);
    static void addCb(Fl_Widget*, void*);
    static void editCb(Fl_Widget*, void*);
    static void issueCb(Fl_Widget*, void*);
    static void returnCb(Fl_Widget*, void*);
};
