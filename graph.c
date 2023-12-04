#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_MAX 1024
#define MAX_CPU 128
#define MAX_DATA_POINTS 60
#define KEY_MAX_WIDTH 800

// Function to generate a random color
void generate_random_color(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[MAX_CPU + 1][MAX_DATA_POINTS]; // Data for overall CPU and each core
    double colors[MAX_CPU + 1][3]; // Colors for each CPU line
    int cpu_count;
} AppData;

// Function to update the line chart with new data
void update_chart(AppData *data, int cpu_index, double new_value) {
    // Shift existing data to the left
    memmove(data->data[cpu_index], data->data[cpu_index] + 1, sizeof(double) * (MAX_DATA_POINTS - 1));

    // Add new value to the end
    data->data[cpu_index][MAX_DATA_POINTS - 1] = new_value;

    // Queue a redraw of the drawing area
    gtk_widget_queue_draw(data->drawing_area);
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData *data = (AppData *)user_data;

    // Set up drawing context
    cairo_set_line_width(cr, 2);

    // Calculate the width and height of the drawing area
    int width, height;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget) - 50; // Reserve space for the key

    // Add padding
    int padding = 70;
    width -= 2 * padding;
    height -= 2 * padding;

    // Calculate the spacing between data points
    double spacing = (double)width / MAX_DATA_POINTS;

    // Set color to black for axes
    cairo_set_source_rgb(cr, 0, 0, 0);

    // Draw the axes
    cairo_move_to(cr, padding, padding);
    cairo_line_to(cr, padding, height + padding);
    cairo_line_to(cr, width + padding, height + padding);
    cairo_stroke(cr);

    // Draw the line chart for overall CPU and each core
    for (int cpu = 0; cpu < data->cpu_count; ++cpu) {
        // Set the color for the current CPU
        cairo_set_source_rgb(cr, data->colors[cpu][0], data->colors[cpu][1], data->colors[cpu][2]);

        // Map the first data point to the graph's scale
        cairo_move_to(cr, padding, height + padding - (data->data[cpu][0] / 100.0) * height);
        for (int i = 1; i < MAX_DATA_POINTS; ++i) {
            // Map each subsequent data point to the graph's scale
            cairo_line_to(cr, i * spacing + padding, height + padding - (data->data[cpu][i] / 100.0) * height);
        }

        // Stroke the path
        cairo_stroke(cr);
    }

    // Set color to black for axis labels
    cairo_set_source_rgb(cr, 0, 0, 0);

    // Draw x-axis and y-axis labels
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);

    // Draw x-axis numbers
    for (int i = 0; i <= MAX_DATA_POINTS; i += 10) {
        char label[4];
        snprintf(label, sizeof(label), "%d", MAX_DATA_POINTS - i);
        cairo_move_to(cr, i * spacing + padding - 5, height + padding + 20);
        cairo_show_text(cr, label);
    }

    // Draw y-axis numbers
    for (int i = 0; i <= 100; i += 10) {
        double y = (100 - i) * height / 100 + padding;
        char label[4];
        snprintf(label, sizeof(label), "%d%%", i);
        cairo_move_to(cr, padding - 40, y + 5);
        cairo_show_text(cr, label);
    }

    // Draw the key for the CPU lines
    cairo_set_font_size(cr, 14.0);
    int key_x = padding;
    int key_y = height + 2 * padding + 20;
    int row = 0;
    for (int cpu = 0; cpu < data->cpu_count; ++cpu) {
        cairo_set_source_rgb(cr, data->colors[cpu][0], data->colors[cpu][1], data->colors[cpu][2]);

        // Draw a small line as a color indicator
        cairo_move_to(cr, key_x, key_y);
        cairo_line_to(cr, key_x + 20, key_y);
        cairo_stroke(cr);

        // Draw the label text
        char key_label[20];
        snprintf(key_label, sizeof(key_label), "CPU%d", cpu);
        cairo_move_to(cr, key_x + 25, key_y + 4);
        cairo_show_text(cr, key_label);

        // Update key_x and key_y for the next entry
        key_x += 100;
        if (key_x > KEY_MAX_WIDTH) {
            key_x = padding;
            key_y += 20; // Move to the next row
            row++;
        }
    }

    return FALSE;
}

int read_fields(FILE *fp, unsigned long long int *fields) {
    char buffer[BUF_MAX];
    if (!fgets(buffer, BUF_MAX, fp)) {
        perror("Error reading /proc/stat");
        return -1;
    }
    return sscanf(buffer, "cpu%*d %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
                  &fields[0], &fields[1], &fields[2], &fields[3],
                  &fields[4], &fields[5], &fields[6], &fields[7],
                  &fields[8], &fields[9]);
}

gboolean update_data(gpointer user_data) {
    AppData *data = (AppData *)user_data;
    unsigned long long int fields[10], total_tick, idle, del_total_tick, del_idle;
    double percent_usage;

    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return TRUE;
    }

    // Read the first line (total CPU usage)
    char buffer[BUF_MAX];
    if (!fgets(buffer, BUF_MAX, fp)) {
        perror("Error reading /proc/stat");
        fclose(fp);
        return TRUE;
    }

    for (int cpu = 0; cpu < data->cpu_count; ++cpu) {
        // Read the current CPU data
        if (read_fields(fp, fields) < 4) {
            fprintf(stderr, "Error reading CPU %d data\n", cpu);
            continue;
        }

        // Calculate CPU usage
        total_tick = 0;
        for (int i = 0; i < 10; i++) {
            total_tick += fields[i];
        }
        idle = fields[3]; // idle ticks index

        static unsigned long long int total_tick_old[MAX_CPU] = {0};
        static unsigned long long int idle_old[MAX_CPU] = {0};

        del_total_tick = total_tick - total_tick_old[cpu];
        del_idle = idle - idle_old[cpu];

        if (del_total_tick > 0) {
            percent_usage = ((del_total_tick - del_idle) / (double)del_total_tick) * 100;
        } else {
            percent_usage = 0.0;
        }

        printf("CPU%d Usage: %3.2lf%%\n", cpu, percent_usage);

        // Update the chart with the new value
        update_chart(data, cpu, percent_usage);

        total_tick_old[cpu] = total_tick;
        idle_old[cpu] = idle;
    }

    fclose(fp);
    return TRUE; // Continue calling this function
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CPU Usage Chart");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create the drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Set up the data structure
    AppData data;
    data.drawing_area = drawing_area;
    data.cpu_count = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of CPU cores + 1 for overall CPU

    // Assign colors
    data.colors[0][0] = 1.0; data.colors[0][1] = 0.0; data.colors[0][2] = 0.0; // Red for Overall CPU
    for (int cpu = 1; cpu < data.cpu_count; ++cpu) {
        if (cpu == 1) {
            data.colors[cpu][0] = 0.0; data.colors[cpu][1] = 1.0; data.colors[cpu][2] = 0.0; // Green for CPU0
        } else {
            generate_random_color(data.colors[cpu]); // Random colors for other CPUs
        }
    }

    // Connect the draw signal
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &data);

    // Set up a timer to update data every second
    g_timeout_add_seconds(1, update_data, &data);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}

