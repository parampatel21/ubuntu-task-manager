#include "tabs_cpu.h"
#include <gtk/gtk.h>

#define MAX_CORES 32 // Adjust this based on your system's maximum number of CPU cores

// Function to retrieve CPU usage for each core and format it as a string
const gchar* get_cpu_usage_string() {
    FILE* fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return "Error";
    }

    unsigned long user, nice, system, idle, iowait, irq, softirq;
    unsigned long total_time1 = 0, total_time2 = 0;
    unsigned long usage[MAX_CORES];
    int core_count = -1; // Start from -1 to exclude the total line from /proc/stat

    // Read the CPU usage data
    while (fscanf(fp, "cpu%*d %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
        total_time1 += user + nice + system + idle + iowait + irq + softirq;
        usage[++core_count] = total_time1;
    }

    fclose(fp);

    // Wait for a short duration to calculate the CPU usage difference
    g_usleep(100000);

    fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return "Error";
    }

    total_time2 = 0;
    core_count = -1;

    // Read the CPU usage data again
    while (fscanf(fp, "cpu%*d %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7) {
        total_time2 += user + nice + system + idle + iowait + irq + softirq;
        usage[++core_count] = total_time2;
    }

    fclose(fp);

    // Calculate and format CPU usage for each core
    static gchar cpu_usage_str[1024];
    g_snprintf(cpu_usage_str, sizeof(cpu_usage_str), "CPU Usage:\n");

    for (int i = 0; i <= core_count; ++i) {
        double cpu_usage = (double)(usage[i] - usage[i - 1]) / (total_time2 - total_time1) * 100.0;
        g_snprintf(cpu_usage_str + strlen(cpu_usage_str), sizeof(cpu_usage_str) - strlen(cpu_usage_str), "Core %d: %.2f%%\n", i, cpu_usage);
    }

    return cpu_usage_str;
}

// Timer callback function to update CPU usage at regular intervals
gboolean update_timer_callback(GtkBox *box) {
    const gchar *cpu_usage_str = get_cpu_usage_string();

    // Clear existing labels in the box
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(box));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Create a label for each core
    gchar** lines = g_strsplit(cpu_usage_str, "\n", -1);
    for (int i = 0; lines[i] != NULL; ++i) {
        GtkWidget *label = gtk_label_new(lines[i]);
        gtk_box_pack_start(box, label, FALSE, FALSE, 0);
    }
    g_strfreev(lines);

    gtk_widget_show_all(GTK_WIDGET(box));
    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    // Create a GtkBox to contain the labels for each core
    GtkWidget *cpu_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkBox *cpu_box = GTK_BOX(cpu_tab);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    // Set up a timer to update CPU usage every second (adjust as needed)
    guint timer_id = g_timeout_add_seconds(1, (GSourceFunc)update_timer_callback, cpu_box);

    // You might want to store the timer ID somewhere if you need to stop the timer later
    // g_source_remove(timer_id);
}
