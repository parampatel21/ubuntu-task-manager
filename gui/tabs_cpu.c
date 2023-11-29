#include <gtk/gtk.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 4096

GtkWidget *cpu_tab;
GtkWidget *cpu_text_view;

void update_cpu_info() {
    FILE *fp;
    char buffer[MAX_BUFFER_SIZE];

    // Run mpstat -P ALL command and read the output
    fp = popen("mpstat -P ALL 1 1", "r");
    if (fp == NULL) {
        perror("Error running mpstat");
        return;
    }

    // Read the output into the buffer
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
    buffer[bytesRead] = '\0';

    // Close the file stream
    pclose(fp);

    // Get the text buffer from the text view
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cpu_text_view));

    // Clear the existing text in the buffer
    gtk_text_buffer_set_text(text_buffer, "", -1);

    // Append the new output to the buffer
    gtk_text_buffer_insert_at_cursor(text_buffer, buffer, -1);
}

gboolean update_callback(gpointer user_data) {
    // Update the CPU information
    update_cpu_info();

    // Return TRUE to continue calling this function periodically
    return TRUE;
}

void async_update_cpu_info() {
    // Create a child process to run the mpstat command
    GPid pid;
    gchar *stdout_contents = NULL;
    GError *error = NULL;

    gboolean result = g_spawn_sync(
            NULL,               // working directory
            (char *[]){        // command and arguments
                    "/bin/sh",
                    "-c",
                    "mpstat -P ALL 1 1",
                    NULL
            },
            NULL,               // environment variables
            G_SPAWN_SEARCH_PATH,
            NULL,               // child setup function
            NULL,               // user data
            &stdout_contents,  // standard output
            NULL,               // standard error
            &pid,               // process ID
            &error              // GError
    );

    if (result) {
        // Get the text buffer from the text view
        GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cpu_text_view));

        // Clear the existing text in the buffer
        gtk_text_buffer_set_text(text_buffer, "", -1);

        // Append the new output to the buffer
        gtk_text_buffer_insert_at_cursor(text_buffer, stdout_contents, -1);

        g_free(stdout_contents);
    } else {
        g_error("Error running mpstat: %s", error->message);
        g_error_free(error);
    }
}

gboolean async_update_callback(gpointer user_data) {
    // Update the CPU information asynchronously
    async_update_cpu_info();

    // Return TRUE to continue calling this function periodically
    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    // Create a scrolled window to contain the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

    // Create a text view to display the CPU information
    cpu_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(cpu_text_view), FALSE);

    // Add the text view to the scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled_window), cpu_text_view);

    // Create the CPU tab with the scrolled window
    cpu_tab = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cpu_tab), "<b>This is the CPU tab content</b>");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, cpu_label);

    // Set up a timer to update the CPU information every second
    g_timeout_add_seconds(1, async_update_callback, NULL);
}
