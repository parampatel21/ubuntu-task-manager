//
// Created by Param Patel on 11/22/23.
//

#include "tabs_memory.h"
#include <gtk/gtk.h>

#include <mntent.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>

typedef struct {
    char* mount_point;
    long total_size;
    long available_size;
    long used_size;
} FileSystemInfo;


char* format_size(long size);
void add_column(GtkWidget *tree_view, const char *title, int column_id);
//GtkWidget* create_memory_bar(long long total, long long used, const char* label, GdkRGBA color_used, GdkRGBA color_free);
FileSystemInfo* get_file_system_info(int* count);



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

        fs_info[*count].mount_point = strdup(mnt->mnt_dir);
        fs_info[*count].total_size = buf.f_blocks * buf.f_frsize;
        fs_info[*count].available_size = buf.f_bavail * buf.f_frsize;
        fs_info[*count].used_size = fs_info[*count].total_size - fs_info[*count].available_size;
        (*count)++;
    }
    endmntent(mtab);

    return fs_info;
}


struct sysinfo get_physical_memory_info() {
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    return memInfo;
}

/*GtkWidget* create_memory_bar(long long total, long long used, const char* label, GdkRGBA color_used, GdkRGBA color_free) {
    GtkWidget* bar = gtk_progress_bar_new();
    double fraction = (double)used / (double)total;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar), fraction);

    char* total_str = format_size(total);
    char* used_str = format_size(used);
    char* text = g_strdup_printf("%s: %s / %s", label, used_str, total_str);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(bar), text);

    GtkStyleContext *context = gtk_widget_get_style_context(bar);
    GdkRGBA color;
    if (fraction > 0.75) {
        color = color_used; //red
    } else {
        color = color_free; //blue
    }
    gtk_style_context_add_class(context, "memory-bar");
    gtk_widget_override_background_color(bar, GTK_STATE_FLAG_NORMAL, &color);

    g_free(total_str);
    g_free(used_str);
    g_free(text);

    return bar;
}*/

GtkWidget* create_memory_section(long long total, long long used, const char* label) {
    GtkWidget *section_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    double used_fraction = (double)used / (double)total;
    double available_fraction = 1.0 - used_fraction;

    GtkWidget* used_memory_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(used_memory_bar), used_fraction);
    char* used_str = format_size(used);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(used_memory_bar), used_str);
    gtk_widget_set_name(used_memory_bar, "used_memory_bar");

    GtkWidget* available_memory_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(available_memory_bar), available_fraction);
    char* available_str = format_size(total - used);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(available_memory_bar), available_str);
    gtk_widget_set_name(available_memory_bar, "available_memory_bar");

    g_free(used_str);
    g_free(available_str);

    gtk_box_pack_start(GTK_BOX(section_box), used_memory_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(section_box), available_memory_bar, TRUE, TRUE, 0);

    GtkWidget *label_widget = gtk_label_new(label);
    GtkWidget *label_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(label_box), label_widget, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(label_box), section_box, FALSE, FALSE, 0);

    return label_box;
}

void add_memory_info(GtkBox *vbox) {
    struct sysinfo memInfo = get_physical_memory_info();

    long long total_physical_mem = memInfo.totalram * memInfo.mem_unit;
    long long used_physical_mem = total_physical_mem - (memInfo.freeram * memInfo.mem_unit);
    long long total_swap = memInfo.totalswap * memInfo.mem_unit;
    long long used_swap = total_swap - (memInfo.freeswap * memInfo.mem_unit);

    GtkWidget* physical_memory_section = create_memory_section(total_physical_mem, used_physical_mem, "Physical Memory");
    GtkWidget* swap_memory_section = create_memory_section(total_swap, used_swap, "Swap Memory");

    gtk_box_pack_start(GTK_BOX(vbox), physical_memory_section, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), swap_memory_section, TRUE, TRUE, 0);
    /*
    struct sysinfo memInfo = get_physical_memory_info();

    
    long long total_physical_mem = memInfo.totalram * memInfo.mem_unit;
    long long used_physical_mem = total_physical_mem - (memInfo.freeram * memInfo.mem_unit);

    long long total_swap = memInfo.totalswap * memInfo.mem_unit;
    long long used_swap = total_swap - (memInfo.freeswap * memInfo.mem_unit);

    GdkRGBA red = {1.0, 0.0, 0.0, 1.0};
    GdkRGBA blue = {0.0, 0.0, 1.0, 1.0};

    GtkWidget* physical_memory_bar = create_memory_bar(total_physical_mem, used_physical_mem, "Physical Memory", red, blue);
    GtkWidget* swap_memory_bar = create_memory_bar(total_swap, used_swap, "Swap Memory", red, blue);

    gtk_box_pack_start(GTK_BOX(vbox), physical_memory_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), swap_memory_bar, TRUE, TRUE, 0); */
}


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

void add_column(GtkWidget *tree_view, const char *title, int column_id) {
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", column_id, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
}

void add_memory_tab(GtkWidget *notebook) {
   /* GdkRGBA red = {1.0, 0.0, 0.0, 1.0};
    GdkRGBA blue = {0.0, 0.0, 1.0, 1.0};
	
    GtkWidget *memory_label = gtk_label_new("Memory");
    GtkWidget *memory_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    struct sysinfo memInfo;
    sysinfo(&memInfo);

    long long total_physical = memInfo.totalram * memInfo.mem_unit;
    long long used_physical = total_physical - memInfo.freeram * memInfo.mem_unit;
    long long total_swap = memInfo.totalswap * memInfo.mem_unit;
    long long used_swap = total_swap - memInfo.freeswap * memInfo.mem_unit;

    GtkWidget *physical_memory_bar = create_memory_bar(total_physical, used_physical, "Physical Memory", red, blue);
    GtkWidget *swap_memory_bar = create_memory_bar(total_swap, used_swap, "Swap Memory", red, blue);

    gtk_box_pack_start(GTK_BOX(memory_tab), physical_memory_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(memory_tab), swap_memory_bar, TRUE, TRUE, 0);

    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_column(tree_view, "Mount Point", 0);
    add_column(tree_view, "Total Size", 1);
    add_column(tree_view, "Available Size", 2);
    add_column(tree_view, "Used Size", 3);

    int fs_count;
    FileSystemInfo* fs_info = get_file_system_info(&fs_count);
    for (int i = 0; i < fs_count; ++i) {
        char* total_str = format_size(fs_info[i].total_size);
        char* available_str = format_size(fs_info[i].available_size);
        char* used_str = format_size(fs_info[i].used_size);

        gtk_list_store_insert_with_values(store, NULL, -1,
                                          0, fs_info[i].mount_point,
                                          1, total_str,
                                          2, available_str,
                                          3, used_str,
                                          -1);

        g_free(total_str);
        g_free(available_str);
        g_free(used_str);
    }

    for (int i = 0; i < fs_count; ++i) {
        free(fs_info[i].mount_point);
    }
    free(fs_info);

    gtk_container_add(GTK_CONTAINER(scroll_window), tree_view);
    gtk_box_pack_start(GTK_BOX(memory_tab), scroll_window, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), memory_tab, memory_label);

    g_object_unref(store);

    gtk_widget_show_all(notebook);*/


    GtkWidget *memory_label = gtk_label_new("Memory");
    GtkWidget *memory_tab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    struct sysinfo memInfo;
    sysinfo(&memInfo);

    long long total_physical = memInfo.totalram * memInfo.mem_unit;
    long long free_physical = memInfo.freeram * memInfo.mem_unit;
    long long total_swap = memInfo.totalswap * memInfo.mem_unit;
    long long free_swap = memInfo.freeswap * memInfo.mem_unit;

    GtkWidget *physical_memory_section = create_memory_section(total_physical, total_physical - free_physical, "Physical Memory");
    GtkWidget *swap_memory_section = create_memory_section(total_swap, total_swap - free_swap, "Swap Memory");

    gtk_box_pack_start(GTK_BOX(memory_tab), physical_memory_section, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(memory_tab), swap_memory_section, TRUE, TRUE, 0);

    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    add_column(tree_view, "Mount Point", 0);
    add_column(tree_view, "Total Size", 1);
    add_column(tree_view, "Available Size", 2);
    add_column(tree_view, "Used Size", 3);

    int fs_count;
    FileSystemInfo* fs_info = get_file_system_info(&fs_count);
    for (int i = 0; i < fs_count; ++i) {
        char* total_str = format_size(fs_info[i].total_size);
        char* available_str = format_size(fs_info[i].available_size);
        char* used_str = format_size(fs_info[i].used_size);

        gtk_list_store_insert_with_values(store, NULL, -1,
                                          0, fs_info[i].mount_point,
                                          1, total_str,
                                          2, available_str,
                                          3, used_str,
                                          -1);

        g_free(total_str);
        g_free(available_str);
        g_free(used_str);
    }

    for (int i = 0; i < fs_count; ++i) {
        free(fs_info[i].mount_point);
    }
    free(fs_info);

    gtk_container_add(GTK_CONTAINER(scroll_window), tree_view);
    gtk_box_pack_start(GTK_BOX(memory_tab), scroll_window, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), memory_tab, memory_label);

    g_object_unref(store);

    gtk_widget_show_all(notebook);
}

void setup_css_provider() {
const char *css_style = 
  "#used_memory_bar { background-image: linear-gradient(to right, red, red); }"
  "#available_memory_bar { background-image: linear-gradient(to right, blue, blue); }";

GtkCssProvider *css_provider = gtk_css_provider_new();
gtk_css_provider_load_from_data(css_provider, css_style, -1, NULL);
gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                          GTK_STYLE_PROVIDER(css_provider),
                                          GTK_STYLE_PROVIDER_PRIORITY_USER);
}
