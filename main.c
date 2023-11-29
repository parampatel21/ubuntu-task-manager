#include <gtk/gtk.h>
#include "gui/tabs_cpu.h"
#include "gui/tabs_memory.h"
#include "gui/tabs_process.h"
#include "gui/tabs_system.h"



void apply_css_to_widget(GtkWidget *widget);

void apply_css_to_widget(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "progressbar#used_memory_bar { background-color: red; } "
        "progressbar#available_memory_bar { background-color: blue; } ", -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    setup_css_provider();


    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // Add tabs
    add_cpu_tab(notebook);
    add_memory_tab(notebook);
    add_process_tab(notebook);
    add_system_tab(notebook);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
