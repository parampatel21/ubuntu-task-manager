#include "tabs_cpu.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAX_THREADS 10
#define HISTORY_SIZE 60
#define UPDATE_INTERVAL 1000

GtkWidget *drawing_area;
gdouble thread_usage[MAX_THREADS][HISTORY_SIZE] = {0};
gint current_second = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

gboolean update_data(gpointer user_data) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return TRUE;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu", 3) == 0) {
            int cpu;
            unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            sscanf(line, "cpu %d %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                   &cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

            unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;
            unsigned long idle_time = idle + iowait;
            unsigned long total_diff, idle_diff;

            pthread_mutex_lock(&mutex);
            total_diff = total - thread_usage[cpu][current_second % HISTORY_SIZE];
            idle_diff = idle_time - thread_usage[cpu][current_second % HISTORY_SIZE];
            thread_usage[cpu][current_second % HISTORY_SIZE] = total;
            pthread_mutex_unlock(&mutex);

            gtk_widget_queue_draw(drawing_area);

            break;
        }
    }

    fclose(fp);
    current_second = (current_second + 1) % HISTORY_SIZE;

    return TRUE;
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    pthread_mutex_lock(&mutex);

    gint width = gtk_widget_get_allocated_width(widget);
    gint height = gtk_widget_get_allocated_height(widget);
    gdouble x_scale = (gdouble)width / HISTORY_SIZE;
    gdouble y_scale = (gdouble)height / 100.0;

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2.0);

    for (int thread = 0; thread < MAX_THREADS; ++thread) {
        for (int i = 0; i < HISTORY_SIZE - 1; ++i) {
            gdouble x1 = i * x_scale;
            gdouble y1 = height - thread_usage[thread][i] * y_scale;
            gdouble x2 = (i + 1) * x_scale;
            gdouble y2 = height - thread_usage[thread][i + 1] * y_scale;

            cairo_move_to(cr, x1, y1);
            cairo_line_to(cr, x2, y2);
        }
    }

    cairo_stroke(cr);
    pthread_mutex_unlock(&mutex);

    return FALSE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 300);
    gtk_container_add(GTK_CONTAINER(cpu_tab), drawing_area);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    g_timeout_add(UPDATE_INTERVAL, update_data, NULL);
}
