#include "ui.h"
#include "gtk/gtkcssprovider.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  GtkEntry *entry;
  GtkBox *messages_box;
  int sockfd;
  int clientfd;
  gboolean is_server;
  guint socket_source_id;
} AppWidgets;

static void add_message_to_ui(AppWidgets *widgets, const char *text, gboolean is_sent)
{
  GtkWidget *message_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_widget_add_css_class(message_container, "message-box");

  if (is_sent)
  {
    gtk_widget_add_css_class(message_container, "sent-message");
  }

  GtkWidget *label = gtk_label_new(text);
  gtk_box_append(GTK_BOX(message_container), label);
  gtk_box_append(GTK_BOX(widgets->messages_box), message_container);
  gtk_widget_set_visible(GTK_WIDGET(widgets->messages_box), TRUE);
}

static gboolean handle_socket_data(GIOChannel *source, GIOCondition condition, gpointer user_data)
{
  AppWidgets *widgets = (AppWidgets *)user_data;
  char buffer[256] = {0};
  int fd = widgets->is_server ? widgets->clientfd : widgets->sockfd;

  if (condition & G_IO_IN)
  {
    ssize_t bytes_recieved = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_recieved < 0)
    {
      g_print("Connection closed.\n");
      return FALSE;
    }

    buffer[bytes_recieved] = '\0';
    add_message_to_ui(widgets, buffer, FALSE);
    return TRUE;
  }
  return FALSE;
}

static void send_message(AppWidgets* widgets, const char *text) 
{
  int fd = widgets->is_server ? widgets->clientfd : widgets->sockfd;
  send(fd, text, strlen(text), 0);
  add_message_to_ui(widgets, text, TRUE);
}

void load_css(void) 
{
  GtkCssProvider *provider = gtk_css_provider_new();
  GFile *css_file = g_file_new_for_path("style.css");

  gtk_css_provider_load_from_file(provider, css_file);

  gtk_style_context_add_provider_for_display(
    gdk_display_get_default(),
    GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );
}


void send_button_click(GtkButton *button, gpointer user_data) 
{
  AppWidgets *widgets = (AppWidgets *) user_data;
  GtkEntryBuffer *buf = gtk_entry_get_buffer(widgets->entry);
  const char *text = gtk_entry_buffer_get_text(buf);

  if (strlen(text) == 0) return;

  send_message(widgets, text);

  gtk_entry_buffer_set_text(buf, "", 0);
}

static gboolean accept_connection(gpointer user_data)
{
  AppWidgets *widgets = (AppWidgets*)user_data;

  widgets->clientfd = accept(widgets->sockfd, NULL, NULL);
  if (widgets->clientfd >= 0) {
    g_print("Client connected.\n");
    GIOChannel *channel = g_io_channel_unix_new(widgets->clientfd);
    widgets->socket_source_id = g_io_add_watch(channel, G_IO_IN | G_IO_HUP, handle_socket_data, widgets);
    g_io_channel_unref(channel);

    return G_SOURCE_REMOVE;
  }
  return G_SOURCE_CONTINUE;
}

static void start_server(GtkButton *button, gpointer user_data)
{
  g_idle_add(accept_connection, (AppWidgets*)user_data);
}

static gboolean initialize_network(AppWidgets *widgets)
{
  struct sockaddr_in addr;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    g_print("Socket creation failed.\n");
    return FALSE;
  }

  widgets->sockfd = sockfd;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
  {
    widgets->is_server = FALSE;
    g_print("Connected to server.\n");

    GIOChannel *channel = g_io_channel_unix_new(sockfd);
    widgets->socket_source_id = g_io_add_watch(channel, G_IO_IN | G_IO_HUP, handle_socket_data, widgets);
    return TRUE;
  }

  g_print("Starting server.\n");
  widgets->is_server = TRUE;

  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    g_print("Bind failed.\n");
    return FALSE;
  }

  if (listen(sockfd, 3) < 0)
  {
    g_print("Listen failed.\n");
    return FALSE;
  }

  g_print("Ready to accept connections");

  return TRUE;
}

void cleanup(AppWidgets *widgets) 
{

  if (widgets->socket_source_id > 0) g_source_remove(widgets->socket_source_id);
  if (widgets->clientfd != -1) close(widgets->clientfd);
  if (widgets->sockfd != -1) close(widgets->sockfd);

  g_free(widgets);
}

void activate(GtkApplication* app, gpointer user_data)
{


  load_css();
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Chat App");
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
  gtk_window_present(GTK_WINDOW(window));

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  // Scrollable window for messages
  GtkWidget *scrolled_window = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_hexpand(scrolled_window, TRUE);
  gtk_widget_set_vexpand(scrolled_window, TRUE);

  // Box for holding messages
  GtkWidget *messages_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), messages_box);
  gtk_box_append(GTK_BOX(vbox), scrolled_window);

  GtkWidget *accept_btn = gtk_button_new_with_label("Start accepting");
  gtk_box_append(GTK_BOX(vbox), accept_btn);

  GtkWidget *entry = gtk_entry_new();
  gtk_box_append(GTK_BOX(vbox), entry);
  gtk_widget_set_valign(entry, GTK_ALIGN_END);

  GtkWidget *send_button = gtk_button_new_with_label("Send");
  gtk_box_append(GTK_BOX(vbox), send_button);
  gtk_widget_set_valign(send_button, GTK_ALIGN_END);

  AppWidgets *widgets = g_new(AppWidgets, 1);
  widgets->entry = GTK_ENTRY(entry);
  widgets->messages_box = GTK_BOX(messages_box);
  widgets->sockfd = -1;
  widgets->clientfd = -1;
  widgets->socket_source_id = 0;

  if (!initialize_network(widgets))
  {
    g_print("Failed to initialize network.\n");
    cleanup(widgets);
    return;
  }

  g_signal_connect(accept_btn, "clicked", G_CALLBACK(start_server), widgets);
  g_signal_connect(entry, "activate", G_CALLBACK(send_button_click), widgets);
  g_signal_connect(send_button, "clicked", G_CALLBACK(send_button_click), widgets);

  GtkWidget *close_btn = gtk_button_new_with_label("Close");
  g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_close), window);
  gtk_box_append(GTK_BOX(vbox), close_btn);

  gtk_window_set_child(GTK_WINDOW(window), vbox);

  g_signal_connect(window, "destroy", G_CALLBACK(cleanup), widgets);
}
