//
// Created by Param Patel on 11/22/23.
//

#include "tabs_process.h"
#include <gtk/gtk.h>

void add_process_tab(GtkWidget *notebook) {
    GtkWidget *process_label = gtk_label_new("Process");
    GtkWidget *process_tab = gtk_label_new("This is the Process tab content");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_tab, process_label);
}
