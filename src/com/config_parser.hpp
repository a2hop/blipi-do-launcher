#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace BlipiDo {

struct ActionItem {
    std::string label;
    std::string action;
    std::string options;
    GtkTreeIter iter;
};

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser();
    
    bool load_xml(const char *filename);
    void parse_xml_menu(xmlNode *node, GtkTreeStore *tree_store, GtkTreeIter *parent);
    const std::vector<ActionItem>& get_action_items() const { return action_items; }
    std::string get_action_for_iter(GtkTreeStore *tree_store, GtkTreeIter *iter);
    std::string get_options_for_iter(GtkTreeStore *tree_store, GtkTreeIter *iter);

private:
    std::vector<ActionItem> action_items;
};

} // namespace BlipiDo

#endif // CONFIG_PARSER_HPP
