
#include "process_info.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function to get process information based on a specific command
void getProcessInfo(const char *command, GtkTextBuffer *buffer) {
    // Clear the text buffer before adding new information
    gtk_text_buffer_set_text(buffer, "", -1);

    // Use popen to get process information and write it to the text buffer
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        gtk_text_buffer_set_text(buffer, "Error executing command.", -1);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        gtk_text_buffer_insert_at_cursor(buffer, line, -1);
    }

    pclose(fp);
}

// Callback for the Refresh buttonvoid refresh_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    // Call the function to update process information for all processes
    getProcessInfo("ps aux", buffer);
}

// Callback for displaying all processes in tree format
void all_processes_tree_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    // Call the function to display all processes in tree format
    getProcessInfo("ps -e --forest", buffer);
}

// Callback for displaying all processes in list format
void all_processes_list_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    // Call the function to display all processes in list format
    getProcessInfo("ps aux", buffer);
}

// Callback for displaying user-owned processes in tree format
void user_processes_tree_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    // Call the function to display user-owned processes in tree format
    getProcessInfo("ps -u $(whoami) --forest", buffer);
}

// Callback for displaying user-owned processes in list format
void user_processes_list_clicked(GtkWidget *widget, gpointer user_data) {

    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    // Call the function to display user-owned processes in list format
    getProcessInfo("ps -u $(whoami)", buffer);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Process Info");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a scrolled window to contain the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create a text view to display process information
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    // Get the text buffer of the text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    // Place the text view inside the scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_all_processes_tree = gtk_button_new_with_label("All Processes (Tree)");
    GtkWidget *btn_all_processes_list = gtk_button_new_with_label("All Processes (List)");
    GtkWidget *btn_user_processes_tree = gtk_button_new_with_label("User Processes (Tree)");
    GtkWidget *btn_user_processes_list = gtk_button_new_with_label("User Processes (List)");

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(refresh_clicked), buffer);
    g_signal_connect(btn_all_processes_tree, "clicked", G_CALLBACK(all_processes_tree_clicked), buffer);
    g_signal_connect(btn_all_processes_list, "clicked", G_CALLBACK(all_processes_list_clicked), buffer);
    g_signal_connect(btn_user_processes_tree, "clicked", G_CALLBACK(user_processes_tree_clicked), buffer);
    g_signal_connect(btn_user_processes_list, "clicked", G_CALLBACK(user_processes_list_clicked), buffer);

    gtk_box_pack_start(GTK_BOX(vbox), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_all_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_all_processes_list, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_user_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_user_processes_list, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
