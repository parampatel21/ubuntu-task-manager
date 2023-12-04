#ifndef TABS_CPU_H
#define TABS_CPU_H

#include <gtk/gtk.h>

#define BUF_MAX 1024
#define MAX_DATA_POINTS 60
#define KEY_MAX_WIDTH 400

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for memory and swap
    double colors[2][3]; // Colors for memory and swap lines
} AppData;

void add_cpu_tab(GtkWidget *notebook, AppData *data);
gboolean update_data(gpointer user_data);
int read_memory_swap(double *memory, double *swap);
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void update_chart(AppData *data, int index, double new_value);
void generate_random_color(double *color);

#endif
