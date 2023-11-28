#include "tabs_system.h"
#include <gtk/gtk.h>
#include <glib.h>

void add_system_tab(GtkWidget *notebook) {
    GtkWidget *system_label = gtk_label_new("System");

    // Get OS details
    FILE *os_fp;
    char OS_name[50] = " ";
    os_fp = popen("hostnamectl | grep \"Operating System\"", "r");
    fgets(OS_name, 50, os_fp);
    pclose(os_fp);

    FILE *kernel_fp;
    char kernel[50] = " ";
    kernel_fp = popen("hostnamectl | grep \"Kernel\" | sed -e 's/^[ \\t]*//'", "r");
    fgets(kernel, 50, kernel_fp);
    pclose(kernel_fp);

    FILE *mem_fp;
    char mem_info[50] = " ";
    mem_fp = popen("cat /proc/meminfo | grep MemTotal", "r");
    fgets(mem_info, 50, mem_fp);
    pclose(mem_fp);

    long total_memory_kb;
    sscanf(mem_info, "MemTotal: %ld kB", &total_memory_kb);

    double total_memory_gb = total_memory_kb / (1024.0 * 1024.0);

    FILE *disk_fp;
    char disk_info[50] = " ";
    disk_fp = popen("df --total | grep total | awk '{print $2}'", "r");
    fgets(disk_info, 50, disk_fp);
    pclose(disk_fp);

    double available_disk_gb = atof(disk_info) / (1024.0 * 1024.0);

    GtkWidget *os_label = gtk_label_new(OS_name);
    GtkWidget *kernel_label = gtk_label_new(kernel);

    FILE *cpu_fp;
    char cpu_info[100] = " ";
    cpu_fp = popen("lscpu | grep 'Model name:'", "r");
    fgets(cpu_info, 100, cpu_fp);
    pclose(cpu_fp);

    char *trimmed_cpu_info = g_strstrip(cpu_info + strlen("Model name:"));
    char cpu_label_text[100];
    snprintf(cpu_label_text, sizeof(cpu_label_text), "Processor: %s", trimmed_cpu_info);
    GtkWidget *cpu_label = gtk_label_new(cpu_label_text);

    char memory_label_text[100];
    snprintf(memory_label_text, sizeof(memory_label_text), "Memory: %.2f GiB", total_memory_gb);
    GtkWidget *memory_label = gtk_label_new(memory_label_text);

    char disk_label_text[100];
    snprintf(disk_label_text, sizeof(disk_label_text), "Available Disk Space: %.2f GiB", available_disk_gb);
    GtkWidget *disk_label = gtk_label_new(disk_label_text);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<b>System Information</b>");

    gtk_box_pack_start(GTK_BOX(box), header, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(box), os_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), kernel_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), memory_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), cpu_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), disk_label, FALSE, FALSE, 0);


    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, system_label);
}

