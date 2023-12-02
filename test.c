#include <gtk/gtk.h>

// Structure to hold data for the line chart
typedef struct {
    GtkWidget *drawing_area;
    double data[60]; // Assuming you want to display the last 60 seconds of data
} AppData;

// Function to update the line chart with new data
void update_chart(AppData *data, double new_value) {
    // Shift existing data to the left
    memmove(data->data, data->data + 1, sizeof(double) * (59));

    // Add new value to the end
    data->data[59] = new_value;

    // Queue a redraw of the drawing area
    gtk_widget_queue_draw(data->drawing_area);
}

// Callback to draw the line chart
gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData *data = (AppData *)user_data;

    // Set up drawing context
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);

    // Calculate the width and height of the drawing area
    int width, height;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    // Add padding
    int padding = 70;
    width -= 2 * padding;
    height -= 2 * padding;

    // Calculate the spacing between data points
    double spacing = (double)width / 60;

    // Draw the axes
    cairo_move_to(cr, padding, padding);
    cairo_line_to(cr, padding, height + padding);
    cairo_line_to(cr, width + padding, height + padding);
    cairo_stroke(cr);

    // Draw the line chart
    cairo_move_to(cr, padding, height + padding - data->data[0] * height);
    for (int i = 1; i < 60; ++i) {
        cairo_line_to(cr, i * spacing + padding, height + padding - data->data[i] * height);
    }

    // Stroke the path
    cairo_stroke(cr);

    // Draw x-axis numbers
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);
    for (int i = 0; i <= 60; i += 10) {
        char label[4];
        snprintf(label, sizeof(label), "%d", 60 - i);
        cairo_move_to(cr, i * spacing + padding - 5, height + padding + 20);
        cairo_show_text(cr, label);
    }

    // Draw y-axis numbers
    for (int i = 0; i <= 100; i += 10) {
        double y = (100 - i) * height / 100 + padding;
        char label[4];
        snprintf(label, sizeof(label), "%d", i);
        cairo_move_to(cr, padding - 30, y + 5);
        cairo_show_text(cr, label);
    }

    return FALSE;
}

// Function to update data every second (for testing purposes)
gboolean update_data(gpointer user_data) {
    AppData *data = (AppData *)user_data;

    char str[100];
    const char d[2] = " ";
    char* token;
    int i = 0,times,lag;
    long int sum = 0, idle, lastSum = 0,lastIdle = 0;
    long double idleFraction;

    FILE* fp = fopen("/proc/stat","r");
    i = 0;
    fgets(str,100,fp);
    fclose(fp);
    token = strtok(str,d);
    sum = 0;
    while(token!=NULL){
        token = strtok(NULL,d);
        if(token!=NULL){
            sum += atoi(token);

            if(i==3)
                idle = atoi(token);

            i++;
        }
    }
    idleFraction = 100 - (idle-lastIdle)*100.0/(sum-lastSum);

    lastIdle = idle;
    lastSum = sum;

    // Update the chart with the new value
    update_chart(data, idleFraction);

    return TRUE; // Continue the timer
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Line Chart Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create the drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    // Set up the data structure
    AppData data;
    data.drawing_area = drawing_area;

    // Connect the draw signal
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &data);

    // Set up a timer to update data every second
    g_timeout_add_seconds(1, update_data, &data);

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}