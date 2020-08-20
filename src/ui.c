/* Name, ui.c, CS 24000, Spring 2020
 * Last updated April 9, 2020
 */

/* Add any includes here */

#include "ui.h"

tree_node_t *g_current_node = NULL;
song_data_t *g_current_song = NULL;
song_data_t *g_modified_song = NULL;

/* Define update_song_list here */

void update_song_list() {

}

/* Define update_drawing_area here */
void update_drawing_area() {

}

/* Define update_info here */
void update_info() {

}

/* Define update_song here */
void update_song() {

}

/* Define range_of_song here */
void range_of_song(song_data_t *song, int *e, int *d, int *g) {

}


/* Define activate here */

void activate(GtkApplication *app, gpointer user_data) {
  g_print("Creating App...\n");
  GtkWidget *window;
  GtkWidget *grid;

  GtkWidget *song_select_view;
  GtkWidget *add_button;
  GtkWidget *add_button_box;
  GtkWidget *load_button;
  GtkWidget *load_button_box;
  GtkWidget *song_list_view;
  GtkWidget *song_list;
  GtkWidget *search_bar;

  GtkWidget *song_alter_view;
  GtkWidget *remove_button_box;
  GtkWidget *remove_button;
  GtkWidget *save_button_box;
  GtkWidget *save_button;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "MIDI Library");
  gtk_window_set_default_size (GTK_WINDOW (window), 750, 900);

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), grid);
  song_select_view = gtk_grid_new();
  gtk_grid_attach(GTK_GRID (grid), song_select_view, 0, 0, 50, 50);
  song_alter_view = gtk_grid_new();
  gtk_grid_attach(GTK_GRID (grid), song_alter_view, 55, 0, 50, 50);
  
  add_button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  add_button = gtk_button_new_with_label ("Add Song from File");
  g_signal_connect (add_button, "clicked", G_CALLBACK (add_song_cb), NULL);
  gtk_container_add (GTK_CONTAINER (add_button_box), add_button);
  gtk_grid_attach(GTK_GRID (song_select_view), add_button_box, 0, 0, 1, 1);
  
  load_button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  load_button = gtk_button_new_with_label ("Load Song from Directory");
  g_signal_connect (load_button, "clicked", G_CALLBACK (load_songs_cb), NULL);
  gtk_container_add (GTK_CONTAINER (load_button_box), load_button);
  gtk_grid_attach_next_to(GTK_GRID (song_select_view), load_button_box, add_button_box, GTK_POS_RIGHT, 1, 1);

  song_list_view = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_vexpand (song_list_view, TRUE);
  song_list = gtk_list_box_new ();
  gtk_container_add (GTK_CONTAINER (song_list_view), song_list);
  GtkWidget *label = gtk_label_new ("Hello");
  gtk_list_box_prepend((GtkListBox *) song_list, label);
  gtk_grid_attach_next_to(GTK_GRID (song_select_view), song_list_view, add_button_box, GTK_POS_BOTTOM, 4, 8);

  search_bar = gtk_search_bar_new();
  //g_signal_connect(search_bar, "clicked", G_CALLBACK (search_bar_cb), NULL);
  gtk_grid_attach_next_to(GTK_GRID (song_select_view), search_bar, song_select_view, GTK_POS_BOTTOM, 4, 2);

  remove_button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  remove_button = gtk_button_new_with_label ("Remove Song");
  g_signal_connect (remove_button, "clicked", G_CALLBACK (remove_song_cb), NULL);
  gtk_container_add (GTK_CONTAINER (remove_button_box), remove_button);
  gtk_grid_attach (GTK_GRID (song_alter_view), remove_button_box, 0, 0, 1, 1);

  save_button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  save_button = gtk_button_new_with_label ("Save Song");
  g_signal_connect (save_button, "clicked", G_CALLBACK (save_song_cb), NULL);
  gtk_container_add (GTK_CONTAINER (save_button_box), save_button);
  gtk_grid_attach (GTK_GRID (song_alter_view), save_button_box, 2, 0, 1, 1);

  gtk_widget_show_all (window);
  g_print("App created\n");
}

void add_song_cb(GtkButton *widget, gpointer data) {
  g_print("Add song called\n");
  printf("print asdsad\n");
}
void load_songs_cb(GtkButton *WIDGET, gpointer data) {

}
void song_selected_cb(GtkListBox *widget, GtkListBoxRow *data) {

}
void search_bar_cb(GtkSearchBar *widget, gpointer data) {

}
void time_scale_cb(GtkSpinButton *widget, gpointer data) {

}
gboolean draw_cb(GtkDrawingArea *widget, cairo_t *qwe, gpointer data) {
  return FALSE;
}
void warp_time_cb(GtkSpinButton *widget, gpointer data) {

}
void song_octave_cb(GtkSpinButton *widget, gpointer data) {

}
void instrument_map_cb(GtkComboBoxText *widget, gpointer data) {

}
void note_map_cb(GtkComboBoxText *widget, gpointer data) {
  
}
  
void save_song_cb(GtkButton *widget, gpointer data) {

}
void remove_song_cb(GtkButton *widget, gpointer data) {

}

/*
 * Function called prior to main that sets up the instrument to color mapping
 */

void build_color_palette()
{
  static GdkRGBA palette[16];	

  memset(COLOR_PALETTE, 0, sizeof(COLOR_PALETTE));
  char* color_specs[] = {
    // Piano, red
    "#ff0000",
    // Chromatic percussion, brown
    "#8b4513",
    // Organ, purple
    "#800080",
    // Guitar, green
    "#00ff00",
    // Bass, blue
    "#0000ff",
    // Strings, cyan
    "#00ffff",
    // Ensemble, teal
    "#008080",
    // Brass, orange
    "#ffa500",
    // Reed, magenta
    "#ff00ff",
    // Pipe, yellow
    "ffff00",
    // Synth lead, indigo
    "#4b0082",
    // Synth pad, dark slate grar
    "#2f4f4f",
    // Synth effects, silver
    "#c0c0c0",
    // Ehtnic, olive
    "#808000",
    // Percussive, silver
    "#c0c0c0",
    // Sound effects, gray
    "#808080",
  };

  for (int i = 0; i < 16; ++i) {
    gdk_rgba_parse(&palette[i], color_specs[i]);
    for (int j = 0; j < 8; ++j) {
      COLOR_PALETTE[i * 8 + j] = &palette[i];
    }
  }
} /* build_color_palette() */
