#include "tabs_cpu.h"
#include <gtk/gtk.h>

const gchar* get_cpu_usage_string() {
    FILE* fp = fopen("/proc/self/stat", "r");

    if (fp == NULL) {
        perror("Error opening /proc/self/stat");
        return "Error";
    }

    unsigned long utime, stime;
    if (fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %lu %lu", &utime, &stime) != 2) {
        perror("Error reading /proc/self/stat");
        fclose(fp);
        return "Error";
    }

    fclose(fp);

    unsigned long total_time = utime + stime;
    double seconds = 1.0;
    double cpu_usage = 100.0 * total_time / (sysconf(_SC_CLK_TCK) * seconds);

    static gchar cpu_usage_str[50];
    g_snprintf(cpu_usage_str, sizeof(cpu_usage_str), "CPU Usage: %.2f%%", cpu_usage);

    return cpu_usage_str;
}

gboolean update_timer_callback(GtkLabel *label) {
    const gchar *cpu_usage_str = get_cpu_usage_string();
    gtk_label_set_text(label, cpu_usage_str);
    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    GtkWidget *cpu_tab = gtk_label_new("Initializing...");
    GtkLabel *cpu_label_widget = GTK_LABEL(cpu_tab);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    guint timer_id = g_timeout_add_seconds(1, (GSourceFunc)update_timer_callback, cpu_label_widget);

}
