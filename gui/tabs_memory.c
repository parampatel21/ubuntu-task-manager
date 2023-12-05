#include "tabs_memory.h"
#include <gtk/gtk.h>

#include <mntent.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>

typedef struct {
  char* device;
  char* mount_point;
  char* type;
  long total_size;
  long free_size;
  long available_size;
  long used_size;
} FileSystemInfo;


char* format_size(long size);
void add_column(GtkWidget *tree_view, const char *title, int column_id);
FileSystemInfo* get_file_system_info(int* count);
GtkWidget* create_memory_bar_widget(long long used, long long total);
void add_memory_info(GtkBox *vbox);
void add_memory_tab(GtkWidget *notebook);


/*
 * Retrieves detailed file system information for each mount point
 */

FileSystemInfo* get_file_system_info(int* count) {
    FILE* mtab = NULL;
    struct mntent* mnt;
    struct statvfs buf;

    int capacity = 10;
    *count = 0;
    FileSystemInfo* fs_info = malloc(capacity * sizeof(FileSystemInfo));

    mtab = setmntent("/etc/mtab", "r");
    if (mtab == NULL) {
        perror("setmntent");
        return NULL;
    }

    while ((mnt = getmntent(mtab)) != NULL) {
        if (statvfs(mnt->mnt_dir, &buf) == -1) {
            continue;
        }

        if (*count >= capacity) {
            capacity *= 2;
            fs_info = realloc(fs_info, capacity * sizeof(FileSystemInfo));
        }

        fs_info[*count].device = strdup(mnt->mnt_fsname);
        fs_info[*count].mount_point = strdup(mnt->mnt_dir);
        fs_info[*count].type = strdup(mnt->mnt_type);
        fs_info[*count].total_size = buf.f_blocks * buf.f_frsize;
        fs_info[*count].free_size = buf.f_bfree * buf.f_frsize;
        fs_info[*count].available_size = buf.f_bavail * buf.f_frsize;
        fs_info[*count].used_size = fs_info[*count].total_size - fs_info[*count].free_size;

        (*count)++;
    }
    endmntent(mtab);

    return fs_info;

}

/*
 * Calculates the physical memory information from the system.
 */

struct sysinfo get_physical_memory_info() {
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    return memInfo;
}

/*
 * Formats a size value (in bytes) into a human-readable string with appropriate units
 */

char* format_size(long size) {
    const char* sizes[] = { "B", "KiB", "MiB", "GiB", "TiB" };
    int order = 0;
    double formatted_size = size;
    while (formatted_size >= 1024 && order + 1 < sizeof(sizes) / sizeof(sizes[0])) {
      order++;
      formatted_size /= 1024;
    }
    char* result = g_strdup_printf("%.2f %s", formatted_size, sizes[order]);
    return result;
}


/*
 * Creates a horizontal box containing a used memory bar and an available memory bar
 */

GtkWidget* create_memory_bar_widget(long long used, long long total) {
    GtkWidget *bar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    double used_fraction = (double)used / (double)total;

    GtkWidget *used_memory_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(used_memory_bar), used_fraction);
    gtk_widget_set_name(used_memory_bar, "used_memory_bar");

    GtkWidget *available_memory_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(available_memory_bar), 1.0 - used_fraction);
    gtk_widget_set_name(available_memory_bar, "available_memory_bar");

    gtk_box_pack_start(GTK_BOX(bar_box), used_memory_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(bar_box), available_memory_bar, TRUE, TRUE, 0);

    return bar_box;
}

/*
 * Adds memory bars for physical and swap memory to a vertical box
 */

void add_memory_info(GtkBox *vbox) {
    struct sysinfo memInfo;
    sysinfo(&memInfo);

    long long total_physical_mem = memInfo.totalram * memInfo.mem_unit;
    long long used_physical_mem = total_physical_mem - (memInfo.freeram * memInfo.mem_unit);
    long long total_swap = memInfo.totalswap * memInfo.mem_unit;
    long long used_swap = total_swap - (memInfo.freeswap * memInfo.mem_unit);

    GtkWidget* physical_memory_section = create_memory_bar_widget(used_physical_mem, total_physical_mem);
    GtkWidget* swap_memory_section = create_memory_bar_widget(used_swap, total_swap);

    gtk_box_pack_start(vbox, physical_memory_section, TRUE, TRUE, 0);
    gtk_box_pack_start(vbox, swap_memory_section, TRUE, TRUE, 0);
}

/*
 * Adds a text column to a tree view with a given title and associates it with a column in the model
 */

void add_text_column(GtkWidget *tree_view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
}

/*
 * Adds a progress bar column to a tree view for displaying used memory percentage
 */

void add_progress_column(GtkWidget *tree_view, const char *title, int column_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_progress_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "value", column_id, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
}

/*
 * Adds the memory tab to the main notebook widget with all memory information
 */

void add_memory_tab(GtkWidget *notebook) {
    GtkWidget *memory_label = gtk_label_new("File Systems");
    GtkWidget *memory_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkListStore *store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_text_column(tree_view, "Device", 0);
    add_text_column(tree_view, "Directory", 1);
    add_text_column(tree_view, "Type", 2);
    add_text_column(tree_view, "Total", 3);
    add_text_column(tree_view, "Free", 4);
    add_text_column(tree_view, "Available", 5);
    add_text_column(tree_view, "Used", 6);

    add_progress_column(tree_view, "Used %", 7);

    int fs_count;
    FileSystemInfo* fs_info = get_file_system_info(&fs_count);
    for (int i = 0; i < fs_count; ++i) {
        char* total_str = format_size(fs_info[i].total_size);
        char* free_str = format_size(fs_info[i].free_size);
        char* available_str = format_size(fs_info[i].available_size);
        char* used_str = format_size(fs_info[i].used_size);
        int used_percentage = (int)((double)fs_info[i].used_size / (double)fs_info[i].total_size * 100.0);

        gtk_list_store_insert_with_values(store, NULL, -1,
                                          0, fs_info[i].device,
                                          1, fs_info[i].mount_point,
                                          2, fs_info[i].type,
                                          3, total_str,
                                          4, free_str,
                                          5, available_str,
                                          6, used_str,
                                          7, used_percentage, //progress value
                                          -1);

        g_free(total_str);
        g_free(free_str);
        g_free(available_str);
        g_free(used_str);
    }

    for (int i = 0; i < fs_count; ++i) {
        free(fs_info[i].device);
        free(fs_info[i].mount_point);
        free(fs_info[i].type);
    }
    free(fs_info);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);
    gtk_box_pack_start(GTK_BOX(memory_tab), scrolled_window, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), memory_tab, memory_label);

    g_object_unref(store);

    gtk_widget_show_all(notebook);
}


