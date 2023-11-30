#include <gtk/gtk.h>
#include <libgtop-2.0/glibtop.h>
#include <libgtop-2.0/glibtop/cpu.h>

#define MAX_CORES 32  // Maximum number of CPU cores

glibtop_cpu cpu[MAX_CORES];
GtkWidget *drawing_area;
guint timer_id;

static gboolean update_graph(gpointer data) {
    gint width, height;
    GdkPixbuf *pixbuf;
    GdkPixbuf *scaled_pixbuf;

    width = gtk_widget_get_allocated_width(drawing_area);
    height = gtk_widget_get_allocated_height(drawing_area);

    pixbuf = gdk_pixbuf_get_from_window(gtk_widget_get_window(drawing_area), 0, 0, width, height);
    scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);

    gtk_image_set_from_pixbuf(GTK_IMAGE(drawing_area), scaled_pixbuf);

    g_object_unref(pixbuf);
    g_object_unref(scaled_pixbuf);

    return G_SOURCE_CONTINUE;
}

static gboolean draw_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
    guint i, j;
    guint width, height;
    guint num_cores;
    gfloat usage[MAX_CORES];

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    num_cores = glibtop_get_cpu_count(&cpu[0]);

    // Clear the drawing area
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    // Draw CPU usage graph
    for (i = 0; i < num_cores; i++) {
        glibtop_get_cpu(&cpu[i]);
        usage[i] = cpu[i].xcpu_user + cpu[i].xcpu_nice + cpu[i].xcpu_sys + cpu[i].xcpu_idle;
    }

    for (i = 0; i < num_cores; i++) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_move_to(cr, 0, height - (height * usage[i]));
        for (j = 0; j < width; j++) {
            cairo_line_to(cr, j, height - (height * usage[i]));
        }
        cairo_stroke(cr);
    }

    return FALSE;
}

static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data) {
    gtk_widget_queue_draw(widget);
    return TRUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;

    gtk_init(&argc, &argv);

    // Initialize libgtop
    glibtop_init();

    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "CPU Monitor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box to hold the drawing area
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a drawing area
    drawing_area = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    // Connect signals
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_graph), NULL);
    g_signal_connect(G_OBJECT(window), "configure-event", G_CALLBACK(on_configure_event), NULL);

    // Set up a timer to update the graph periodically
    timer_id = g_timeout_add(1000, update_graph, NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
