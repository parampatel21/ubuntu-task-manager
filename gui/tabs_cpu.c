#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>

// Structure to hold CPU information
typedef struct {
    float usage[8];  // Assuming you have 8 cores, adjust as needed
} CPUInfo;

// Function to parse the output of mpstat
void parse_mpstat_output(const char *output, CPUInfo *cpu_info) {
    const char *delimiter = " \t";
    const char *line = strtok((char *)output, "\n");

    // Skip the first two lines
    for (int i = 0; i < 2; ++i) {
        line = strtok(NULL, "\n");
    }

    // Parse the rest of the lines to get CPU usage
    int core = 0;
    while (line != NULL && core < 8) {
        sscanf(line, "%*s %f", &cpu_info->usage[core]);
        line = strtok(NULL, "\n");
        core++;
    }
}

// Function to run mpstat command and update CPU information
void *update_cpu_info(void *data) {
    CPUInfo *cpu_info = (CPUInfo *)data;

    while (1) {
        FILE *fp = popen("mpstat -P ALL 1 1", "r");
        if (fp != NULL) {
            char buffer[1024];
            size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, fp);
            buffer[bytesRead] = '\0';

            parse_mpstat_output(buffer, cpu_info);

            pclose(fp);
        }

        // Sleep for 1 second
        sleep(1);
    }

    return NULL;
}

// Function to update the GTK GUI with CPU information
gboolean update_gui(gpointer user_data) {
    CPUInfo *cpu_info = (CPUInfo *)user_data;

    // TODO: Update GTK GUI with cpu_info->usage
    // For simplicity, let's print the CPU usage to the console
    printf("CPU Usage: ");
    for (int i = 0; i < 8; ++i) {
        printf("Core %d: %.2f%% ", i, cpu_info->usage[i]);
    }
    printf("\n");

    return G_SOURCE_CONTINUE;
}

// Function to add CPU tab to the notebook
void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");
    GtkWidget *cpu_tab = gtk_label_new("This is the CPU tab content");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    // Create a thread to update CPU information
    pthread_t thread;
    pthread_create(&thread, NULL, update_cpu_info, NULL);

    // Create a timer to update the GUI periodically
    g_timeout_add_seconds(1, update_gui, NULL);
}
