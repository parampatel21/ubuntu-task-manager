#ifndef TABS_CPU_H
#define TABS_CPU_H

#include <gtk/gtk.h>

#define BUF_MAX 1024
#define MAX_DATA_POINTS 60
#define MAX_CPU 128
#define KEY_MAX_WIDTH 800

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for memory and swap
    double colors[2][3]; // Colors for memory and swap lines
    GtkWidget *memory_label;
    GtkWidget *swap_label;
    double total_memory;
    double total_swap;
} AppData;

typedef struct {
    GtkWidget *drawing_area;
    double data[MAX_CPU + 1][MAX_DATA_POINTS]; // Data for overall CPU and each core
    double colors[MAX_CPU + 1][3]; // Colors for each CPU line
    int cpu_count;
} AppData2;

typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for network receiving and sending
    double colors[2][3];             // Colors for network lines
    int first_update;
    float total_received;
    float total_sent;
} AppData3;

void add_cpu_tab(GtkWidget *notebook, AppData *data, AppData2 *data2, AppData3 *data3);
gboolean update_data(gpointer user_data);
int read_memory_swap(double *memory, double *swap);
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void update_chart(AppData *data, int index, double new_value);
void generate_random_color(double *color);

void generate_random_color2(double *color);
void update_chart2(AppData2 *data, int cpu_index, double new_value);
gboolean on_draw2(GtkWidget *widget, cairo_t *cr, gpointer user_data);
int read_fields(FILE *fp, unsigned long long int *fields);
gboolean update_data2(gpointer user_data);

void update_chart3(AppData3 *data, int index, double new_value);
void init_network_utilization(double *last_recv, double *last_send);
void update_total_received_sent(AppData3 *data);
gboolean on_draw3(GtkWidget *widget, cairo_t *cr, gpointer user_data);
int read_network_utilization(double *recv_speed, double *send_speed, double *last_recv, double *last_send);
gboolean update_data3(gpointer user_data);
void generate_random_color3(double *color);

#endif
