/* Name, alterations.c, CS 24000, Spring 2020
 * Last updated April 9, 2020
 */

/* Add any includes here */

#include "alterations.h"

#include <string.h>

#define ABS(x) ((x > 0)? (x) : (-x))

int apply_to_events(song_data_t *song, event_func_t event_function, void *data) {
  int func_sum = 0;
  track_node_t *track_node = song->track_list;
  while (track_node) {
    event_node_t *event_node = track_node->track->event_list;
    int change_in_length = 0;
    while (event_node) {
      change_in_length += event_function(event_node->event, data);
      event_node = event_node->next_event;
    }
    if ((event_func_t *) event_function == (event_func_t *) change_event_time) {
      track_node->track->length += change_in_length;
    }
    func_sum += change_in_length;
    track_node = track_node->next_track;
  }
  return func_sum;
}

int change_event_octave(event_t *event, int *num_of_octave) {
  if (event_type(event) == MIDI_EVENT_T) {
    if (!strcmp(event->midi_event.name, "Note On")) {
      int note_change = *num_of_octave * OCTAVE_STEP;
      if (event->midi_event.data[0] + note_change < 127) {
        if (event->midi_event.data[0] + note_change > 0) {
          event->midi_event.data[0] += note_change;
          return 1;
        }
      }
    }
    if (!strcmp(event->midi_event.name, "Note Off")) {
      int note_change = *num_of_octave * OCTAVE_STEP;
      if (event->midi_event.data[0] + note_change < 127) {
        if (event->midi_event.data[0] + note_change > 0) {
          event->midi_event.data[0] += note_change;
          return 1;
        }
      }
    }
    if (!strcmp(event->midi_event.name, "Polyphonic Key")) {
      int note_change = *num_of_octave * OCTAVE_STEP;
      if (event->midi_event.data[0] + note_change < 127) {
        if (event->midi_event.data[0] + note_change > 0) {
          event->midi_event.data[0] += note_change;
          return 1;
        }
      }
    }
  }
  return 0;
}

int change_event_time(event_t *event, float *time) {
  if (*time == 1) {
    return 0;
  }
  uint32_t old_time = event->delta_time;
  event->delta_time *= (*time);
  if (ABS(event->delta_time - old_time) <= (1 << 7)) {
    return 1;
  }
  if (ABS(event->delta_time - old_time) <= (1 << 15)) {
    return 2;
  }
  if (ABS(event->delta_time - old_time) <= (1 << 23)) {
    return 3;
  }
  if (ABS(event->delta_time - old_time) <= (1 << 31)) {
    return 4;
  }
  return 0;
}

int change_event_instrument(event_t *event, remapping_t instrument_map) {

  if (event_type(event) == MIDI_EVENT_T) {
    if (!strcmp(event->midi_event.name, "Program Change")) {
      event->midi_event.data[0] = instrument_map[event->midi_event.data[0]];
      return 1;
    }
  }
  return 0;
}

int change_event_note(event_t *event, remapping_t note_map) {
if (event_type(event) == MIDI_EVENT_T) {
    // 1000, 1001, 1010
    if (!strcmp(event->midi_event.name, "Note On")) {
      event->midi_event.data[0] = note_map[event->midi_event.data[0]];
      return 1;
    }
    if (!strcmp(event->midi_event.name, "Note Off")) {
      event->midi_event.data[0] = note_map[event->midi_event.data[0]];
      return 1;
    }
    if (!strcmp(event->midi_event.name, "Polyphonic Key")) {
      event->midi_event.data[0] = note_map[event->midi_event.data[0]];
      return 1;
    }
  }
  return 0;
}

int change_octave(song_data_t *song, int num_octaves) {
  event_func_t octave_change = (event_func_t) change_event_note;
  return apply_to_events(song, octave_change, &num_octaves);
}


int warp_time(song_data_t *song, float time) {
  event_func_t change_time = (event_func_t) change_event_time;
  return apply_to_events(song, change_time, &time);
}

int remap_instruments(song_data_t *song, remapping_t instrument_table) {
  event_func_t instrument_change = (event_func_t) change_event_instrument;
  return apply_to_events(song, instrument_change, instrument_table);
}

int remap_notes(song_data_t *song, remapping_t note_table) {
  event_func_t note_change = (event_func_t) change_event_note;
  return apply_to_events(song, note_change, note_table);
}

void add_round(song_data_t *song, int track, int octave, unsigned int time, uint8_t instrument) {

}

/*
 * Function called prior to main that sets up random mapping tables
 */

void build_mapping_tables()
{
  for (int i = 0; i <= 0xFF; i++) {
    I_BRASS_BAND[i] = 61;
  }

  for (int i = 0; i <= 0xFF; i++) {
    I_HELICOPTER[i] = 125;
  }

  for (int i = 0; i <= 0xFF; i++) {
    N_LOWER[i] = i;
  }
  //  Swap C# for C
  for (int i = 1; i <= 0xFF; i += 12) {
    N_LOWER[i] = i-1;
  }
  //  Swap F# for G
  for (int i = 6; i <= 0xFF; i += 12) {
    N_LOWER[i] = i+1;
  }
} /* build_mapping_tables() */
