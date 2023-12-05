#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_DATA_POINTS 60

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[2][MAX_DATA_POINTS]; // Data for network receiving and sending
    double colors[2][3];             // Colors for network lines
    int first_update;
    float total_received;
    float total_sent;
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

// Function to initialize network utilization
void init_network_utilization(double *last_recv, double *last_send) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Error opening /proc/net/dev");
        return;
    }

    char line[256];
    unsigned long long int recv_bytes, send_bytes;
    char interface[16];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "eth0") != NULL) {  // Replace "eth0" with your interface name
            sscanf(line, "%s %llu %*d %*d %*d %*d %*d %*d %*d %llu %*d %*d %*d %*d %*d %*d %*d",
                   interface, &recv_bytes, &send_bytes);
            *last_recv = recv_bytes;
            *last_send = send_bytes;
            break;
        }
    }

    fclose(fp);
}

void update_total_received_sent(AppData *data) {
    FILE *netstat_fp = fopen("/proc/net/dev", "r");
    if (netstat_fp == NULL) {
        perror("Error opening /proc/net/dev");
        return;
    }

    // Skip the first two lines in /proc/net/dev
    char buffer[256];
    for (int i = 0; i < 2; ++i) {
        if (fgets(buffer, sizeof(buffer), netstat_fp) == NULL) {
            perror("Error reading /proc/net/dev");
            fclose(netstat_fp);
            return;
        }
    }

    // Read the network data for each interface
    float total_received = 0.0;
    float total_sent = 0.0;

    while (fgets(buffer, sizeof(buffer), netstat_fp) != NULL) {
        char interface[32];
        unsigned long long int received, sent;
        if (sscanf(buffer, "%31s %llu %*u %*u %*u %*u %*u %*u %*u %llu", interface, &received, &sent) == 3) {
            total_received += received;
            total_sent += sent;
        }
    }

    fclose(netstat_fp);

    // Update the total_received and total_sent fields in AppData
    data->total_received = total_received;
    data->total_sent = total_sent;
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

    // Draw y-axis numbers (adjusted for 20 MiB/s max)
    for (int i = 0; i <= 20; i += 2) {
        double y = (20 - i) * height / 20.0 + padding;  // Adjusted for 20 MiB/s max
        char label[15];
        snprintf(label, sizeof(label), "%d MiB/s", i);

        cairo_move_to(cr, padding - 65, y + 5);

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

    
    update_total_received_sent(data);  // Call the function to update total_received and total_sent

    // Calculate total received and total sent in GiB
    double total_received_gb = data->total_received / (1024.0 * 1024.0 * 1024.0);
    double total_sent_gb = data->total_sent / (1024.0 * 1024.0 * 1024.0);

    // Create labels text
    gchar *total_received_text, *total_sent_text;

    total_received_text = g_strdup_printf("Total Received: %.2lf GiB", total_received_gb);
    total_sent_text = g_strdup_printf("Total Sent: %.2lf GiB", total_sent_gb);

    // Display Total Received label
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, key_x, key_y + 4);
    cairo_show_text(cr, total_received_text);

    // Update key_x for the Total Sent label
    key_x += 300; // Adjusted space between Total Received and Total Sent

    // Display Total Sent label
    cairo_move_to(cr, key_x, key_y + 4);
    cairo_show_text(cr, total_sent_text);

    // Free allocated memory
    g_free(total_received_text);
    g_free(total_sent_text);

    return FALSE;
}

// Function to read network utilization
int read_network_utilization(double *recv_speed, double *send_speed, double *last_recv, double *last_send) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Error opening /proc/net/dev");
        return -1;
    }

    char line[256];
    unsigned long long int recv_bytes, send_bytes;
    unsigned long long int dummy;
    char interface[16];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "eth0") != NULL) {  // Replace "eth0" with your interface name
            sscanf(line, "%s %llu %*d %*d %*d %*d %*d %*d %*d %llu %*d %*d %*d %*d %*d %*d %*d",
                   interface, &recv_bytes, &send_bytes);
            break;
        }
    }

    fclose(fp);

    // Calculate the speed in MiB/s
    *recv_speed = (double)(recv_bytes - *last_recv) / (1024.0 * 1024.0);
    *send_speed = (double)(send_bytes - *last_send) / (1024.0 * 1024.0);

    // Update last values
    *last_recv = recv_bytes;
    *last_send = send_bytes;

    return 0;
}


gboolean update_data(gpointer user_data) {
    AppData *data = (AppData *)user_data;
    static double last_recv_bytes = 0.0;
    static double last_send_bytes = 0.0;

    double recv_speed, send_speed;

    if (read_network_utilization(&recv_speed, &send_speed, &last_recv_bytes, &last_send_bytes) == 0) {
        // Update the chart with the new values
        if (data->first_update == 0) {
            data->first_update = 1; 
            update_chart(data, 0, 0);
            update_chart(data, 1, 0);
        } else {
            update_chart(data, 0, recv_speed);
            update_chart(data, 1, send_speed);
        }

        // Print network utilization in the console
        printf("Network Receiving: %.2lf MiB/s\n", recv_speed);
        printf("Network Sending: %.2lf MiB/s\n", send_speed);
    }

    return TRUE; // Continue calling this function
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Initialize network utilization
    double last_recv_bytes = 0.0, last_send_bytes = 0.0;
    init_network_utilization(&last_recv_bytes, &last_send_bytes);


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
    data.first_update = 0;

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

