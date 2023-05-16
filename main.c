#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include "curl.h"

typedef struct
{
    gpointer entry;
    gpointer response_area;
} ObjectContainer;

static void send_request(GtkWidget *widget, gpointer data)
{
    ObjectContainer *container = (ObjectContainer *)data;
    struct StringBuffer res;
    GtkTextIter *iter = malloc(sizeof(GtkTextIter));

    // Call the API
    call(gtk_entry_get_text(container->entry), &res);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(container->response_area);

    // Manage the response
    char *ret = malloc(sizeof(char) * strlen(res.data) + 2);
    if (gtk_text_buffer_get_char_count(buffer) != 0)
        sprintf(ret, "\n%s", res.data);
    else
        sprintf(ret, "%s", res.data);

    // Add the response at the end.
    gtk_text_buffer_get_end_iter(buffer, iter);
    gtk_text_buffer_insert(buffer, iter, ret, -1);
    gtk_text_view_set_buffer(container->response_area, buffer);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "layout.glade", NULL);

    // /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "main_window");
    gtk_window_set_application(GTK_WINDOW(window), app);

    GObject *entry = gtk_builder_get_object(builder, "url_entry");
    GObject *response_area = gtk_builder_get_object(builder, "response_area");

    ObjectContainer *container = malloc(sizeof(ObjectContainer));
    container->entry = entry;
    container->response_area = response_area;

    GObject *button = gtk_builder_get_object(builder, "send_button");
    g_signal_connect(button, "clicked", G_CALLBACK(send_request), container);

    gtk_widget_set_visible(GTK_WIDGET(window), TRUE);

    /* We do not need the builder any more */
    g_object_unref(builder);
}

int main(int argc, char *argv[])
{
#ifdef GTK_SRCDIR
    g_chdir(GTK_SRCDIR);
#endif

    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
