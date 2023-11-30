#include "tabs_cpu.h"
#include <gtk/gtk.h>


void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    int core_count = atoi(popen("nproc --all", "r"));

//    I want to get the output of this command every second: mpstat -P ALL | tail -n +4 | awk '{print 100 - $NF}'
//    Example output (first line is always total of all cores, there are core_count lines afterwards):
//    1.96
//    2.94
//    2.29
//    2.56
//    1.98
//    2.89
//    2.41
//    2.47
//    2.11

//  I should plot the line afterward on a GTK graph, adding new data in every single
//  time I get the output from the command above

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, cpu_label);
}

