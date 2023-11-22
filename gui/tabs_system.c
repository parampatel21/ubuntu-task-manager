//
// Created by Param Patel on 11/22/23.
//

#include "tabs_system.h"
#include <gtk/gtk.h>

void add_system_tab(GtkWidget *notebook) {
    GtkWidget *system_label = gtk_label_new("System");
    GtkWidget *system_tab = gtk_label_new("This is the System tab content");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), system_tab, system_label);
}
