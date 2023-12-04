#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_MAX 1024
#define MAX_DATA_POINTS 60
#define KEY_MAX_WIDTH 400

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for network receiving and sending
    double colors[2][3];             // Colors for network lines
} AppData;

// Function to generate a random color
void generate_random_color(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

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

    // Draw the line chart for network receiving and sending
    for (int index = 0; index < 2; ++index) {
        // Set the color for the current line
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);

        // Map the first data point to the graph's scale
        cairo_move_to(cr, padding, height + padding - (data->data[index][0] / 10.0) * height);
        for (int i = 1; i < MAX_DATA_POINTS; ++i) {
            // Map each subsequent data point to the graph's scale
            cairo_line_to(cr, i * spacing + padding, height + padding - (data->data[index][i] / 10.0) * height);
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
for (int i = 0; i <= 10; i += 2) {
    double y = height - i * height / 10 + padding;
    char label[12];  // Increase the size of the array to accommodate the formatted string
    snprintf(label, sizeof(label), "%.1f MiB/s", i * 1.0);
    cairo_move_to(cr, padding - 40, y + 5);
    cairo_show_text(cr, label);
}


    // Draw the key for the network lines
    cairo_set_font_size(cr, 14.0);
    int key_x = padding;
    int key_y = height + 2 * padding + 20;
    const char *key_labels[] = {"Receiving", "Sending"};
    for (int index = 0; index < 2; ++index) {
        cairo_set_source_rgb(cr, data->colors[index][0], data->colors[index][1], data->colors[index][2]);

        // Draw a small line as a color indicator
        cairo_move_to(cr, key_x, key_y);
        cairo_line_to(cr, key_x + 20, key_y);
        cairo_stroke(cr);

        // Draw the label text
        cairo_move_to(cr, key_x + 25, key_y + 4);
        cairo_show_text(cr, key_labels[index]);

        // Update key_x and key_y for the next entry
        key_x += 100;
    }

    return FALSE;
}

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int read_network_utilization(double *recv_speed, double *send_speed) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Error opening /proc/net/dev");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "eth0") != NULL) {
            break;
        }
    }

    fclose(fp);

    // Updated sscanf to capture additional fields
    int recv_packets, recv_errs, recv_drop, recv_fifo, recv_frame, recv_compressed, recv_multicast;
    int send_packets, send_errs, send_drop, send_fifo, send_colls, send_carrier, send_compressed;
    
int read_count = sscanf(line, "%*s %lf %d %d %d %d %d %d %d %d %lf %d %d %d %d %d %d %d %d",
    &recv_speed, &recv_packets, &recv_errs, &recv_drop, &recv_fifo, &recv_frame, &recv_compressed, &recv_multicast,
    &send_speed, &send_packets, &send_errs, &send_drop, &send_fifo, &send_colls, &send_carrier, &send_compressed);


    if (read_count != 17) {  // Adjust the count based on the actual number of fields
        perror("Error reading network utilization");
        return -1;
    }

    return 0;
}



gboolean update_data(gpointer user_data) {
    AppData *data = (AppData *)user_data;
    static double prev_recv_speed = 0.0;
    static double prev_send_speed = 0.0;

    double recv_speed, send_speed;

    if (read_network_utilization(&recv_speed, &send_speed) == 0) {
        // Convert from bytes to MiB/s
        recv_speed /= (1024 * 1024);
        send_speed /= (1024 * 1024);

        // Calculate the difference
        double diff_recv_speed = recv_speed - prev_recv_speed;
        double diff_send_speed = send_speed - prev_send_speed;

        // Update the chart with the new values
        update_chart(data, 0, diff_recv_speed);
        update_chart(data, 1, diff_send_speed);

        // Print network utilization in the console
        printf("Network Receiving: %.2lf MiB/s\n", diff_recv_speed);
        printf("Network Sending: %.2lf MiB/s\n", diff_send_speed);

        // Update the previous values
        prev_recv_speed = recv_speed;
        prev_send_speed = send_speed;
    }

    return TRUE; // Continue calling this function
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Network Utilization Chart");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create the drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Set up the data structure
    AppData data = {0};
    data.drawing_area = drawing_area;

    // Assign colors
    generate_random_color(data.colors[0]); // Color for receiving
    generate_random_color(data.colors[1]); // Color for sending

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
