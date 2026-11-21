// Antoine Bernadat, Timothy Biersohn
#include <gtkmm/application.h>
#include "simulation.h"
#include "gui.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        Gui::set_file("");  // Initialise un monde vide
    }
    else {
        Gui::set_file(argv[1]); // Charge le monde initial
    }

    auto app = Gtk::Application::create();

    return app->make_window_and_run<Gui>(1, argv);
}