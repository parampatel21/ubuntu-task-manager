#include "tabs_system.h"
#include <gtk/gtk.h>

void add_system_tab(GtkWidget *notebook) {
    GtkWidget *system_label = gtk_label_new("System");

//    get OS details
    FILE *fp;
    char OS_name[50] = " ";
    fp = popen("lsb_release -ds", "r");
    fgets(OS_name, 50, fp);
    pclose(fp);
    GtkWidget *system_tab = gtk_label_new(OS_name);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), system_tab, system_label);
}
