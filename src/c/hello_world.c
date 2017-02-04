#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_timesec_layer;
static TextLayer *s_data_layer;

static char WEEKDAYS[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
#define LESSON_COUNT 6
#define H 3600
#define M 60
#define BEGINN 0
#define ENDE 1
#define PAUSE -1
#define VORSTART -2
#define NACHENDE -3
#define WOCHENENDE -4 

static int LESSONS[LESSON_COUNT][2] = {{ 7*H+50*M, 8*H+35*M},
                                       { 8*H+40*M, 9*H+25*M}, 
                                       { 9*H+45*M,10*H+30*M},
                                       {10*H+35*M,11*H+20*M},
                                       {11*H+35*M,12*H+20*M},
                                       {12*H+25*M,13*H+10*M}};

                          
bool show_lesson_len = false;


static int remaining_sec(time_t now)
{
  if (localtime(&now)->tm_wday == 0 || localtime(&now)->tm_wday == 6) {return WOCHENENDE;}
  int t = localtime(&now)->tm_sec + localtime(&now)->tm_min*60 + localtime(&now)->tm_hour*3600;

  
  if (t < LESSONS[0][BEGINN]) {
    return VORSTART;
  } else if (t > LESSONS[LESSON_COUNT-1][ENDE]) {
    return NACHENDE;
  } else {
    for(int i=0;i<LESSON_COUNT-1;i++) {
      if (t >= LESSONS[i][BEGINN] && t < LESSONS[i][ENDE]) {return LESSONS[i][ENDE]-t; }
    }
  }
  return PAUSE;
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[10];
  strftime(s_buffer, sizeof(s_buffer), "%H:%M" , tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);

  static char s_buffer2[10];
  strftime(s_buffer2, sizeof(s_buffer2), "%S" , tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_timesec_layer, s_buffer2);

  static char s_bufferdate[12];
  strftime(s_bufferdate, sizeof(s_bufferdate), "%d.%m.%Y" , tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_bufferdate);

  static char s_bufferday[2];
  strftime(s_bufferday, sizeof(s_bufferday), "%w" , tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_text_layer, WEEKDAYS[atoi(s_bufferday)]);
  
  if(show_lesson_len)
  {
    static char s[30]; 
    int sec = remaining_sec(temp);
    if (sec == WOCHENENDE) {
      snprintf(s,sizeof(s),"Jule hat Wochenende!");
    } else if (sec == VORSTART) {
      snprintf(s,sizeof(s),"Zu früh für Jule!");
    } else if (sec == PAUSE) {
      snprintf(s,sizeof(s),"Jule macht Pause!");
    } else if (sec == NACHENDE) {
      snprintf(s,sizeof(s),"Schluss für heute!");
    } else {
      snprintf(s,sizeof(s),"Noch %d:%02d Sek...",sec / 60, sec % 60);
    }
    text_layer_set_text(s_data_layer, s);
  } else {
      text_layer_set_text(s_data_layer, "");
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  show_lesson_len= !show_lesson_len;
  update_time();
}

static void click_config_provider(void *context) {
  ButtonId id = BUTTON_ID_UP ; 
  window_single_click_subscribe(id, select_click_handler);
}

static void init(void) {
	// Create a window and get information about the window
	s_window = window_create();
  
  // Use this provider to add button click subscriptions
  window_set_click_config_provider(s_window, click_config_provider);
  
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
	
  // Create a text layer and set the text
	s_text_layer = text_layer_create(bounds);
	text_layer_set_text(s_text_layer, "Mittwoch");
  
  // Set the font and text alignment
	text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  
    s_date_layer = text_layer_create(
      GRect(0, 32, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "01.01.2017");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  
  
  
    // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w-20, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);

  s_timesec_layer = text_layer_create(
      GRect(126, 76, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_timesec_layer, GColorClear);
  text_layer_set_text_color(s_timesec_layer, GColorBlack);
  text_layer_set_text(s_timesec_layer, "00");
  text_layer_set_font(s_timesec_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_timesec_layer, GTextAlignmentLeft);

  s_data_layer = text_layer_create(
      GRect(0, 90, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_data_layer, GColorClear);
  text_layer_set_text_color(s_data_layer, GColorBlack);
  text_layer_set_text(s_data_layer, "");
  text_layer_set_font(s_data_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_data_layer, GTextAlignmentCenter);

  
  
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_text_layer));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_date_layer));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_time_layer));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_timesec_layer));
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_data_layer));

  // Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
  text_layer_enable_screen_text_flow_and_paging(s_text_layer, 10);

	// Push the window, setting the window animation to 'true'
	window_stack_push(s_window, true);

    // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

static void deinit(void) {
	// Destroy the text layer
	text_layer_destroy(s_text_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_timesec_layer);
	text_layer_destroy(s_data_layer);
	
	// Destroy the window
	window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
