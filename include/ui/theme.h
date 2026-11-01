#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Return_Button.H>

namespace Theme {
    inline const Fl_Color bg = fl_rgb_color(0xF5, 0xF6, 0xFA);
    inline const Fl_Color surface = fl_rgb_color(0xFF, 0xFF, 0xFF);
    inline const Fl_Color headerBg = fl_rgb_color(0x1E, 0x29, 0x3B);
    inline const Fl_Color accent = fl_rgb_color(0x3B, 0x82, 0xF6);
    inline const Fl_Color accentHover = fl_rgb_color(0x25, 0x63, 0xEB);
    inline const Fl_Color success = fl_rgb_color(0x10, 0xB9, 0x81);
    inline const Fl_Color warning = fl_rgb_color(0xF5, 0x9E, 0x0B);
    inline const Fl_Color danger = fl_rgb_color(0xEF, 0x44, 0x44);
    inline const Fl_Color textPrimary = fl_rgb_color(0x1F, 0x29, 0x37);
    inline const Fl_Color textMuted = fl_rgb_color(0x6B, 0x72, 0x80);
    inline const Fl_Color border = fl_rgb_color(0xE5, 0xE7, 0xEB);
    inline const Fl_Color rowAlt = fl_rgb_color(0xFA, 0xFB, 0xFD);
    inline const Fl_Color rowHover = fl_rgb_color(0xF0, 0xF5, 0xFF);
    inline const Fl_Color rowSelected = fl_rgb_color(0xDB, 0xEA, 0xFE);
    inline const Fl_Color tabInactive = fl_rgb_color(0x37, 0x47, 0x5F);

    inline const Fl_Font font = FL_HELVETICA;
    inline const Fl_Font fontBold = FL_HELVETICA_BOLD;

    constexpr int PAD = 24;
    constexpr int BTN_H = 40;
    constexpr int BTN_W = 120;
    constexpr int GAP = 10;
    constexpr int HEADER = 72;
    constexpr int STATUS = 32;

    Fl_Button* primary(int x, int y, int w, int h, const char* label);
    Fl_Button* secondary(int x, int y, int w, int h, const char* label);
    Fl_Button* dangerBtn(int x, int y, int w, int h, const char* label);

    Fl_Button* returnButton(int x, int y, int w, int h, const char* label);

    Fl_Input* input(int x, int y, int w, int h, const char* def = "");
    Fl_Box* fieldLabel(int x, int y, int w, const char* text);
}
