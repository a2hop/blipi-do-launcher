#include <gtk/gtk.h>
#include "com/window.hpp"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    BlipiDo::Window window(argc, argv);
    window.show();
    window.run();
    
    return 0;
}
