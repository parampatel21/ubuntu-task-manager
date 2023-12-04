#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_MAX 1024
#define MAX_DATA_POINTS 60
#define KEY_MAX_WIDTH 400

// Function to generate a random color
void generate_random_color(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for memory and swap
    double colors[2][3]; // Colors for memory and swap lines
} AppData;

// Function to update the line chart with new data
void update_chart(AppData *data, int index, double new_value) {
    // Shift existing data to the left
    memmove(data->data[index], data->data[index] + 1, sizeof(double) * (MAX_DATA_POINTS - 1));

    // Add new value to the end
    data->data[index][MAX_DATA_POINTS - 1] = new_value;

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

    // Draw the line chart for memory and swap
    for (int index = 0; index < 2; ++index) {
        // Set the color for the current line
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);

        // Map the first data point to the graph's scale
        cairo_move_to(cr, padding, height + padding - (data->data[index][0] / 100.0) * height);
        for (int i = 1; i < MAX_DATA_POINTS; ++i) {
            // Map each subsequent data point to the graph's scale
            cairo_line_to(cr, i * spacing + padding, height + padding - (data->data[index][i] / 100.0) * height);
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
        double y = i * height / 100 + padding;
        char label[4];
        snprintf(label, sizeof(label), "%d%%", i);
        cairo_move_to(cr, padding - 40, height + padding - y + 5);
        cairo_show_text(cr, label);
    }

    // Draw the key for the memory and swap lines
    cairo_set_font_size(cr, 14.0);
    int key_x = padding;
    int key_y = height + 2 * padding + 20;
    for (int index = 0; index < 2; ++index) {
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);

        // Draw a small line as a color indicator
        cairo_move_to(cr, key_x, key_y);
        cairo_line_to(cr, key_x + 20, key_y);
        cairo_stroke(cr);

        // Draw the label text
        char key_label[20];
        snprintf(key_label, sizeof(key_label), (index == 0) ? "Memory" : "Swap");
        cairo_move_to(cr, key_x + 25, key_y + 4);
        cairo_show_text(cr, key_label);

        // Update key_x and key_y for the next entry
        key_x += 100;
    }

    return FALSE;
}


int read_memory_swap(double *memory, double *swap) {
    FILE *fp = popen("free -m | awk 'NR==2 {print ($3/$2)*100} NR==3 {print ($3/$2)*100}'", "r");
    if (fp == NULL) {
        perror("Error opening pipe to free command");
        return -1;
    }

    int read_count = fscanf(fp, "%lf\n%lf", memory, swap);
    if (read_count != 2) {
        perror("Error reading from free command");
        pclose(fp);
        return -1;
    }

    pclose(fp);
    return 0;
}

gboolean update_data(gpointer user_data) {
    AppData *data = (AppData *)user_data;
    double memory_usage, swap_usage;

    if (read_memory_swap(&memory_usage, &swap_usage) == 0) {
        // Print memory and swap usage in the console
        printf("Memory Usage: %.2lf%%\n", memory_usage);
        printf("Swap Usage: %.2lf%%\n", swap_usage);

        // Update the chart with the new values
        update_chart(data, 0, memory_usage);
        update_chart(data, 1, swap_usage);
    }

    return TRUE; // Continue calling this function
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Memory and Swap Usage Chart");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create the drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Set up the data structure
AppData data = {0};
data.drawing_area = drawing_area;

    // Assign colors
    generate_random_color(data.colors[0]); // Color for memory
    generate_random_color(data.colors[1]); // Color for swap

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

