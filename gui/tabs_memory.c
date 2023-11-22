//
// Created by Param Patel on 11/22/23.
//

#include "tabs_memory.h"
#include <gtk/gtk.h>

void add_memory_tab(GtkWidget *notebook) {
    GtkWidget *memory_label = gtk_label_new("Memory");
    GtkWidget *memory_tab = gtk_label_new("This is the Memory tab content");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), memory_tab, memory_label);
}
