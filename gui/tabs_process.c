#include "tabs_process.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


void stop_process(GtkWidget *widget, GtkListStore *store);
void continue_process(GtkWidget *widget, GtkListStore *store);
void kill_process(GtkWidget *widget, GtkListStore *store);
void list_memory_maps(GtkWidget *widget, GtkListStore *store);
void list_open_files(GtkWidget *widget, GtkListStore *store);
void create_context_menu(GtkWidget *text_view, GtkListStore *store);
void update_process_info(int command, int pid, GtkWidget *widget, GtkListStore *store);
void update_proc_information(const char *command, GtkListStore *store, GtkWidget *widget);
void all_processes_tree_clicked(GtkWidget *widget, GtkListStore *store);
void all_processes_list_clicked(GtkWidget *widget, GtkListStore *store);
void user_processes_tree_clicked(GtkWidget *widget, GtkListStore *store);
void user_processes_list_clicked(GtkWidget *widget, GtkListStore *store);
void refresh_clicked(GtkWidget *widget, GtkListStore *store);
void add_proc_column(GtkWidget *text_view, const char *title, int column_id);
void display_open_files_dialog(GtkListStore *store);
void display_mem_map_dialog(GtkListStore *store);
void get_process_info_n_update(const char *command, GtkListStore *store, GtkWidget *widget);
gboolean show_context_menu(GtkWidget *text_view, GdkEventButton *event, GtkWidget *menu);
int get_cursor_line(GtkWidget *widget);
pid_t get_pid_from_model(GtkListStore *store);

struct ProcessInfo {
    char user[13];
    int pid;
    float cpu;
    float mem;
    int vsz;
    int rss;
    char tty[11];
    char stat[9];
    char start[9];
    char time[9];
    char command[11];
    char status[11];
};


enum g_commands {
    STOP,
    CONTINUE,
    KILL,
    MAPS,
    FILES
};

enum g_context {
    ALLPROCESS,
    ALLPROCESS_TREE,
    USERPROCESS,
    USERPROCESS_TREE
};

int g_current_row = -1;   
int g_current_context = ALLPROCESS;

// Function to get process information based on a specific command and update text buffer
void get_process_info_n_update(const char *command, GtkListStore *store, GtkWidget *widget) {

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return;
    }

    int l_line_num = 0;
    gtk_list_store_clear(store);
    char ps_output[1024];
    struct ProcessInfo l_process_info;

    while (fgets(ps_output, sizeof(ps_output), fp) != NULL) {
        if (l_line_num == 0) {    // Header line
            l_line_num++;
            continue;
        }

        sscanf(ps_output, "%s %d %f %f %d %d %s %s %s %s %[^\n]",
                    l_process_info.user,
                    &l_process_info.pid,
                    &l_process_info.cpu,
                    &l_process_info.mem,
                    &l_process_info.vsz,
                    &l_process_info.rss,
                    l_process_info.tty,
                    l_process_info.stat,
                    l_process_info.start,
                    l_process_info.time,
                    l_process_info.command);

        strcpy(l_process_info.status, "Running"); // Initial status for all processes

        gtk_list_store_insert_with_values(store, NULL, -1,
                                          0, l_process_info.user,
                                          1, l_process_info.pid,
                                          2, l_process_info.cpu,
                                          3, l_process_info.mem,
                                          4, l_process_info.vsz,
                                          5, l_process_info.rss,
                                          6, l_process_info.tty,
                                          7, l_process_info.stat,
                                          8, l_process_info.start,
                                          9, l_process_info.time,
                                          10, l_process_info.command,
                                          11, l_process_info.status,
                                          -1);
        l_line_num++;
    }

    pclose(fp);
    gtk_widget_show_all(widget);

} 

// Updates state of a process in the process array and refreshes
void update_process_info(int c_command, int l_pid, GtkWidget *widget, GtkListStore *store) {
    GtkTreeIter iter;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while (valid) {
        gint pid;

        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &pid, -1);

        if (pid == l_pid) {
            gchar *status;
            switch (c_command) {
                case STOP:
                    status = g_strdup("Stopped");
                    break;
                case CONTINUE:
                    status = g_strdup("Running");
                    break;
                case KILL:
                    status = g_strdup("Killed");
                    break;
                default:
                    break;
            }
            gtk_list_store_set(store, &iter, 11, status, -1);
            if (status != NULL) {
                g_free(status);
            }
            break;
        }
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
    gtk_widget_show_all(widget);
} 


// Fetch process_id from the model
pid_t get_pid_from_model(GtkListStore *store) {
    GtkTreeIter iter;
    gboolean valid;
    gint pid;
    int row_index = 0;
    int found = 0;

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while (valid) {
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &pid, -1);
        if (row_index == g_current_row) {
            found = 1;
            break;
        }
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
        row_index++;
    }
    if (found) {
        return pid;
    }
    else {
        return -1;
    }
} 


// Function to stop a process using GTK
void stop_process(GtkWidget *widget, GtkListStore *store) {

    pid_t pid = get_pid_from_model(store);

    if (pid != -1 && kill(pid, SIGSTOP) == 0) {
        update_process_info(STOP, pid, widget, store);
        printf("Process with PID %d has been stopped.\n", pid);
    }
    else {
        perror("Error stopping process");
    }
} 


// Function to continue a process using GTK
void continue_process(GtkWidget *widget, GtkListStore *store) {

    pid_t pid = get_pid_from_model(store);

    if (pid != -1 && kill(pid, SIGCONT) == 0) {
        update_process_info(CONTINUE, pid, widget, store);
        printf("Process with PID %d has been continued.\n", pid);
    }
    else {
        perror("Error continuing process");
    }
} 


// Function to kill a process using GTK
void kill_process(GtkWidget *widget, GtkListStore *store) {

    pid_t pid = get_pid_from_model(store);
    if (pid != -1 && kill(pid, SIGKILL) == 0) {
        update_process_info(KILL, pid, widget, store);
        printf("Process with PID %d has been killed.\n", pid);
    }
    else {
        perror("Error killing process");
    }
} 


// Function to list memory maps of a process using GTK
void list_memory_maps(GtkWidget *widget, GtkListStore *store) {

    pid_t pid = get_pid_from_model(store);

    char command[256];
    sprintf(command, "pmap -XX %d", pid); 
  
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        return;
    }

    GtkListStore *open_files_store = gtk_list_store_new(9, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    char line[1024];

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char file_name[256], start_addr[20], end_addr[20], size[20], flags[20], offset[20],
             private_clean[20], private_dirty[20], shared_clean[20], shared_dirty[20];

        sscanf(line, "%s %*s %s %*s %*s %s %*s %*s %*s %*s %s %s %s %s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s %s",
               start_addr, offset, size, shared_clean, shared_dirty, private_clean, private_dirty,
               flags, file_name);

        gtk_list_store_insert_with_values(open_files_store, NULL, -1,
                                              0, file_name,
                                              1, start_addr,
                                              2, size,
                                              3, flags,
                                              4, offset,
                                              5, private_clean,
                                              6, private_dirty,
                                              7, shared_clean,
                                              8, shared_dirty,
                                              -1);
    }

    pclose(fp);
    display_mem_map_dialog(open_files_store);

} 


// Function to display open files in a dialog
void display_mem_map_dialog(GtkListStore *store) {
    GtkWidget *dialog, *scrolled_window, *tree_view;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    dialog = gtk_dialog_new_with_buttons("Memory Map",
                                         NULL,
                                         GTK_DIALOG_MODAL,
                                         "_Close",
                                         GTK_RESPONSE_CLOSE,
                                         NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 500);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled_window, TRUE, TRUE, 0);

    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_proc_column(tree_view, "Filename", 0);
    add_proc_column(tree_view, "VM Start", 1);
    add_proc_column(tree_view, "VM Size", 2);
    add_proc_column(tree_view, "Flags", 3);
    add_proc_column(tree_view, "VM Offset", 4);
    add_proc_column(tree_view, "Private Clean", 5);
    add_proc_column(tree_view, "Private Dirty", 6);
    add_proc_column(tree_view, "Shared Clean", 7);
    add_proc_column(tree_view, "Shared Dirty", 8);

    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);

    gtk_widget_show_all(dialog);

  
    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

} 

// Function to list open files of a process using GTK
void list_open_files(GtkWidget *widget, GtkListStore *store) {
    pid_t pid = get_pid_from_model(store);
    if (pid == -1) {
        g_print("No process selected.\n");
        return;
    }

    char command[256];
    sprintf(command, "lsof -p %d", pid);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        return;
    }

    GtkListStore *open_files_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gboolean header_skipped = FALSE;
    char line[1024];

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char fd[10], type[10], object[256];

        sscanf(line, "%*s %*d %*s %s %s %*s %*d %*d %s", fd, type, object);

        if (fd && type && object) {
            gtk_list_store_insert_with_values(open_files_store, NULL, -1,
                                              0, fd,
                                              1, type,
                                              2, object,
                                              -1);
        }
    }

    pclose(fp);
    display_open_files_dialog(open_files_store);

} 


// Function to display open files in a dialog
void display_open_files_dialog(GtkListStore *store) {
    GtkWidget *dialog, *scrolled_window, *tree_view;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    dialog = gtk_dialog_new_with_buttons("Open Files",
                                         NULL,
                                         GTK_DIALOG_MODAL,
                                         "_Close",
                                         GTK_RESPONSE_CLOSE,
                                         NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 500);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled_window, TRUE, TRUE, 0);

    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_proc_column(tree_view, "FD", 0);
    add_proc_column(tree_view, "Type", 1);
    add_proc_column(tree_view, "Object", 2);

    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);

    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

} 


// Function to create a context menu for right-click on text view
void create_context_menu(GtkWidget *text_view, GtkListStore *store) {
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

    g_signal_connect(item_stop, "activate", G_CALLBACK(stop_process), store);
    g_signal_connect(item_continue, "activate", G_CALLBACK(continue_process), store);
    g_signal_connect(item_kill, "activate", G_CALLBACK(kill_process), store);
    g_signal_connect(item_memory_maps, "activate", G_CALLBACK(list_memory_maps), store);
    g_signal_connect(item_open_files, "activate", G_CALLBACK(list_open_files), store);

    g_signal_connect(text_view, "button-press-event", G_CALLBACK(show_context_menu), menu);

} 


// Function to handle right-click event and show context menu
gboolean show_context_menu(GtkWidget *text_view, GdkEventButton *event, GtkWidget *menu) {
    gint row, col; 
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    if (event->type == GDK_BUTTON_PRESS && event->button == GDK_BUTTON_SECONDARY) {

        gboolean cell_exists = gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(text_view),
                                      event->x, event->y, &path, &column, &col, NULL);

        if (cell_exists) {
          
            gchar *path_str = gtk_tree_path_to_string(path);

            g_current_row = atoi(path_str);

            g_free(path_str);
            gtk_tree_path_free(path);
        }

        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
        return TRUE;
    }
    return FALSE;

} 

// Function to update process information for all processes
void update_proc_information(const char *command, GtkListStore *store, GtkWidget *widget) {
    get_process_info_n_update(command, store, widget);

} 

// Callback for the Refresh button
void refresh_clicked(GtkWidget *widget, GtkListStore *store) {
    switch (g_current_context) {
        case 0:
            all_processes_list_clicked(widget, store);
            break;
        case 1:
            all_processes_tree_clicked(widget, store);
            break;
        case 2:
            user_processes_list_clicked(widget, store);
            break;
        case 3:
            user_processes_tree_clicked(widget, store);
            break;
    }
} 

// Callback for displaying all processes in tree format
void all_processes_tree_clicked(GtkWidget *widget, GtkListStore *store) {
    update_proc_information("ps -e --forest", store, widget);
    g_current_context = ALLPROCESS_TREE;

} 

void all_processes_list_clicked(GtkWidget *widget, GtkListStore *store) {
    update_proc_information("ps aux", store, widget);
    g_current_context = ALLPROCESS;

} 

// Callback for displaying user-owned processes in tree format
void user_processes_tree_clicked(GtkWidget *widget, GtkListStore *store) {
    update_proc_information("ps -u --forest", store, widget);
    g_current_context = USERPROCESS_TREE;

} 

// Callback for displaying user-owned processes in list format
void user_processes_list_clicked(GtkWidget *widget, GtkListStore *store) {
    update_proc_information("ps -u", store, widget);
    g_current_context = USERPROCESS;

} 

// Adds a text column to a tree view with a given title and associates it with a column in the model
void add_proc_column(GtkWidget *text_view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(text_view), column);

} 

// Function to add process tab
void add_process_tab(GtkWidget *notebook) {
    GtkWidget *tab_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkListStore *store = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_INT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
                                                 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *text_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_proc_column(text_view, "User", 0);
    add_proc_column(text_view, "PID", 1);
    add_proc_column(text_view, "%CPU", 2);
    add_proc_column(text_view, "%Mem", 3);
    add_proc_column(text_view, "VSZ", 4);
    add_proc_column(text_view, "RSS", 5);
    add_proc_column(text_view, "TTY", 6);
    add_proc_column(text_view, "Stat", 7);
    add_proc_column(text_view, "Start", 8);
    add_proc_column(text_view, "Time", 9);
    add_proc_column(text_view, "Command", 10);
    add_proc_column(text_view, "Status", 11);

    create_context_menu(text_view, store);

    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_all_processes_tree = gtk_button_new_with_label("All Processes (Tree)");
    GtkWidget *btn_all_processes_list = gtk_button_new_with_label("All Processes (List)");
    GtkWidget *btn_user_processes_tree = gtk_button_new_with_label("User Processes (Tree)");
    GtkWidget *btn_user_processes_list = gtk_button_new_with_label("User Processes (List)");

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(refresh_clicked), store);
    g_signal_connect(btn_all_processes_tree, "clicked", G_CALLBACK(all_processes_tree_clicked), store);
    g_signal_connect(btn_all_processes_list, "clicked", G_CALLBACK(all_processes_list_clicked), store);
    g_signal_connect(btn_user_processes_tree, "clicked", G_CALLBACK(user_processes_tree_clicked), store);
    g_signal_connect(btn_user_processes_list, "clicked", G_CALLBACK(user_processes_list_clicked), store);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(tab_content), scrolled_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_all_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_all_processes_list, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_user_processes_tree, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_content), btn_user_processes_list, FALSE, FALSE, 0);

    GtkWidget *tab_label = gtk_label_new("Processes");

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content, tab_label);
    refresh_clicked(text_view, store);
    gtk_widget_show_all(notebook);

} 
