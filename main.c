#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <curl/curl.h>
#include "curl.h"
#include "cJSON.h"

typedef struct
{
    gpointer entry;
    gpointer response_area;
} ObjectContainer;

void set_text(GtkTextBuffer *buffer, ObjectContainer *container, char *text)
{
    GtkTextIter *iter = malloc(sizeof(GtkTextIter));
    gtk_text_buffer_get_end_iter(buffer, iter);
    gtk_text_buffer_insert(buffer, iter, text, -1);
    gtk_text_view_set_buffer(container->response_area, buffer);
}

void parse_json(cJSON *item, GtkTextBuffer *buffer, ObjectContainer *container)
{
    // Vérifier si l'élément est un objet
    if (item->type == cJSON_Object)
    {
        char *key = item->string;
        if (key != NULL)
        {
            char *ret = malloc(sizeof(char) * (strlen(key) + 20));
            sprintf(ret, "%s:\n\n", key);
            set_text(buffer, container, ret);
        }
        cJSON *subItem = item->child;
        while (subItem != NULL)
        {
            // Recuring call
            parse_json(subItem, buffer, container);
            subItem = subItem->next;
        }
    }
    // Vérifier si l'élément est un tableau
    else if (item->type == cJSON_Array)
    {
        char *key = item->string;
        if (key != NULL)
        {
            char *ret = malloc(sizeof(char) * (strlen(key) + 20));
            sprintf(ret, "%s:\n\n", key);
            set_text(buffer, container, ret);
        }
        cJSON *subItem = item->child;
        while (subItem != NULL)
        {
            // Recuring call
            parse_json(subItem, buffer, container);
            subItem = subItem->next;
        }
    }
    else
    {
        // Get key and value as string
        char *key = item->string;
        char *value = cJSON_Print(item);

        // Create the string based on the key and the value
        char *ret = malloc(sizeof(char) * (strlen(key) + strlen(value)) + 20);
        sprintf(ret, "%s: %s\n", key, value);

        // Add the response at the end.
        set_text(buffer, container, ret);
        free(value);
    }
}

static void send_request(GtkWidget *widget, gpointer data)
{
    ObjectContainer *container = (ObjectContainer *)data;
    struct StringBuffer res;
    GtkTextIter *iter = malloc(sizeof(GtkTextIter));

    // Call the API
    call(gtk_entry_get_text(container->entry), &res);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(container->response_area);

    // Manage the response
    cJSON *json = cJSON_Parse(res.data);
    parse_json(json, buffer, container);
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
