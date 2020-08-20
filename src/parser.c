/* Name, parser.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#include "parser.h"

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

#define MASK_BIT_15 (1 << 15)
#define MASK_BIT_7 (1 << 7)
#define SHIFT_7 (7)
#define SHIFT_8 (8)
#define BITS_7 (0b01111111)

bool g_last_event_midi = false;
midi_event_t g_last_midi = {0};

/*
 * This function reads in data
 */

int read_data(void *variable_ptr, int size, int num, FILE *file_ptr) {
  if (fread(variable_ptr, size, num, file_ptr) == 0) {
    return -1;
  }
  return 0;
} /* read_data() */


/*
 * This function creates a new event_node
 */

event_node_t *create_event_node(FILE *song_file_ptr) {
  event_node_t *event_node = malloc(sizeof(event_node_t));
  event_node->next_event = NULL;
  assert(event_node->event);
  event_node->event = parse_event(song_file_ptr);
  return event_node;
} /* create_event_node() */

/*
 * This function parses the midi file
 * and stores the data in a song_data_t
 */

song_data_t *parse_file(const char *file) {
  assert(file);
  FILE *song_file_ptr = fopen(file, "r");
  assert(song_file_ptr);
  song_data_t *song_data = malloc(sizeof(song_data_t));
  assert(song_data);
  song_data->path = malloc(sizeof(char) * (strlen(file) + 1));
  assert(song_data->path);
  strcpy(song_data->path, file);

  fseek(song_file_ptr, 0, SEEK_END);
  int total_file_length = ftell(song_file_ptr);
  fseek(song_file_ptr, 0, SEEK_SET);

  parse_header(song_file_ptr, song_data);
  parse_track(song_file_ptr, song_data);

  assert(total_file_length == ftell(song_file_ptr));
  fclose(song_file_ptr);
  song_file_ptr = NULL;
  return song_data;
} /* parse_file() */

/*
 * Parses the header of midi file
 */

void parse_header(FILE *song_file_ptr, song_data_t *song_data) {
  char chunk_type[4] = {0};
  for (int i = 0; i < strlen("MThd"); i++) {
    read_data(chunk_type + i, sizeof(char), 1, song_file_ptr);
  }
  assert(!strcmp(chunk_type, "MThd"));

  uint32_t length = 0;
  read_data(&length, sizeof(length), 1, song_file_ptr);
  length = end_swap_32((uint8_t *)&length);
  assert(length == 0x6);

  uint16_t format = 0;
  read_data(&format, sizeof(format), 1, song_file_ptr);
  format = end_swap_16((uint8_t *)&format);
  assert(format >= 0);
  assert(format < 3);

  uint16_t ntrks = 0;
  read_data(&ntrks, sizeof(ntrks), 1, song_file_ptr);
  ntrks = end_swap_16((uint8_t *)&ntrks);
  assert(ntrks < 32768);

  uint16_t division = 0;
  read_data(&division, sizeof(division), 1, song_file_ptr);
  division = end_swap_16((uint8_t *)&division);
  assert(division < 65536);

  song_data->format = format;
  song_data->num_tracks = ntrks;
  song_data->division.uses_tpq = !((MASK_BIT_15 & division));

  if (song_data->division.uses_tpq) {
    (song_data->division).ticks_per_qtr = division;
  }
  else {
    division = division >> 1;
    uint8_t *division_bytes = (uint8_t *)&division;
    song_data->division.ticks_per_frame = division_bytes[1];
    song_data->division.frames_per_sec = division_bytes[0];
  }
} /* parse_header() */

/*
 * Parses all the tracks
 */

void parse_track(FILE *song_file_ptr, song_data_t *song) {

  song->track_list = malloc(sizeof(track_node_t));
  assert(song->track_list);
  track_node_t *track_ptr = song->track_list;

  for (int i = 0; i < song->num_tracks; i++) {
    char chunk_type[4] = {0};
    for (int i = 0; i < strlen("MTrk"); i++) {
      read_data(chunk_type + i, sizeof(char), 1, song_file_ptr);
    }
    assert(!strcmp(chunk_type, "MTrk"));

    uint32_t length = 0;
    read_data(&length, sizeof(length), 1, song_file_ptr);
    length = end_swap_32((uint8_t *)&length);

    track_ptr->track = malloc(sizeof(track_t));
    assert(track_ptr->track);
    track_ptr->track->length = length;
    track_ptr->next_track = NULL;

    event_node_t *event_ptr_head = NULL;
    bool read_events = true;
    while (read_events) {
      event_node_t *event_ptr = event_ptr_head;
      if (!event_ptr_head) {
        event_ptr_head = create_event_node(song_file_ptr);
        if (event_type(event_ptr_head->event) == META_EVENT_T) {
          if (event_ptr_head->event->meta_event.name == META_TABLE[0x2f].name) {
            read_events = false;
          }
        }
      }
      else {
        while (event_ptr->next_event != NULL) {
          event_ptr = event_ptr->next_event;
        }
        event_ptr->next_event = create_event_node(song_file_ptr);
        if (event_type(event_ptr->next_event->event) == META_EVENT_T) {
          const char *event_name = NULL;
          event_name = event_ptr->next_event->event->meta_event.name;
          if (!strcmp(event_name, META_TABLE[0x2f].name)) {
            read_events = false;
          }
        }
      }
    }
    track_ptr->track->event_list = event_ptr_head;

    if (i != song->num_tracks - 1) {
      track_ptr->next_track = malloc(sizeof(track_node_t));
      track_ptr = track_ptr->next_track;
    }

  }
} /* parse_track() */

/*
 * Parses an event
 */

event_t *parse_event(FILE *song_file_ptr) {
  event_t *event = malloc(sizeof(event_t));
  assert(event);

  uint32_t delta_time = parse_var_len(song_file_ptr);
  event->delta_time = delta_time;

  uint8_t type = 0;
  read_data(&type, sizeof(type), 1, song_file_ptr);
  if (type == SYS_EVENT_1) {
    g_last_event_midi = false;
    event->type = SYS_EVENT_1;
    event->sys_event = parse_sys_event(song_file_ptr, type);
  }
  else if (type == SYS_EVENT_2) {
    g_last_event_midi = false;
    event->type = SYS_EVENT_2;
    event->sys_event = parse_sys_event(song_file_ptr, type);
  }
  else if (type == META_EVENT) {
    g_last_event_midi = false;
    event->type = META_EVENT;
    event->meta_event = parse_meta_event(song_file_ptr);
  }
  else {
    event->type = MIDI_EVENT_T;
    event->midi_event = parse_midi_event(song_file_ptr, type);
    g_last_midi = event->midi_event;
    g_last_event_midi = true;
  }

  return event;
} /* parse_event() */

/*
 * Parses a sys_event
 */

sys_event_t parse_sys_event(FILE *song_file_ptr, uint8_t type) {
  sys_event_t sys_event = {0};
  uint32_t length = parse_var_len(song_file_ptr);

  sys_event.data_len = length;
  if (length > 0) {
    sys_event.data = malloc(length);
    for (int i = 0; i < length; i++) {
      read_data((sys_event.data) + i, sizeof(char), 1, song_file_ptr);
    }
  }
  return sys_event;
} /* parse_sys_event() */

/*
 * parses a meta event
 */

meta_event_t parse_meta_event(FILE *song_file_ptr) {
  meta_event_t meta_event = {0};
  uint8_t type = 0;
  read_data(&type, sizeof(type), 1, song_file_ptr);
  meta_event.name = META_TABLE[type].name;
  assert(meta_event.name);
  uint32_t length = parse_var_len(song_file_ptr);
  meta_event.data_len = length;
  if (!(((type >= 0x1) && (type <= 0x07)) || (type == 0x7f))) {
    assert(meta_event.data_len == META_TABLE[type].data_len);
  }
  if (length > 0) {
    meta_event.data = malloc((length));
    for (int i = 0; i < length; i++) {
      read_data((meta_event.data) + i, sizeof(char), 1, song_file_ptr);
    }
  }
  return meta_event;
} /* parse_meta_event() */

/*
 * parses a midi event
 */

midi_event_t parse_midi_event(FILE *song_file_ptr, uint8_t status) {
  midi_event_t midi_event = {0};

  if (g_last_event_midi) {
    if (!(status & MASK_BIT_7)) {
      midi_event.status = g_last_midi.status;
      midi_event.name = MIDI_TABLE[midi_event.status].name;
      assert(midi_event.name);
      midi_event.data_len = MIDI_TABLE[midi_event.status].data_len;
      if (midi_event.data_len > 0) {
        midi_event.data = malloc(midi_event.data_len);
        *midi_event.data = status;
        for (int i = 1; i < midi_event.data_len; i++) {
          read_data(midi_event.data + i, sizeof(char), 1, song_file_ptr);
        }
      }
      return midi_event;
    }
  }
  midi_event.status = status;
  midi_event.name = MIDI_TABLE[midi_event.status].name;
  assert(midi_event.name);
  midi_event.data_len = MIDI_TABLE[midi_event.status].data_len;
  if (midi_event.data_len > 0) {
    midi_event.data = malloc(midi_event.data_len);
    for (int i = 0; i < midi_event.data_len; i++) {
      read_data(midi_event.data + i, sizeof(char), 1, song_file_ptr);
    }
  }
  return midi_event;
} /* parse_midi_event() */

/*
 * Reads in a variable length quantity
 */

uint32_t parse_var_len(FILE *midi_file_ptr) {
  uint32_t result = 0;
  uint8_t in = 0;
  do {
    in = 0;
    read_data(&in, sizeof(char), 1, midi_file_ptr);
    uint8_t to_add = (in & BITS_7);
    result = result << SHIFT_7;
    result |= to_add;
  } while (MASK_BIT_7 & in);
  return result;
} /* parse_var_len() */

/*
 * Swaps the endianness of a 16-bit number
 */

uint16_t end_swap_16(uint8_t number[2]) {
  uint16_t result = 0;
  result |= number[1];
  result |= (number[0] << SHIFT_8);
  return result;
} /* end_swap_16() */

/*
 * Frees a song
 */

void free_song(song_data_t *song) {
  free(song->path);
  song->path = NULL;

  track_node_t *track_ptr = song->track_list;
  for (int i = 0; i < song->num_tracks; i++) {
    track_node_t *next = track_ptr->next_track;
    free_track_node(track_ptr);
    track_ptr = next;
  }
  song->track_list = NULL;
  free(song);
  song = NULL;
} /* free_song() */

/*
 * Frees a track node
 */

void free_track_node(track_node_t *track_node) {
  track_node->next_track = NULL;
  assert(track_node->track);
  event_node_t *event_ptr = track_node->track->event_list;

  int count = 0;
  while (event_ptr) {
    event_node_t *next = event_ptr->next_event;
    free_event_node(event_ptr);
    count++;
    event_ptr = next;
  }

  free(track_node->track);
  track_node->track = NULL;
  free(track_node);
  track_node = NULL;
} /* free_track_node() */

/*
 * Frees an event node
 */

void free_event_node(event_node_t *event_node) {
  event_node->next_event = NULL;
  if (event_type(event_node->event) == 1) {
    if (event_node->event->sys_event.data_len > 0) {
      free(event_node->event->sys_event.data);
      event_node->event->sys_event.data = NULL;
    }
  }
  else if (event_type(event_node->event) == 2) {
    if (event_node->event->meta_event.data_len > 0) {
      free(event_node->event->meta_event.data);
      event_node->event->meta_event.data = NULL;
    }
  }
  else {
    if (event_node->event->midi_event.data_len > 0) {
      free(event_node->event->midi_event.data);
      event_node->event->midi_event.data = NULL;
    }
  }
  free(event_node->event);
  event_node->event = NULL;
  free(event_node);
  event_node = NULL;
} /* free_event_node() */

/*
 * Returns what type of event it is
 */

uint8_t event_type(event_t *event) {
  if ((event->type == SYS_EVENT_1) || (event->type == SYS_EVENT_2)) {
    return SYS_EVENT_T;
  }
  if (event->type == META_EVENT) {
    return META_EVENT_T;
  }
  return MIDI_EVENT_T;
} /* event_type() */

/*
 * Swaps the endianness of a 32-bit number
 */

uint32_t end_swap_32(uint8_t number[4]) {
  uint32_t result = 0;
  int shift = SHIFT_8;
  result |= number[3];
  result |= (number[2] << shift);
  shift += SHIFT_8;
  result |= (number[1] << shift);
  shift += SHIFT_8;
  result |= (number[0] << shift);
  return result;
} /* end_swap_32() */
