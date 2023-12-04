#include "tabs_cpu.h"
#include <gtk/gtk.h>


void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, cpu_label);
}

