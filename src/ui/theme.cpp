#include "ui/theme.h"

namespace Theme {

    Fl_Button* primary(int x, int y, int w, int h, const char* label) {
        auto button = new Fl_Button(x, y, w, h, label);
        button->box(FL_RFLAT_BOX);
        button->down_box(FL_RFLAT_BOX);
        button->color(accent);
        button->selection_color(accentHover);
        button->labelcolor(FL_WHITE);
        button->labelfont(fontBold);
        button->labelsize(13);
        return button;
    }

    Fl_Button* secondary(int x, int y, int w, int h, const char* label) {
        auto button = new Fl_Button(x, y, w, h, label);
        button->box(FL_RFLAT_BOX);
        button->down_box(FL_RFLAT_BOX);
        button->color(surface);
        button->selection_color(rowSelected);
        button->labelcolor(textPrimary);
        button->labelfont(font);
        button->labelsize(13);
        return button;
    }

    Fl_Button* dangerBtn(int x, int y, int w, int h, const char* label) {
        auto button = secondary(x, y, w, h, label);
        button->labelcolor(danger);
        return button;
    }

    Fl_Button* returnButton(int x, int y, int w, int h, const char* label) {
        auto button = new Fl_Return_Button(x, y, w, h, label);
        button->box(FL_RFLAT_BOX);
        button->down_box(FL_RFLAT_BOX);
        button->color(accent);
        button->selection_color(accentHover);
        button->labelcolor(FL_WHITE);
        button->labelfont(fontBold);
        button->labelsize(13);
        return button;
    }

    Fl_Input* input(int x, int y, int w, int h, const char* def) {
        auto widget = new Fl_Input(x, y, w, h);
        widget->box(FL_DOWN_BOX);
        widget->color(surface);
        widget->selection_color(rowSelected);
        widget->textcolor(textPrimary);
        widget->textfont(font);
        widget->textsize(13);

        if (def && *def) {
            widget->value(def);
        }

        return widget;
    }

    Fl_Box* fieldLabel(int x, int y, int w, const char* text) {
        auto box = new Fl_Box(x, y, w, 16);
        box->box(FL_NO_BOX);
        box->labelcolor(textMuted);
        box->labelfont(fontBold);
        box->labelsize(11);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        box->label(text);
        return box;
    }

}
