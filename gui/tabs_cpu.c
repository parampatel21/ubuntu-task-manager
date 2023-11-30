#include "tabs_cpu.h"
#include <gtk/gtk.h>

int core_count = 0;

double *cpu_usage = NULL;

void update_cpu_usage() {
    FILE *mpstat_pipe = popen("mpstat -P ALL 1 1 | tail -n +4 | awk '{print 100 - $NF}'", "r");
    if (mpstat_pipe == NULL) {
        g_print("Error opening pipe!\n");
        return;
    }

    fscanf(mpstat_pipe, "%lf", &cpu_usage[0]);

    for (int i = 1; i <= core_count; i++) {
        fscanf(mpstat_pipe, "%lf", &cpu_usage[i]);
    }

    pclose(mpstat_pipe);
}

gboolean draw_cpu_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    int width, height;
    gtk_widget_get_size_request(widget, &width, &height);

    cairo_set_source_rgb(cr, 1, 1, 1);  // White color
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);  // Black color

    cairo_set_line_width(cr, 2.0);
    for (int i = 0; i <= core_count; i++) {
        double x = (double)i / core_count * width;
        double y = height - cpu_usage[i] * height / 100.0;
        if (i == 0) {
            cairo_move_to(cr, x, y);
        } else {
            cairo_line_to(cr, x, y);
        }
    }

    cairo_stroke(cr);

    return FALSE;
}

gboolean update_cpu_data(gpointer user_data) {
    update_cpu_usage();

    GtkWidget *drawing_area = GTK_WIDGET(user_data);
    gtk_widget_queue_draw(drawing_area);

    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);

    core_count = atoi(popen("nproc --all", "r"));

    cpu_usage = g_malloc((core_count + 1) * sizeof(double));

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 200);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_cpu_graph), NULL);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 0, 1, 1);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid, cpu_label);

    update_cpu_usage();

    g_timeout_add_seconds(1, update_cpu_data, drawing_area);
}