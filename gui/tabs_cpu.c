#include "tabs_cpu.h"
#include <gtk/gtk.h>

// Structure to hold data for each core
typedef struct {
    GtkWidget *label;
    int core_number;
} CoreData;

// Function to update core usage labels
void update_core_labels(CoreData *core_data) {
    FILE *mpstat_fp;
    char mpstat_buffer[5000];
    mpstat_fp = popen("mpstat -P ALL 1 1", "r"); // Run the command for 1 second to get updated values
    fgets(mpstat_buffer, sizeof(mpstat_buffer), mpstat_fp);
    pclose(mpstat_fp);

    // Find the line with core usage information
    char *line = strstr(mpstat_buffer, "all");
    if (line != NULL) {
        // Move to the next line to get the core details
        line = strtok(NULL, "\n");

        while (line != NULL) {
            int core;
            float usage;
            sscanf(line, "%*s %d %*f %*f %*f %*f %*f %*f %*f %*f %*f %f", &core, &usage);

            // Update the corresponding label
            if (core_data[core].label != NULL) {
                char text[50];
                snprintf(text, sizeof(text), "Core %d: %.2f%%", core, usage);
                gtk_label_set_text(GTK_LABEL(core_data[core].label), text);
            }

            line = strtok(NULL, "\n");
        }
    }
}

// Function to create the CPU tab
void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Array to hold core data
    int num_cores = 8; // Change this according to the number of cores on your system
    CoreData core_data[num_cores];

    // Create labels for each core
    for (int i = 0; i < num_cores; i++) {
        core_data[i].label = gtk_label_new(NULL);
        core_data[i].core_number = i;
        gtk_box_pack_start(GTK_BOX(cpu_tab), core_data[i].label, FALSE, FALSE, 0);
    }

    // Add the CPU tab to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    // Set up a timer to update core labels every half second
    g_timeout_add(500, (GSourceFunc)update_core_labels, core_data);
}
