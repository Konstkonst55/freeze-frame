#pragma once
#include "service.h"
#include "ui/data_table.h"
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Window.H>
#include <functional>
#include <string>
#include <vector>

class EmployeeDialog : public Fl_Window {
public:
    using OnSave = std::function<void(const std::string& name, const std::string& email, bool active)>;
    EmployeeDialog(
            OnSave cb, long id = 0, const std::string& name = "", const std::string& email = "", bool active = true);

private:
    OnSave onSave_;
    long id_ = 0;
    Fl_Input *name_, *email_;
    Fl_Check_Button* activeChk_ = nullptr;
    static void okCb(Fl_Widget*, void*);
};

class AssetDialog : public Fl_Window {
public:
    using OnSave = std::function<void(const std::string& inv, const std::string& model, const std::string& category)>;
    AssetDialog(
            OnSave cb,
            const std::string& inv = "",
            const std::string& model = "",
            const std::string& category = "",
            bool readOnlyDetails = false);

private:
    OnSave onSave_;
    Fl_Input *inv_, *model_;
    Fl_Choice* cat_;
    static void okCb(Fl_Widget*, void*);
};

class IssueDialog : public Fl_Window {
public:
    using OnSave = std::function<void(long employeeId, long assetId)>;
    IssueDialog(
            OnSave cb,
            const std::vector<std::pair<long, std::string>>& employees,
            const std::vector<std::pair<long, std::string>>& assets);

private:
    OnSave onSave_;
    std::vector<std::pair<long, std::string>> employees_, assets_;
    Fl_Choice *emp_, *asset_;
    static void okCb(Fl_Widget*, void*);
};

class BulkIssueDialog : public Fl_Window {
public:
    using OnSave = std::function<void(const std::vector<long>&, long)>;
    BulkIssueDialog(
            OnSave cb,
            const std::vector<std::pair<long, std::string>>& employees,
            const std::vector<std::pair<long, std::string>>& assets);

private:
    OnSave onSave_;
    std::vector<std::pair<long, std::string>> employees_, assets_;
    Fl_Choice* emp_;
    Fl_Multi_Browser* list_;
    Fl_Box* counter_;
    static void okCb(Fl_Widget*, void*);
    static void listCb(Fl_Widget*, void*);
};

class SearchDialog : public Fl_Window {
public:
    explicit SearchDialog(Service& svc);

private:
    Service& svc_;
    Fl_Choice* category_;
    Fl_Check_Button* onlyFree_;
    Fl_Input* text_;
    Fl_Button* searchBtn_;
    DataTable* table_;
    Fl_Box* hint_;
    void doSearch();
    static void searchCb(Fl_Widget*, void*);
};

class HistoryDialog : public Fl_Window {
public:
    HistoryDialog(const std::string& assetLabel, const std::vector<HistoryEntry>& history);
};

class ReassignDialog : public Fl_Window {
public:
    using OnSave = std::function<void(long)>;
    ReassignDialog(
            OnSave cb,
            const std::string& assetLabel,
            const std::string& currentEmployee,
            const std::vector<std::pair<long, std::string>>& employees);

private:
    OnSave onSave_;
    std::vector<std::pair<long, std::string>> employees_;
    Fl_Choice* emp_;
    static void okCb(Fl_Widget*, void*);
};
