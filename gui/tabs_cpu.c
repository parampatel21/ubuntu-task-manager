#include <gtk/gtk.h>
#include <stdio.h>

#define MAX_BUFFER_SIZE 4096

GtkWidget *cpu_tab;
GtkWidget *cpu_text_view;

void update_cpu_info() {
    FILE *fp;
    char buffer[MAX_BUFFER_SIZE];

    fp = popen("mpstat -P ALL 1 1", "r");
    if (fp == NULL) {
        perror("Error running mpstat");
        return;
    }

    size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
    buffer[bytesRead] = '\0';

    pclose(fp);

    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cpu_text_view));

    gtk_text_buffer_set_text(text_buffer, "", -1);

    gtk_text_buffer_insert_at_cursor(text_buffer, buffer, -1);
}

gboolean update_callback(gpointer user_data) {
    update_cpu_info();

    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);

    cpu_text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(cpu_text_view), FALSE);

    gtk_container_add(GTK_CONTAINER(scrolled_window), cpu_text_view);

    cpu_tab = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cpu_tab), "<b>This is the CPU tab content</b>");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_window, cpu_label);

    g_timeout_add_seconds(1, update_callback, NULL);
}
