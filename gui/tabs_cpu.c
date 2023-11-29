#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#define MAX_THREADS 100

typedef struct {
    GtkWidget *drawing_area;
    guint graph_update_timeout;
    double thread_usage[MAX_THREADS];
    int num_threads;
} CPUData;

static gboolean update_graph(gpointer user_data) {
    CPUData *data = (CPUData *)user_data;

    FILE *stat_file = fopen("/proc/stat", "r");
    if (!stat_file) {
        g_error("Failed to open /proc/stat file");
        return FALSE;
    }

    char line[256];
    if (!fgets(line, sizeof(line), stat_file)) {
        g_error("Failed to read /proc/stat file");
        fclose(stat_file);
        return FALSE;
    }

    fclose(stat_file);

    unsigned long long total_time = 0;
    char *token = strtok(line, " ");
    while (token) {
        token = strtok(NULL, " ");
        if (token) {
            total_time += strtoull(token, NULL, 10);
        }
    }

    FILE *status_file = fopen("/proc/self/status", "r");
    if (!status_file) {
        g_error("Failed to open /proc/self/status file");
        return FALSE;
    }

    for (int i = 0; i < data->num_threads; ++i) {
        char thread_line[256];
        do {
            if (!fgets(thread_line, sizeof(thread_line), status_file)) {
                g_error("Failed to read /proc/self/status file");
                fclose(status_file);
                return FALSE;
            }
        } while (strncmp(thread_line, "Threads:", 8) != 0);

        int num_threads;
        if (sscanf(thread_line, "Threads: %d", &num_threads) != 1) {
            g_error("Failed to parse Threads line");
            fclose(status_file);
            return FALSE;
        }

        data->num_threads = num_threads;

        unsigned long long thread_time;
        if (fscanf(status_file, " %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %llu", &thread_time) != 1) {
            g_error("Failed to parse thread usage");
            fclose(status_file);
            return FALSE;
        }

        data->thread_usage[i] = (double)thread_time / total_time * 100.0;
    }

    fclose(status_file);

    gtk_widget_queue_draw(data->drawing_area);

    return TRUE;
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    CPUData *data = (CPUData *)user_data;

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

        double y = height * (1.0 - data->thread_usage[i] / 100.0);

        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);

        cairo_stroke(cr);
    }

    return FALSE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    CPUData *data = g_malloc(sizeof(CPUData));
    data->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(data->drawing_area, 400, 300);
    gtk_box_pack_start(GTK_BOX(cpu_tab), data->drawing_area, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT(data->drawing_area), "draw", G_CALLBACK(on_draw), data);

    data->graph_update_timeout = g_timeout_add(1000, (GSourceFunc)update_graph, data);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);
}

