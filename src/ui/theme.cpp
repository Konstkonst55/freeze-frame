#include "ui/theme.h"

namespace Theme {

    Fl_Button* primary(int x, int y, int w, int h, const char* label) {
        auto b = new Fl_Button(x, y, w, h, label);
        b->box(FL_RFLAT_BOX);
        b->down_box(FL_RFLAT_BOX);
        b->color(accent);
        b->selection_color(accentHover);
        b->labelcolor(FL_WHITE);
        b->labelfont(fontBold);
        b->labelsize(13);
        return b;
    }

    Fl_Button* secondary(int x, int y, int w, int h, const char* label) {
        auto b = new Fl_Button(x, y, w, h, label);
        b->box(FL_RFLAT_BOX);
        b->down_box(FL_RFLAT_BOX);
        b->color(surface);
        b->selection_color(rowSelected);
        b->labelcolor(textPrimary);
        b->labelfont(font);
        b->labelsize(13);
        return b;
    }

    Fl_Button* dangerBtn(int x, int y, int w, int h, const char* label) {
        auto b = secondary(x, y, w, h, label);
        b->labelcolor(danger);
        return b;
    }

    Fl_Button* returnButton(int x, int y, int w, int h, const char* label) {
        auto b = new Fl_Return_Button(x, y, w, h, label);
        b->box(FL_RFLAT_BOX);
        b->down_box(FL_RFLAT_BOX);
        b->color(accent);
        b->selection_color(accentHover);
        b->labelcolor(FL_WHITE);
        b->labelfont(fontBold);
        b->labelsize(13);
        return b;
    }

    Fl_Input* input(int x, int y, int w, int h, const char* def) {
        auto i = new Fl_Input(x, y, w, h);
        i->box(FL_DOWN_BOX);
        i->color(surface);
        i->selection_color(rowSelected);
        i->textcolor(textPrimary);
        i->textfont(font);
        i->textsize(13);
        if (def && *def) {
            i->value(def);
        }
        return i;
    }

    Fl_Box* fieldLabel(int x, int y, int w, const char* text) {
        auto b = new Fl_Box(x, y, w, 16);
        b->box(FL_NO_BOX);
        b->labelcolor(textMuted);
        b->labelfont(fontBold);
        b->labelsize(11);
        b->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        b->label(text);
        return b;
    }

}
