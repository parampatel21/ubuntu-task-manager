#include "tabs_cpu.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function to generate a random color
void generate_random_color(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

int get_memory_swap_info(double *total_memory, double *memory_used, double *total_swap, double *swap_used) {
    FILE *fp = popen("free -m | awk 'NR==2{print $2} NR==2{print $3} NR==3{print $2} NR==3{print $3}'", "r");
    if (fp == NULL) {
        perror("Error opening pipe to free command");
        return -1;
    }

    int read_count = fscanf(fp, "%lf\n%lf\n%lf\n%lf", total_memory, memory_used, total_swap, swap_used);
    if (read_count != 4) {
        perror("Error reading from free command");
        pclose(fp);
        return -1;
    }

    *total_memory /= 1024.0;  // Convert to GiB
    *memory_used /= 1024.0;
    *total_swap /= 1024.0;
    *swap_used /= 1024.0;

    pclose(fp);
    return 0;
}

void generate_random_color3(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

// Function to update the line chart with new data
void update_chart3(AppData3 *data, int index, double new_value) {
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

void update_total_received_sent(AppData3 *data) {
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

    // Update the total_received and total_sent fields in AppData3
    data->total_received = total_received;
    data->total_sent = total_sent;
}

gboolean on_draw3(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData3 *data = (AppData3 *)user_data;

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
    cairo_set_font_size(cr, 9.5);

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

    // Draw x-axis label for seconds
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr, width / 2 + padding - 25, height + padding + 40);
    cairo_show_text(cr, "Seconds");


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


gboolean update_data3(gpointer user_data) {
    AppData3 *data = (AppData3 *)user_data;
    static double last_recv_bytes = 0.0;
    static double last_send_bytes = 0.0;

    double recv_speed, send_speed;

    if (read_network_utilization(&recv_speed, &send_speed, &last_recv_bytes, &last_send_bytes) == 0) {
        // Update the chart with the new values
        if (data->first_update == 0) {
            data->first_update = 1; 
            update_chart3(data, 0, 0);
            update_chart3(data, 1, 0);
        } else {
            update_chart3(data, 0, recv_speed);
            update_chart3(data, 1, send_speed);
        }

        // Print network utilization in the console
        printf("Network Receiving: %.2lf MiB/s\n", recv_speed);
        printf("Network Sending: %.2lf MiB/s\n", send_speed);
    }

    return TRUE; // Continue calling this function
}


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
    cairo_set_font_size(cr, 9.5);

    // Draw x-axis numbers
    for (int i = 0; i <= MAX_DATA_POINTS; i += 10) {
        char label[4];
        snprintf(label, sizeof(label), "%d", MAX_DATA_POINTS - i);
        cairo_move_to(cr, i * spacing + padding - 5, height + padding + 20);
        cairo_show_text(cr, label);
    }

    // Draw y-axis numbers
    for (int i = 0; i <= 100; i += 10) {
        double y = height - i * height / 100 + padding;
        char label[5];
        snprintf(label, sizeof(label), "%d%%", i);
        cairo_move_to(cr, padding - 40, y + 5);
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

                // Draw x-axis label for seconds
                cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr, width / 2 + padding - 25, height + padding + 40);
    cairo_show_text(cr, "Seconds");

    }

    // Draw the Memory and Swap labels horizontally inline
    gchar *memory_label_text, *swap_label_text;
    double memory_used, swap_used, total_memory, total_swap;

    if (get_memory_swap_info(&total_memory, &memory_used, &total_swap, &swap_used) == 0) {
        double memory_percentage = (memory_used / total_memory) * 100;
        double swap_percentage = (swap_used / total_swap) * 100;

        memory_label_text = g_strdup_printf("Memory: %.1lf GiB (%.1lf%%) of %.1lf GiB",
                                            memory_used, memory_percentage, total_memory);
        swap_label_text = g_strdup_printf("Swap: %.1lf GiB (%.1lf%%) of %.1lf GiB",
                                          swap_used, swap_percentage, total_swap);

        // Display Memory label
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, key_x, key_y + 4);
        // cairo_show_text(cr, "Memory:");

        cairo_show_text(cr, memory_label_text);

        // Update key_x for the Swap label
        key_x += 300; // Adjusted space between Memory and Swap

        // Display Swap label
        cairo_move_to(cr, key_x, key_y + 4);
        // cairo_show_text(cr, "Swap:");

        cairo_show_text(cr, swap_label_text);

        g_free(memory_label_text);
        g_free(swap_label_text);
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
        // printf("Memory Usage: %.2lf%%\n", memory_usage);
        // printf("Swap Usage: %.2lf%%\n", swap_usage);

        // Update the chart with the new values
        update_chart(data, 0, memory_usage);
        update_chart(data, 1, swap_usage);
    }

    return TRUE; // Continue calling this function
}

// Function to generate a random color
void generate_random_color2(double *color) {
    for (int i = 0; i < 3; i++) {
        color[i] = (double)rand() / (double)RAND_MAX;
    }
}

// Function to update the line chart with new data
void update_chart2(AppData2 *data, int cpu_index, double new_value) {
    // Shift existing data to the left
    memmove(data->data[cpu_index], data->data[cpu_index] + 1, sizeof(double) * (MAX_DATA_POINTS - 1));

    // Add new value to the end
    data->data[cpu_index][MAX_DATA_POINTS - 1] = new_value;

    // Queue a redraw of the drawing area
    gtk_widget_queue_draw(data->drawing_area);
}

gboolean on_draw2(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData2 *data = (AppData2 *)user_data;

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
    cairo_set_font_size(cr, 9.5);

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
        char label[5];
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

        // Draw x-axis label for seconds
        cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr, width / 2 + padding - 25, height + padding + 40);
    cairo_show_text(cr, "Seconds");


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

    gboolean update_data2(gpointer user_data) {
    AppData2 *data = (AppData2 *)user_data;
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

        // printf("CPU%d Usage: %3.2lf%%\n", cpu, percent_usage);

        // Update the chart with the new value
        update_chart2(data, cpu, percent_usage);

        total_tick_old[cpu] = total_tick;
        idle_old[cpu] = idle;
}

fclose(fp);
return TRUE; // Continue calling this function
}

void add_cpu_tab(GtkWidget *notebook, AppData *data, AppData2 *data2, AppData3 *data3) {
    GtkWidget *cpu_label = gtk_label_new("Resources");

    // Create a vertical box to hold the header and drawing area
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    //------------------
    GtkWidget *header2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header2), "<b>CPU History</b>");

    gtk_box_pack_start(GTK_BOX(vbox), header2, FALSE, FALSE, 10);

    GtkWidget *drawing_area2 = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area2, TRUE, TRUE, 0);
    data2->drawing_area = drawing_area2;
    data2->cpu_count = sysconf(_SC_NPROCESSORS_ONLN); // Get the number of CPU cores + 1 for overall CPU
    // Assign colors
    data2->colors[0][0] = 1.0; data2->colors[0][1] = 0.0; data2->colors[0][2] = 0.0; // Red for Overall CPU
    for (int cpu = 1; cpu < data2->cpu_count; ++cpu) {
        if (cpu == 1) {
            data2->colors[cpu][0] = 0.0; data2->colors[cpu][1] = 1.0; data2->colors[cpu][2] = 0.0; // Green for CPU0
        } else {
            generate_random_color2(data2->colors[cpu]); // Random colors for other CPUs
        }
    }
    g_signal_connect(drawing_area2, "draw", G_CALLBACK(on_draw2), data2);
    g_timeout_add_seconds(1, update_data2, data2);

    //------------------

    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<b>Memory and Swap History</b>");

    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 10);

    // Set up the drawing area and timer
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);
    data->drawing_area = drawing_area;

    // Assign colors
    generate_random_color(data->colors[0]); // Color for memory
    generate_random_color(data->colors[1]); // Color for swap

    // Create labels for memory and swap information
    GtkWidget *memory_label = gtk_label_new("Memory: -");
    GtkWidget *swap_label = gtk_label_new("Swap: -");

    // Assign labels to the data structure
    data->memory_label = memory_label;
    data->swap_label = swap_label;

    // Connect the draw signal
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), data);

    // Set up a timer to update data every second
    g_timeout_add_seconds(1, update_data, data);

    // -----------

    GtkWidget *header3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header3), "<b>Network History</b>");

    gtk_box_pack_start(GTK_BOX(vbox), header3, FALSE, FALSE, 10);

    double last_recv_bytes = 0.0, last_send_bytes = 0.0;
    init_network_utilization(&last_recv_bytes, &last_send_bytes);

    GtkWidget *drawing_area3 = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area3, TRUE, TRUE, 0);
    data3->drawing_area = drawing_area3;
    data3->first_update = 0;

    generate_random_color3(data3->colors[0]); // Color for receiving
    generate_random_color3(data3->colors[1]); // Color for sending

    // Connect the draw signal
    g_signal_connect(drawing_area3, "draw", G_CALLBACK(on_draw3), data3);

    // Set up a timer to update data every second
    g_timeout_add_seconds(1, update_data3, data3);

    // -----------

    // Create a parent box for the header and drawing area
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);

    // Append the box to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, cpu_label);
}