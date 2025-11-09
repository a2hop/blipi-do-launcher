#include "window.hpp"
#include <iostream>
#include <unistd.h>

namespace BlipiDo {

Window::Window(int argc, char *argv[]) {
    // Store command line arguments (excluding program name)
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // Create window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "BliPi Do");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 400);
    
    // Load icon
    load_icon();
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), this);

    // Setup UI
    setup_ui();
    
    // Load actions from XML
    load_actions();
}

Window::~Window() {
    if (tree_store) {
        g_object_unref(tree_store);
    }
}

void Window::load_icon() {
    GError *error = NULL;
    GdkPixbuf *icon = gdk_pixbuf_new_from_resource("/com/blipi/do/icon.png", &error);
    if (icon) {
        gtk_window_set_icon(GTK_WINDOW(window), icon);
        g_object_unref(icon);
    } else {
        if (error) {
            std::cerr << "Failed to load icon: " << error->message << std::endl;
            g_error_free(error);
        }
    }
}

void Window::setup_ui() {
    // Create vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Create label
    GtkWidget *label = gtk_label_new("Select action:");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    // Create tree store and tree view
    tree_store = gtk_tree_store_new(1, G_TYPE_STRING);
    
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store));
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        "Actions", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    
    // Connect row activation (double-click or Enter key)
    g_signal_connect(treeview, "row-activated", G_CALLBACK(on_row_activated), this);

    // Add scrolled window for tree view
    GtkWidget *scrolled_tree = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_tree),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_tree), treeview);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_tree, TRUE, TRUE, 0);

    // Create text view for output
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_widget_set_size_request(text_view, -1, 75);

    GtkWidget *scrolled_text = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_text), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_text, FALSE, FALSE, 0);

    // Create button
    button = gtk_button_new_with_label("Select");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), this);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

void Window::load_actions() {
    // Read actions from /etc/blipi-do/actions.xml
    std::string xml_path = "/etc/blipi-do/actions.xml";
    
    // Load and parse XML
    xmlDoc *doc = xmlReadFile(xml_path.c_str(), NULL, 0);
    if (doc != NULL) {
        xmlNode *root = xmlDocGetRootElement(doc);
        if (root) {
            config_parser.parse_xml_menu(root->children, tree_store, NULL);
        }
        xmlFreeDoc(doc);
    } else {
        std::cerr << "Failed to load actions.xml from " << xml_path << std::endl;
    }
}

void Window::perform_action(const std::string &action, const std::string &options) {
    std::cout << "Performing action: " << action << std::endl;
    
    // Clear text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
    
    // Build command with arguments (files/directories)
    std::string command = action;
    for (const auto &arg : args) {
        // Escape quotes in the argument
        std::string escaped_arg = arg;
        size_t pos = 0;
        while ((pos = escaped_arg.find("\"", pos)) != std::string::npos) {
            escaped_arg.replace(pos, 1, "\\\"");
            pos += 2;
        }
        command += " \"" + escaped_arg + "\"";
    }
    
    // Parse and append options if they exist
    if (!options.empty()) {
        // Split options by semicolon
        size_t start = 0;
        size_t end = options.find(';');
        while (end != std::string::npos) {
            std::string option = options.substr(start, end - start);
            if (!option.empty()) {
                // Add -- prefix only if not already present
                if (option.substr(0, 2) != "--") {
                    command += " --" + option;
                } else {
                    command += " " + option;
                }
            }
            start = end + 1;
            end = options.find(';', start);
        }
        // Add the last option (or the only one if no semicolon)
        std::string option = options.substr(start);
        if (!option.empty()) {
            // Add -- prefix only if not already present
            if (option.substr(0, 2) != "--") {
                command += " --" + option;
            } else {
                command += " " + option;
            }
        }
    }
    
    // Display action info
    std::string text = "Executing: " + command + "\n\n";
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text.c_str(), -1);
    
    // Process pending events to update UI (button release, text view)
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    // Execute the action and capture output
    FILE *pipe = popen((command + " 2>&1").c_str(), "r");
    if (pipe) {
        char line[256];
        while (fgets(line, sizeof(line), pipe)) {
            gtk_text_buffer_get_end_iter(buffer, &end);
            gtk_text_buffer_insert(buffer, &end, line, -1);
            
            // Process pending GTK events to update UI
            while (gtk_events_pending()) {
                gtk_main_iteration();
            }
        }
        int status = pclose(pipe);
        int exit_code = WEXITSTATUS(status);
        
        // Display result
        std::string result_text = "\nExit code: " + std::to_string(exit_code);
        if (exit_code == 0) {
            result_text += " (Success)\n";
        } else {
            result_text += " (Failed)\n";
        }
        
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, result_text.c_str(), -1);
        
        // Close after a short delay if successful
        if (exit_code == 0) {
            g_timeout_add(500, (GSourceFunc)gtk_main_quit, NULL);
        }
    } else {
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, "Failed to execute command\n", -1);
    }
}

gboolean Window::perform_action_idle(gpointer data) {
    ActionData *action_data = static_cast<ActionData*>(data);
    action_data->window->perform_action(action_data->action, action_data->options);
    delete action_data;
    return FALSE; // Remove the timeout callback
}

void Window::on_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                              GtkTreeViewColumn *column, gpointer data) {
    Window *self = static_cast<Window*>(data);
    
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        std::string action = self->config_parser.get_action_for_iter(self->tree_store, &iter);
        if (!action.empty()) {
            std::string options = self->config_parser.get_options_for_iter(self->tree_store, &iter);
            // Schedule action to run after a short delay to let UI update
            ActionData *action_data = new ActionData{self, action, options};
            g_timeout_add(50, perform_action_idle, action_data);
        } else {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->text_view));
            gtk_text_buffer_set_text(buffer, "This item has no action defined", -1);
        }
    }
}

void Window::on_button_clicked(GtkWidget *widget, gpointer data) {
    Window *self = static_cast<Window*>(data);
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->treeview));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        std::string action = self->config_parser.get_action_for_iter(self->tree_store, &iter);
        if (!action.empty()) {
            std::string options = self->config_parser.get_options_for_iter(self->tree_store, &iter);
            // Schedule action to run after a short delay to let button release animation complete
            ActionData *action_data = new ActionData{self, action, options};
            g_timeout_add(200, perform_action_idle, action_data);
        } else {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->text_view));
            gtk_text_buffer_set_text(buffer, "Please select an item with an action defined", -1);
        }
    } else {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->text_view));
        gtk_text_buffer_set_text(buffer, "Please select an item first", -1);
    }
}

gboolean Window::on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return TRUE;
    }
    return FALSE;
}

void Window::show() {
    gtk_widget_show_all(window);
}

void Window::run() {
    gtk_main();
}

} // namespace BlipiDo
