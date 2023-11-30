//
// Created by Param Patel on 11/22/23.
//

#include "tabs_process.h"
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>

// Function to update process information in a GtkTextView
void updateProcessInfo(const char *command, GtkTextBuffer *buffer) {
    FILE *fp;
    char buffer_cmd[1024];
    fp = popen(command, "r");
    if (fp == NULL) {
        g_print("Failed to run command\n");
        return;
    }

    // Clear previous content from the text buffer
    gtk_text_buffer_set_text(buffer, "", -1);

    // Read the command output line by line and append it to the text buffer
    while (fgets(buffer_cmd, sizeof(buffer_cmd), fp) != NULL) {
        gtk_text_buffer_insert_at_cursor(buffer, buffer_cmd, -1);
    }

    // Close the pipe
    pclose(fp);
}

// Callbacks for process tab buttons
static void refresh_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfo("ps aux", buffer);
}

static void all_processes_tree_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfo("ps -e --forest", buffer);
}

static void all_processes_list_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfo("ps aux", buffer);
}

static void user_processes_tree_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfo("ps -u $(whoami) --forest", buffer);
}

static void user_processes_list_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfo("ps -u $(whoami)", buffer);
}

void add_process_tab(GtkWidget *notebook) {
    GtkWidget *tab_label = gtk_label_new("Processes");
    GtkWidget *tab_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(tab_content), 10);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(tab_content), scrolled_window, TRUE, TRUE, 0);

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

    gtk_box_pack_start(GTK_BOX(tab_content), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_all_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_all_processes_list, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_user_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_user_processes_list, FALSE, FALSE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content, tab_label);
}


//int main(int argc, char *argv[]) {
//    gtk_init(&argc, &argv);
//
//    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//    gtk_window_set_title(GTK_WINDOW(window), "Process Info");
//    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
//    gtk_widget_set_size_request(window, 600, 400);
//    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
//
//    GtkWidget *notebook = gtk_notebook_new();
//    gtk_container_add(GTK_CONTAINER(window), notebook);
//
//    add_process_tab(notebook);
//
//    gtk_widget_show_all(window);
//
//    gtk_main();
//
//    return 0;
//}
