#ifndef UI_H
#define UI_H
#include <gtk/gtk.h>

void activate(GtkApplication* app, gpointer user_data);

void on_window_destroy(GtkWindow *widget, gpointer user_data);

#endif // !UI_H
