#ifndef TABS_CPU_H
#define TABS_CPU_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *drawing_area;
    guint graph_update_timeout;
    double thread_usage[256];
    int num_threads;
} cpu_info;

void add_cpu_tab(GtkWidget *notebook);

#endif
