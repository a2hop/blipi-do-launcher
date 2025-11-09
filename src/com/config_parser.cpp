#include "config_parser.hpp"
#include <iostream>

namespace BlipiDo {

ConfigParser::ConfigParser() {
}

ConfigParser::~ConfigParser() {
    xmlCleanupParser();
}

bool ConfigParser::load_xml(const char *filename) {
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        std::cerr << "Failed to parse " << filename << std::endl;
        return false;
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    xmlFreeDoc(doc);
    
    return (root != NULL);
}

void ConfigParser::parse_xml_menu(xmlNode *node, GtkTreeStore *tree_store, GtkTreeIter *parent) {
    for (xmlNode *cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(cur_node->name, (const xmlChar *)"Item") == 0) {
                xmlChar *label = xmlGetProp(cur_node, (const xmlChar *)"label");
                xmlChar *action = xmlGetProp(cur_node, (const xmlChar *)"action");
                xmlChar *options = xmlGetProp(cur_node, (const xmlChar *)"options");
                
                if (label) {
                    GtkTreeIter iter;
                    gtk_tree_store_append(tree_store, &iter, parent);
                    gtk_tree_store_set(tree_store, &iter, 0, (const char*)label, -1);
                    
                    if (action) {
                        ActionItem item;
                        item.label = (const char*)label;
                        item.action = (const char*)action;
                        item.options = options ? (const char*)options : "";
                        item.iter = iter;
                        action_items.push_back(item);
                        xmlFree(action);
                        if (options) {
                            xmlFree(options);
                        }
                    }
                    
                    // Recursively parse children
                    if (cur_node->children) {
                        parse_xml_menu(cur_node->children, tree_store, &iter);
                    }
                    
                    xmlFree(label);
                }
            }
        }
    }
}

std::string ConfigParser::get_action_for_iter(GtkTreeStore *tree_store, GtkTreeIter *iter) {
    for (const auto &item : action_items) {
        GtkTreePath *path1 = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), iter);
        GtkTreePath *path2 = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), 
                                                      const_cast<GtkTreeIter*>(&item.iter));
        
        if (gtk_tree_path_compare(path1, path2) == 0) {
            gtk_tree_path_free(path1);
            gtk_tree_path_free(path2);
            return item.action;
        }
        
        gtk_tree_path_free(path1);
        gtk_tree_path_free(path2);
    }
    return "";
}

std::string ConfigParser::get_options_for_iter(GtkTreeStore *tree_store, GtkTreeIter *iter) {
    for (const auto &item : action_items) {
        GtkTreePath *path1 = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), iter);
        GtkTreePath *path2 = gtk_tree_model_get_path(GTK_TREE_MODEL(tree_store), 
                                                      const_cast<GtkTreeIter*>(&item.iter));
        
        if (gtk_tree_path_compare(path1, path2) == 0) {
            gtk_tree_path_free(path1);
            gtk_tree_path_free(path2);
            return item.options;
        }
        
        gtk_tree_path_free(path1);
        gtk_tree_path_free(path2);
    }
    return "";
}

} // namespace BlipiDo
