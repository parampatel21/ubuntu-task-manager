#include "tabs_cpu.h"
#include <stdlib.h>
#include <proc/readproc.h>

static gboolean update_graph(gpointer user_data) {
    cpu_info *data = (cpu_info *)user_data;

    // Read thread usage information from /proc
    PROCTAB *pt;
    proc_t proc_info;
    pt = openproc(PROC_FILLSTAT);

    int i = 0;
    while (readproc(pt, &proc_info) != NULL && i < MAX_THREADS) {
        data->thread_usage[i] = (double)proc_info.utime / ((double)proc_info.utime + (double)proc_info.stime);
        i++;
    }
    data->num_threads = i;
    closeproc(pt);

    gtk_widget_queue_draw(data->drawing_area);

    return TRUE;
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    cpu_info *data = (cpu_info *)user_data;

    int width, height;
    gtk_widget_get_size_request(widget, &width, &height);

    const GdkRGBA thread_colors[MAX_THREADS] = {
            {0.0, 0.0, 1.0, 1.0}, // Blue
            {1.0, 0.0, 0.0, 1.0}, // Red
            {0.0, 1.0, 0.0, 1.0}, // Green
    };

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0); // White
    cairo_paint(cr);

    cairo_set_line_width(cr, 2.0);

    for (int i = 0; i < data->num_threads; ++i) {
        cairo_set_source_rgba(cr, thread_colors[i].red, thread_colors[i].green, thread_colors[i].blue, thread_colors[i].alpha);

        double y = height * (1.0 - data->thread_usage[i]);

        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);

        cairo_stroke(cr);
    }

    return FALSE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    cpu_info *data = g_malloc(sizeof(cpu_info));
    data->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(data->drawing_area, 400, 300);
    gtk_box_pack_start(GTK_BOX(cpu_tab), data->drawing_area, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT(data->drawing_area), "draw", G_CALLBACK(on_draw), data);

    data->graph_update_timeout = g_timeout_add(1000, (GSourceFunc)update_graph, data);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);
}