#include "tabs_cpu.h"
#include <gtk/gtk.h>
#include <dirent.h>

const gchar* get_thread_cpu_usage_string(pid_t tid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/self/task/%d/stat", tid);

    FILE* fp = fopen(path, "r");

    if (fp == NULL) {
        perror("Error opening thread stat file");
        return "Error";
    }

    unsigned long utime, stime;
    if (fscanf(fp, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %lu %lu", &utime, &stime) != 2) {
        perror("Error reading thread stat file");
        fclose(fp);
        return "Error";
    }

    fclose(fp);

    unsigned long total_time = utime + stime;
    double seconds = 1.0;
    double cpu_usage = 100.0 * total_time / (sysconf(_SC_CLK_TCK) * seconds);

    static gchar thread_usage_str[50];
    g_snprintf(thread_usage_str, sizeof(thread_usage_str), "Thread %d Usage: %.2f%%", tid, cpu_usage);

    return thread_usage_str;
}

void update_thread_cpu_usage(GtkLabel *label) {
    DIR* task_dir = opendir("/proc/self/task");
    if (task_dir == NULL) {
        perror("Error opening task directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(task_dir)) != NULL) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            pid_t tid = atoi(entry->d_name);
            const gchar *thread_usage_str = get_thread_cpu_usage_string(tid);
            gtk_label_set_text(label, thread_usage_str);
            g_usleep(100000);
        }
    }

    closedir(task_dir);
}

gboolean update_timer_callback(GtkLabel *label) {
    update_thread_cpu_usage(label);
    return TRUE;
}

void add_cpu_tab(GtkWidget *notebook) {
    GtkWidget *cpu_label = gtk_label_new("CPU");

    GtkWidget *cpu_tab = gtk_label_new("Initializing...");
    GtkLabel *cpu_label_widget = GTK_LABEL(cpu_tab);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpu_tab, cpu_label);

    guint timer_id = g_timeout_add_seconds(1, (GSourceFunc)update_timer_callback, cpu_label_widget);
}
