#pragma once
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <functional>
#include <string>
#include <vector>

class EmployeeDialog : public Fl_Window {
public:
    using OnSave = std::function<void(const std::string& name,
        const std::string& email,
        bool active)>;
    EmployeeDialog(OnSave cb, long id = 0,
        const std::string& name = "",
        const std::string& email = "",
        bool active = true);

private:
    OnSave onSave_;
    long id_ = 0;
    Fl_Input *name_, *email_;
    Fl_Check_Button* activeChk_ = nullptr;
    static void okCb(Fl_Widget*, void*);
};

class AssetDialog : public Fl_Window {
public:
    using OnSave = std::function<void(const std::string& inv,
        const std::string& model,
        const std::string& category)>;
    AssetDialog(OnSave cb,
        const std::string& inv = "",
        const std::string& model = "",
        const std::string& category = "");

private:
    OnSave onSave_;
    Fl_Input *inv_, *model_;
    Fl_Choice* cat_;
    static void okCb(Fl_Widget*, void*);
};

class IssueDialog : public Fl_Window {
public:
    using OnSave = std::function<void(long employeeId, long assetId)>;
    IssueDialog(OnSave cb,
        const std::vector<std::pair<long, std::string>>& employees,
        const std::vector<std::pair<long, std::string>>& assets);

private:
    OnSave onSave_;
    std::vector<std::pair<long, std::string>> employees_, assets_;
    Fl_Choice *emp_, *asset_;
    static void okCb(Fl_Widget*, void*);
};
