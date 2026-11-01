#pragma once
#include "service.h"
#include "ui/data_table.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>

class AssetsTab : public Fl_Group {
public:
    AssetsTab(int X, int Y, int W, int H, Service& svc);
    void reload();
    void resize(int X, int Y, int W, int H) override;

private:
    Service& svc_;
    Fl_Choice* filter_;
    Fl_Button *searchBtn_, *historyBtn_;
    Fl_Button *prevPageBtn_, *nextPageBtn_;
    Fl_Box* pageLabel_;
    Fl_Button *addBtn_, *editBtn_, *copyBtn_, *deleteBtn_;
    Fl_Button *issueBtn_, *bulkIssueBtn_, *reassignBtn_, *returnBtn_;
    DataTable* table_;
    std::vector<AssetRow> current_;

    int currentPage_ = 0;
    bool hasNext_ = false;
    static constexpr int PAGE_SIZE = 20;

    void layout();
    void updateButtons();
    void updatePagination();
    void doAdd();
    void doEdit(long id);
    void doCopy(long id);
    void doDelete(long id);
    void doIssue(long id);
    void doBulkIssue();
    void doReassign(long id);
    void doReturn(long id);
    void doSearch();
    void doHistory(long id);

    static void filterCb(Fl_Widget*, void*);
    static void searchCb(Fl_Widget*, void*);
    static void historyCb(Fl_Widget*, void*);
    static void prevCb(Fl_Widget*, void*);
    static void nextCb(Fl_Widget*, void*);
    static void addCb(Fl_Widget*, void*);
    static void editCb(Fl_Widget*, void*);
    static void copyCb(Fl_Widget*, void*);
    static void deleteCb(Fl_Widget*, void*);
    static void issueCb(Fl_Widget*, void*);
    static void bulkIssueCb(Fl_Widget*, void*);
    static void reassignCb(Fl_Widget*, void*);
    static void returnCb(Fl_Widget*, void*);
};
