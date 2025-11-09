#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "config_parser.hpp"

namespace BlipiDo {

class Window {
public:
    Window(int argc, char *argv[]);
    ~Window();
    
    void show();
    void run();

private:
    GtkWidget *window;
    GtkWidget *treeview;
    GtkWidget *text_view;
    GtkWidget *button;
    GtkTreeStore *tree_store;
    ConfigParser config_parser;
    std::vector<std::string> args;

    void setup_ui();
    void load_icon();
    void load_actions();
    void perform_action(const std::string &action, const std::string &options);
    
    struct ActionData {
        Window *window;
        std::string action;
        std::string options;
    };
    static gboolean perform_action_idle(gpointer data);
    
    // Static callbacks
    static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                                  GtkTreeViewColumn *column, gpointer data);
    static void on_button_clicked(GtkWidget *widget, gpointer data);
    static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
};

} // namespace BlipiDo

#endif // WINDOW_HPP
