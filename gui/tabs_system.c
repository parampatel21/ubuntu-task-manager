#include "tabs_system.h"
#include <gtk/gtk.h>
#include <sys/sysinfo.h>
#include <stdio.h>

void add_system_tab(GtkWidget *notebook) {
    GtkWidget *system_label = gtk_label_new("System");
    GtkWidget *system_tab = gtk_label_new("");

    // Gather system information
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        perror("sysinfo");
        return;
    }

    // Create a string to display system information
    char info_str[512];
    snprintf(info_str, sizeof(info_str),
             "OS Release: %s\nKernel Version: %s\nMemory: %lu KB\nProcessor: %s\nDisk Storage: %lu KB",
             g_unix_get_os_info(), g_unix_get_kernel_version(), si.totalram * si.mem_unit / 1024,
             g_cpu_info_get_model(), si.totalswap * si.mem_unit / 1024);

    gtk_label_set_text(GTK_LABEL(system_tab), info_str);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), system_tab, system_label);
}
