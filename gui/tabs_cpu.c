//
// Created by Param Patel on 11/22/23.
//

#include "tabs_cpu.h"
#include <gtk/gtk.h>

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_label_new("This is the CPU tab content");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);
}
