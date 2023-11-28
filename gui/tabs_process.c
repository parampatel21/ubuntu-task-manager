//
// Created by Param Patel on 11/22/23.
//

#include "tabs_process.h"
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void getProcessInfoAndUpdateBuffer(const char *command, GtkTextBuffer *buffer) {
    gtk_text_buffer_set_text(buffer, "", -1);

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

void stop_process() {
    printf("Stopping process...\n");
}

void continue_process() {
    printf("Continuing process...\n");
}

void kill_process() {
    printf("Killing process...\n");
}

void list_memory_maps() {
    printf("Listing memory maps...\n");
}

void list_open_files() {
    printf("Listing open files...\n");
}

gboolean show_context_menu(GtkWidget *text_view, GdkEventButton *event, GtkWidget *menu) {
    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
        return TRUE;
    }
    return FALSE;
}

void createContextMenu(GtkWidget *text_view) {
    GtkWidget *menu = gtk_menu_new();

    GtkWidget *item_stop = gtk_menu_item_new_with_label("Stop");
    GtkWidget *item_continue = gtk_menu_item_new_with_label("Continue");
    GtkWidget *item_kill = gtk_menu_item_new_with_label("Kill");
    GtkWidget *item_memory_maps = gtk_menu_item_new_with_label("List Memory Maps");
    GtkWidget *item_open_files = gtk_menu_item_new_with_label("List Open Files");

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_stop);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_continue);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_kill);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_memory_maps);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_open_files);

    gtk_widget_show_all(menu);

    g_signal_connect(item_stop, "activate", G_CALLBACK(stop_process), NULL);
    g_signal_connect(item_continue, "activate", G_CALLBACK(continue_process), NULL);
    g_signal_connect(item_kill, "activate", G_CALLBACK(kill_process), NULL);
    g_signal_connect(item_memory_maps, "activate", G_CALLBACK(list_memory_maps), NULL);
    g_signal_connect(item_open_files, "activate", G_CALLBACK(list_open_files), NULL);

    g_signal_connect(text_view, "button-press-event", G_CALLBACK(show_context_menu), menu);
}

void updateProcessInfoAndDisplay(const char *command, GtkTextBuffer *buffer) {
    getProcessInfoAndUpdateBuffer(command, buffer);
}

void refresh_clicked(GtkWidget *widget, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    updateProcessInfoAndDisplay("ps aux", buffer);
}

// Implement other callbacks for various button functionalities...

void add_process_tab(GtkWidget *notebook) {
    GtkWidget *process_label = gtk_label_new("Process");
    GtkWidget *process_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(process_tab), scrolled_window, TRUE, TRUE, 0);

    createContextMenu(text_view);

    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    // Create other buttons for various functionalities...

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(refresh_clicked), buffer);
    // Connect other buttons similarly...

    gtk_box_pack_start(GTK_BOX(process_tab), btn_refresh, FALSE, FALSE, 0);
    // Add other buttons similarly...

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), process_tab, process_label);

    updateProcessInfoAndDisplay("ps aux", buffer);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Process Info");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    add_process_tab(notebook);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
