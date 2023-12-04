#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_MAX 1024
#define MAX_DATA_POINTS 60
#define KEY_MAX_WIDTH 400

typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for memory and swap
    double colors[2][3]; // Colors for memory and swap lines
} MemSwapData;

void generate_random_color(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

void update_chart(MemSwapData *data, int index, double new_value) {
    memmove(data->data[index], data->data[index] + 1, sizeof(double) * (MAX_DATA_POINTS - 1));
    data->data[index][MAX_DATA_POINTS - 1] = new_value;
    gtk_widget_queue_draw(data->drawing_area);
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    MemSwapData *data = (MemSwapData *)user_data;
    cairo_set_line_width(cr, 2);

    int width, height;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget) - 50;

    int padding = 70;
    width -= 2 * padding;
    height -= 2 * padding;

    double spacing = (double)width / MAX_DATA_POINTS;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, padding, padding);
    cairo_line_to(cr, padding, height + padding);
    cairo_line_to(cr, width + padding, height + padding);
    cairo_stroke(cr);

    for (int index = 0; index < 2; ++index) {
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);
        cairo_move_to(cr, padding, height + padding - (data->data[index][0] / 100.0) * height);
        for (int i = 1; i < MAX_DATA_POINTS; ++i) {
            cairo_line_to(cr, i * spacing + padding, height + padding - (data->data[index][i] / 100.0) * height);
        }
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);

    for (int i = 0; i <= MAX_DATA_POINTS; i += 10) {
        char label[4];
        snprintf(label, sizeof(label), "%d", MAX_DATA_POINTS - i);
        cairo_move_to(cr, i * spacing + padding - 5, height + padding + 20);
        cairo_show_text(cr, label);
    }

    for (int i = 0; i <= 100; i += 10) {
        double y = (100 - i) * height / 100 + padding;
        char label[4];
        snprintf(label, sizeof(label), "%d%%", i);
        cairo_move_to(cr, padding - 40, y + 5);
        cairo_show_text(cr, label);
    }

    cairo_set_font_size(cr, 14.0);
    int key_x = padding;
    int key_y = height + 2 * padding + 20;

    for (int index = 0; index < 2; ++index) {
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);
        cairo_move_to(cr, key_x, key_y);
        cairo_line_to(cr, key_x + 20, key_y);
        cairo_stroke(cr);

        char key_label[20];
        snprintf(key_label, sizeof(key_label), (index == 0) ? "Memory" : "Swap");
        cairo_move_to(cr, key_x + 25, key_y + 4);
        cairo_show_text(cr, key_label);

        key_x += 100;
        if (key_x > KEY_MAX_WIDTH) {
            key_x = padding;
            key_y += 20;
        }
    }

    return FALSE;
}

int read_mem_info(FILE *fp, unsigned long long int *mem_total, unsigned long long int *mem_free,
                  unsigned long long int *swap_total, unsigned long long int *swap_free) {
    char buffer[BUF_MAX];
    if (!fgets(buffer, BUF_MAX, fp)) {
        perror("Error reading /proc/meminfo");
        return -1;
    }

    sscanf(buffer, "MemTotal: %llu kB", mem_total);
    sscanf(buffer, "MemFree: %llu kB", mem_free);
    sscanf(buffer, "SwapTotal: %llu kB", swap_total);
    sscanf(buffer, "SwapFree: %llu kB", swap_free);

    return 0;
}

gboolean update_data(gpointer user_data) {
    MemSwapData *data = (MemSwapData *)user_data;
    unsigned long long int mem_total, mem_free, swap_total, swap_free;

    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("Error opening /proc/meminfo");
        return TRUE;
    }

    if (read_mem_info(fp, &mem_total, &mem_free, &swap_total, &swap_free) == 0) {
        double mem_usage = ((mem_total - mem_free) / (double)mem_total) * 100;
        double swap_usage = ((swap_total - swap_free) / (double)swap_total) * 100;

        update_chart(data, 0, mem_usage);
        update_chart(data, 1, swap_usage);

        printf("Memory Usage: %3.2lf%%\n", mem_usage);
        printf("Swap Usage: %3.2lf%%\n", swap_usage);
    }

    fclose(fp);
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Memory and Swap Usage Chart");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    MemSwapData data;
    data.drawing_area = drawing_area;

    // Assign colors
    data.colors[0][0] = 0.0; data.colors[0][1] = 0.0; data.colors[0][2] = 1.0; // Blue for Memory
    data.colors[1][0] = 1.0; data.colors[1][1] = 0.5; data.colors[1][2] = 0.0; // Orange for Swap

    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &data);

    g_timeout_add_seconds(1, update_data, &data);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
