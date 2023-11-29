#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

GtkWidget *cpu_tab;

void update_cpu_tab(GtkWidget *labels[]) {
    FILE *mpstat_fp;
    char mpstat_buffer[5000];
    mpstat_fp = popen("mpstat -P ALL 1 1", "r");  // Run the command for 1 second to get updated values
    fgets(mpstat_buffer, sizeof(mpstat_buffer), mpstat_fp);
    pclose(mpstat_fp);

    char *line = strtok(mpstat_buffer, "\n");
    int core_index = 0;

    while (line != NULL) {
        if (strstr(line, "all") == NULL && strstr(line, "CPU") == NULL) {
            double usage;
            sscanf(line + 13, "%lf", &usage);

            char label_text[50];
            snprintf(label_text, sizeof(label_text), "Core %d: %.2f%%", core_index, usage);

            gtk_label_set_text(GTK_LABEL(labels[core_index]), label_text);
            core_index++;
        }

        line = strtok(NULL, "\n");
    }
}

gboolean update_cpu_tab_timeout(gpointer user_data) {
    GtkWidget **labels = (GtkWidget **)user_data;
    update_cpu_tab(labels);
    return G_SOURCE_CONTINUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    // Create a grid to organize labels
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(cpu_tab), grid);

    // Create an array of labels for each core
    GtkWidget *core_labels[8];
    for (int i = 0; i < 8; i++) {
        core_labels[i] = gtk_label_new("");
        gtk_grid_attach(GTK_GRID(grid), core_labels[i], 0, i, 1, 1);
    }

    // Append the grid to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    // Update the CPU tab every half second
    g_timeout_add(500, update_cpu_tab_timeout, core_labels);
}