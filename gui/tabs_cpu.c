#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>

// Global variables for GTK
GtkWidget *cpu_text_view;
GtkTextBuffer *cpu_text_buffer;

void update_cpu_tab() {
    FILE *fp;
    char path[1035];

    // Run the mpstat -P ALL command and capture the output
    fp = popen("mpstat -P ALL 1 1", "r");
    if (fp == NULL) {
        printf("Error opening pipe!\n");
        return;
    }

    // Clear the existing text in the text view
    gtk_text_buffer_set_text(cpu_text_buffer, "", -1);

    // Read the output line by line
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        // Look for lines starting with "Average:"
        if (strstr(path, "Average:") != NULL) {
            int core;
            float usage;

            // Parse the output to retrieve core number and usage
            sscanf(path, "Average: %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %f", &core, &usage);

            // Append the result to the text buffer
            char buffer[50];
            sprintf(buffer, "Core %d: %.2f%%\n", core, usage);
            gtk_text_buffer_insert_at_cursor(cpu_text_buffer, buffer, -1);
        }
    }

    // Close the pipe
    pclose(fp);
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    // Create a scrolled window to contain the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create a text view and add it to the scrolled window
    cpu_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(cpu_text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), cpu_text_view);

    // Get the text buffer associated with the text view
    cpu_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cpu_text_view));

    // Create the CPU tab
    GtkWidget *cpu_tab = scrolled_window;

    // Append the CPU tab to the notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    // Update the CPU tab content every second
    g_timeout_add_seconds(1, (GSourceFunc)update_cpu_tab, NULL);
}