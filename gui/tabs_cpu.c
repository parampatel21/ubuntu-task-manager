#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#define MAX_CORES 16

GtkWidget *cpu_labels[MAX_CORES];

// Function to read CPU usage from /proc/stat
void read_cpu_usage(double cpu_usage[MAX_CORES]) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu", 3) == 0) {
            int core;
            sscanf(line, "cpu%d", &core);
            if (core >= 0 && core < MAX_CORES) {
                unsigned long long user, nice, sys, idle;
                sscanf(line + 3, "%llu %llu %llu %llu", &user, &nice, &sys, &idle);

                unsigned long long total = user + nice + sys + idle;
                unsigned long long diff_total = total - cpu_usage[core];

                if (diff_total > 0) {
                    double usage = ((double)(total - idle - cpu_usage[core]) / diff_total) * 100.0;
                    cpu_usage[core] = total - idle;
                    g_print("Core %d Usage: %.2f%%\n", core, usage);

                    // Update the label text
                    char label_text[50];
                    snprintf(label_text, sizeof(label_text), "Core %d Usage: %.2f%%", core, usage);
                    gtk_label_set_text(GTK_LABEL(cpu_labels[core]), label_text);
                }
            }
        }
    }

    fclose(fp);
}

// Callback function for updating CPU usage
gboolean update_cpu_usage(gpointer user_data) {
    double cpu_usage[MAX_CORES] = {0};
    read_cpu_usage(cpu_usage);
    return G_SOURCE_CONTINUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_tab = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(cpu_tab), 10);
    gtk_grid_set_row_spacing(GTK_GRID(cpu_tab), 10);

    for (int i = 0; i < MAX_CORES; ++i) {
        cpu_labels[i] = gtk_label_new("");
        gtk_grid_attach(GTK_GRID(cpu_tab), cpu_labels[i], 0, i, 1, 1);
    }

    // Add the CPU tab to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, gtk_label_new("CPU"));

    // Add timeout function to update CPU usage
    g_timeout_add(1000, update_cpu_usage, NULL);
}
