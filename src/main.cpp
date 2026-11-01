#include "service.h"
#include "ui/main_window.h"
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <cstdlib>
#include <exception>
#include <odb/pgsql/database.hxx>
#include <string>

int main(int argc, char** argv) {
    try {
        auto env = [](const char* key, const char* def) {
            const char* v = std::getenv(key);
            return std::string(v ? v : def);
        };

        odb::pgsql::database db(
                env("EQDB_NAME", "equipment_tracker"),
                env("EQDB_USER", "postgres"),
                env("EQDB_PASS", "postgres"),
                env("EQDB_HOST", "localhost"),
                5432);

        Service svc(db);

        auto* win = new MainWindow(svc);
        win->callback([](Fl_Widget* w, void*) { w->hide(); });
        win->show(argc, argv);
        int ret = Fl::run();
        delete win;
        return ret;
    } catch (std::exception& e) {
        fl_message("Не удалось запустить:\n%s", e.what());
        return 1;
    }
}
