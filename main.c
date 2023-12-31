#include <gtk/gtk.h>
#include "gui/tabs_cpu.h"
#include "gui/tabs_memory.h"
#include "gui/tabs_process.h"
#include "gui/tabs_system.h"

void apply_css_to_widget() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css_style = 
        "progressbar#used_memory_bar {"
        "   background: red;"
        "   border-radius: 0px;"
        "   border-style: none;"
        "}"
        "progressbar#available_memory_bar {"
        "   background: blue;"
        "   border-radius: 0px;"
        "   border-style: none;"
        "}"
        "progressbar.vertical trough {"
        "   min-width: 20px;"
        "}"
        "progressbar.horizontal trough {"
        "   min-height: 20px;"
        "}";
    gtk_css_provider_load_from_data(provider, css_style, -1, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    apply_css_to_widget();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    AppData data = {0};
    AppData2 data2;

    AppData3 data3 = {0};

    add_system_tab(notebook);
    add_process_tab(notebook);
    add_cpu_tab(notebook, &data, &data2, &data3);
    add_memory_tab(notebook);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
