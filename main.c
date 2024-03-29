#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "curl.h"
#include "cJSON.h"

typedef struct
{
    gpointer entry;
    gpointer response_area;
    gpointer save_output;
    gpointer key_entry;
    gpointer value_entry;
} ObjectContainer;

// Reset the text in the response area
void reset_text(GtkTextBuffer *buffer, ObjectContainer *container)
{
    gtk_text_buffer_set_text(buffer, "", -1);
    gtk_text_view_set_buffer(container->response_area, buffer);
}

// Add text at the end of the response area
void set_text(GtkTextBuffer *buffer, ObjectContainer *container, char *text)
{
    GtkTextIter *iter = malloc(sizeof(GtkTextIter));
    gtk_text_buffer_get_end_iter(buffer, iter);
    gtk_text_buffer_insert(buffer, iter, text, -1);
    gtk_text_view_set_buffer(container->response_area, buffer);
}

// Parse the json and add the response in the response area
void parse_json(cJSON *item, GtkTextBuffer *buffer, ObjectContainer *container, int indent)
{
    char *space = malloc(sizeof(char) * indent + 1);
    for (int i = 0; i < indent; i++)
    {
        space[i] = '\t';
    }
    space[indent] = '\0';

    // Vérifier si l'élément est un objet
    if (item->type == cJSON_Object || item->type == cJSON_Array)
    {
        char *key = item->string;
        if (key != NULL)
        {
            char *ret = malloc(sizeof(char) * (strlen(key) + 20));
            sprintf(ret, "%s%s:\n", space, key);
            set_text(buffer, container, ret);
        }
        cJSON *subItem = item->child;
        while (subItem != NULL)
        {
            // Recuring call
            parse_json(subItem, buffer, container, indent + 1);
            subItem = subItem->next;
        }
        set_text(buffer, container, "");
    }
    else
    {
        // Get key and value as string
        char *key = item->string;
        char *value = cJSON_Print(item);

        // Create the string based on the key and the value
        char *ret = malloc(sizeof(char) * (strlen(key) + strlen(value)) + 20);
        sprintf(ret, "%s%s: %s\n", space, key, value);

        // Add the response at the end.
        set_text(buffer, container, ret);
        free(value);
    }
}

// Send the request and manage the response
static void send_request(GtkWidget *widget, gpointer data)
{
    ObjectContainer *container = (ObjectContainer *)data;
    struct StringBuffer res;
    GtkTextIter *iter = malloc(sizeof(GtkTextIter));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(container->response_area);

    // Here i reset so everything looks clean in the response area.
    reset_text(buffer, container);

    // Call the API and check for error
    if (strlen(gtk_entry_get_text(container->entry)) == 0)
    {
        set_text(buffer, container, "Please enter an url !\n");
        return;
    }
    if (call(gtk_entry_get_text(container->entry), &res) == 0)
    {
        set_text(buffer, container, "Error while calling the API ! Please verify the URL.\n");
        return;
    }

    // Manage the response
    cJSON *json = cJSON_Parse(res.data);

    // Save the response in a file if the checkbox is checked, else show
    if (gtk_toggle_button_get_active(container->save_output))
    {
        FILE *file = fopen("output.json", "w");
        if (file == NULL)
        {
            set_text(buffer, container, "Error while opening the file\n");
            return;
        }
        else
        {
            set_text(buffer, container, "Response saved in ./output.json\n");
            fprintf(file, "%s", cJSON_Print(json));
            fclose(file);
        }
    }
    else
    {
        parse_json(json, buffer, container, -1);
    }
    cJSON_Delete(json);
    free(res.data);
}

// Add the key and the value to the url
static void add_to_url(GtkWidget *widget, gpointer data)
{
    ObjectContainer *container = (ObjectContainer *)data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(container->response_area);

    // Here i reset so everything looks clean in the response area.
    reset_text(buffer, container);

    // Check if the url and the key/value are not empty
    if (strlen(gtk_entry_get_text(container->entry)) == 0)
    {
        set_text(buffer, container, "Please enter an url !\n");
    }
    else if (strlen(gtk_entry_get_text(container->key_entry)) == 0 || strlen(gtk_entry_get_text(container->value_entry)) == 0)
    {
        set_text(buffer, container, "Please enter a key and a value !\n");
    }
    else
    {
        char *final = NULL;
        const char *url = gtk_entry_get_text(container->entry);
        const char *key = gtk_entry_get_text(container->key_entry);
        const char *value = gtk_entry_get_text(container->value_entry);

        // Check if the url already contains a parameter
        if (strstr(url, "?") != NULL)
        {
            final = malloc(sizeof(char) * (strlen(url) + strlen(key) + strlen(value) + 3));
            sprintf(final, "%s&%s=%s", url, key, value);
        }
        else
        {
            final = malloc(sizeof(char) * (strlen(url) + strlen(key) + strlen(value) + 3));
            sprintf(final, "%s?%s=%s", url, key, value);
        }

        // Set the new url
        gtk_entry_set_text(container->entry, final);
        gtk_entry_set_text(container->key_entry, "");
        gtk_entry_set_text(container->value_entry, "");
        set_text(buffer, container, "Key and value added to the url !\n");
    }
}

// Create the window and connect the signals
static void activate(GtkApplication *app, gpointer user_data)
{
    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "layout.glade", NULL);

    // /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "main_window");
    gtk_window_set_title(GTK_WINDOW(window), "Cook Master REST Client");
    gtk_window_set_application(GTK_WINDOW(window), app);

    GObject *entry = gtk_builder_get_object(builder, "url_entry");
    GObject *response_area = gtk_builder_get_object(builder, "response_area");
    GObject *checbox = gtk_builder_get_object(builder, "save_output");
    GObject *key_entry = gtk_builder_get_object(builder, "key_entry");
    GObject *value_entry = gtk_builder_get_object(builder, "value_entry");

    ObjectContainer *container = malloc(sizeof(ObjectContainer));
    container->entry = entry;
    container->response_area = response_area;
    container->save_output = checbox;
    container->key_entry = key_entry;
    container->value_entry = value_entry;

    GObject *button = gtk_builder_get_object(builder, "send_button");
    g_signal_connect(button, "clicked", G_CALLBACK(send_request), container);

    button = gtk_builder_get_object(builder, "add_url_button");
    g_signal_connect(button, "clicked", G_CALLBACK(add_to_url), container);

    gtk_widget_set_visible(GTK_WIDGET(window), TRUE);

    /* We do not need the builder any more */
    g_object_unref(builder);
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
