/**
 * @file lv_demo_keypad_encoder.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "display/fbdev.h"
#include "display/monitor.h"
#include "indev/keyboard.h"
#include "indev/mouse.h"
#include "indev/mousewheel.h"

#include "lvgl.h"
#include "lvgl_demo.h"
#include "lvgl_hal.h"
#include "src/widgets/spinbox/lv_spinbox.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
// static void selectors_create( lv_obj_t* parent );
static void text_input_create( lv_obj_t* parent );
static void mpCreateWidgets( lv_obj_t* parent );
void	    lv_example_bar_6( void );

static void ta_event_cb( lv_event_t* e );

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t* g;
static lv_obj_t*   tv;
static lv_obj_t*   t1;
static lv_obj_t*   t2;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_keypad_encoder( void )
{
	g = lv_group_create();
	lv_group_set_default( g );

	lv_indev_t* cur_drv = NULL;
	for ( ;; ) {
		cur_drv = lv_indev_get_next( cur_drv );
		if ( !cur_drv ) {
			break;
		}

		if ( cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD ) {
			lv_indev_set_group( cur_drv, g );
		}

		if ( cur_drv->driver->type == LV_INDEV_TYPE_ENCODER ) {
			lv_indev_set_group( cur_drv, g );
		}
	}

	tv = lv_tabview_create( lv_scr_act(), LV_DIR_TOP, LV_DPI_DEF / 3 );

	t1 = lv_tabview_add_tab( tv, "Multipass" );
	t2 = lv_tabview_add_tab( tv, "Generate" );
	lv_tabview_add_tab( tv, "Execute" );

	mpCreateWidgets( t1 );

	// selectors_create( t1 );
	text_input_create( t2 );
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t* spinbox;

static void lv_spinbox_increment_event_cb( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	if ( code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT ) {
		lv_spinbox_increment( spinbox );
	}
}

static void lv_spinbox_decrement_event_cb( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	if ( code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT ) {
		lv_spinbox_decrement( spinbox );
	}
}


// static void
// selectors_create( lv_obj_t* parent )
// {
// 	lv_obj_set_flex_flow( parent, LV_FLEX_FLOW_COLUMN );
// 	lv_obj_set_flex_align( parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER );

// 	lv_obj_t* obj;

// 	// Instead of parent, we can use pv_scr_act(), which gives us
// 	// the current screen
// 	obj = lv_spinner_create( parent, 1000, 60 );
// 	lv_obj_set_size( obj, 100, 100 );
// 	lv_obj_center( obj );

// 	obj = lv_checkbox_create( parent );
// 	lv_obj_add_flag( obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS );

// 	obj = lv_slider_create( parent );
// 	lv_slider_set_range( obj, 0, 10 );
// 	lv_obj_add_flag( obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS );

// 	obj = lv_switch_create( parent );
// 	lv_obj_add_flag( obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS );

// 	obj = lv_spinbox_create( parent );
// 	lv_obj_add_flag( obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS );

// 	obj = lv_dropdown_create( parent );
// 	lv_obj_add_flag( obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS );
// }

static void ta_event_cb( lv_event_t* e )
{
	lv_indev_t* indev = lv_indev_get_act();
	if ( indev == NULL ) {
		return;
	}
	lv_indev_type_t indev_type = lv_indev_get_type( indev );

	lv_event_code_t code = lv_event_get_code( e );
	lv_obj_t*	ta   = lv_event_get_target( e );
	lv_obj_t*	kb   = lv_event_get_user_data( e );

	if ( code == LV_EVENT_CLICKED && indev_type == LV_INDEV_TYPE_ENCODER ) {
		// Middle mouse button clicked - show keyboard
		lv_keyboard_set_textarea( kb, ta );
		lv_obj_clear_flag( kb, LV_OBJ_FLAG_HIDDEN );
		lv_group_focus_obj( kb );
		lv_group_set_editing( lv_obj_get_group( kb ), kb );
		lv_obj_set_height( tv, LV_VER_RES / 2 );
		lv_obj_align( kb, LV_ALIGN_BOTTOM_MID, 0, 0 );
	}

	if ( code == LV_EVENT_READY || code == LV_EVENT_CANCEL ) {
		lv_obj_add_flag( kb, LV_OBJ_FLAG_HIDDEN );
		lv_obj_set_height( tv, LV_VER_RES );
		printf( "Text: %s\n", lv_textarea_get_text( ta ) );
	}
}

static void text_input_create( lv_obj_t* parent )
{
	lv_obj_set_flex_flow( parent, LV_FLEX_FLOW_COLUMN );

	lv_obj_t* ta1 = lv_textarea_create( parent );
	lv_obj_set_width( ta1, LV_PCT( 100 ) );
	lv_textarea_set_one_line( ta1, true );
	lv_textarea_set_placeholder_text( ta1, "Click with an encoder to show a keyboard" );

	lv_obj_t* ta2 = lv_textarea_create( parent );
	lv_obj_set_width( ta2, LV_PCT( 100 ) );
	lv_textarea_set_one_line( ta2, true );
	lv_textarea_set_placeholder_text( ta2, "Type something" );

	lv_obj_t* kb = lv_keyboard_create( lv_scr_act() );
	lv_obj_add_flag( kb, LV_OBJ_FLAG_HIDDEN );

	lv_obj_add_event_cb( ta1, ta_event_cb, LV_EVENT_ALL, kb );
	lv_obj_add_event_cb( ta2, ta_event_cb, LV_EVENT_ALL, kb );
}

static void set_value( void* bar, int32_t v )
{
	lv_bar_set_value( bar, v, LV_ANIM_OFF );
}

static void event_cb( lv_event_t* e )
{
	lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc( e );
	if ( dsc->part != LV_PART_INDICATOR ) {
		return;
	}

	lv_obj_t* obj = lv_event_get_target( e );

	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init( &label_dsc );
	label_dsc.font = LV_FONT_DEFAULT;

	char buf[8];
	lv_snprintf( buf, sizeof( buf ), "%d", (int)lv_bar_get_value( obj ) );

	lv_point_t txt_size;
	lv_txt_get_size( &txt_size,
			 buf,
			 label_dsc.font,
			 label_dsc.letter_space,
			 label_dsc.line_space,
			 LV_COORD_MAX,
			 label_dsc.flag );

	lv_area_t txt_area;
	/*If the indicator is long enough put the text inside on the right*/
	if ( lv_area_get_width( dsc->draw_area ) > txt_size.x + 20 ) {
		txt_area.x2	= dsc->draw_area->x2 - 5;
		txt_area.x1	= txt_area.x2 - txt_size.x + 1;
		label_dsc.color = lv_color_white();
	}
	/*If the indicator is still short put the text out of it on the right*/
	else {
		txt_area.x1	= dsc->draw_area->x2 + 5;
		txt_area.x2	= txt_area.x1 + txt_size.x - 1;
		label_dsc.color = lv_color_black();
	}

	txt_area.y1 = dsc->draw_area->y1 + ( lv_area_get_height( dsc->draw_area ) - txt_size.y ) / 2;
	txt_area.y2 = txt_area.y1 + txt_size.y - 1;

	lv_draw_label( dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL );
}

/**
 * Custom drawer on the bar to display the current value
 */
void lv_example_bar_6( void )
{
	lv_obj_t* bar = lv_bar_create( lv_scr_act() );
	lv_obj_add_event_cb( bar, event_cb, LV_EVENT_DRAW_PART_END, NULL );
	lv_obj_set_size( bar, 200, 20 );
	lv_obj_center( bar );
	set_value( bar, 75 );
}

int main( void )
{
	lv_init();
	hal_setup();

	lv_demo_keypad_encoder();

	// Put this in its own thread
	while ( 1 ) {
		hal_loop();
	}
}

static void event_handler( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	lv_obj_t*	obj  = lv_event_get_target( e );
	if ( code == LV_EVENT_VALUE_CHANGED ) {
		char buf[32];
		lv_dropdown_get_selected_str( obj, buf, sizeof( buf ) );
		LV_LOG_USER( "Option: %s", buf );
	}
}

static void mpCreateWidgets( lv_obj_t* parent )
{
	lv_obj_t *obj, *label;

	// Put all objects into a single column
	lv_obj_set_flex_flow( parent, LV_FLEX_FLOW_COLUMN );

	// Set the placement of the child objects
	lv_obj_set_flex_align( parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START );

	// Add padding to the page to prevent widgets touching the sides
	lv_obj_set_style_pad_top( parent, 10, LV_PART_MAIN );
	lv_obj_set_style_pad_bottom( parent, 10, LV_PART_MAIN );
	lv_obj_set_style_pad_left( parent, 10, LV_PART_MAIN );
	lv_obj_set_style_pad_right( parent, 10, LV_PART_MAIN );

	lv_obj_set_scrollbar_mode( parent, LV_SCROLLBAR_MODE_OFF );

	// Set the font
	// static lv_style_t style;
	// lv_style_init(&style);
	// lv_style_set_text_font(&style, &lv_font_montserrat_10);
	// lv_obj_add_style(parent, &style, 0);

	// ░░░█░█░▀█▀░█▀▄░█▀▀░█▀▀░▀█▀░█▀▀
	// ░░░█▄█░░█░░█░█░█░█░█▀▀░░█░░▀▀█
	// ░░░▀░▀░▀▀▀░▀▀░░▀▀▀░▀▀▀░░▀░░▀▀▀

	// Customer Drop-down List (lv_dropdown)


	/*Create a normal drop down list*/
	lv_obj_t* dd = lv_dropdown_create( parent );
	lv_dropdown_set_text( dd, "Select Customer" );
	lv_dropdown_set_options( dd,
				 "Mitchel & Co.\n"
				 "Rivermounts\n"
				 "Touchstone\n"
				 "Ashes Memorial Jewellery" );
	lv_obj_set_width( dd, 170 );

	lv_obj_add_event_cb( dd, event_handler, LV_EVENT_ALL, NULL );

	obj = lv_textarea_create( parent );
	lv_obj_set_width( obj, LV_PCT( 100 ) );
	lv_textarea_set_one_line( obj, true );
	lv_textarea_set_placeholder_text( obj, "Customer" );

	// Filename
	obj = lv_textarea_create( parent );
	lv_obj_set_width( obj, LV_PCT( 100 ) );
	lv_textarea_set_one_line( obj, true );
	lv_textarea_set_placeholder_text( obj, "Filename" );

	// ░░░█▀▀░█▀█░▀█▀░█▀█░█▀▄░█▀█░█░█
	// ░░░▀▀█░█▀▀░░█░░█░█░█▀▄░█░█░▄▀▄
	// ░░░▀▀▀░▀░░░▀▀▀░▀░▀░▀▀░░▀▀▀░▀░▀


	// lv_obj_align_to( label, spb_cont, LV_ALIGN_TOP_LEFT, 0, -10 );

	// lv_obj_align_to( label, spinbox, LV_ALIGN_CENTER, 150, 0 );

	lv_obj_t* spb_cont = lv_obj_create( parent );
	lv_obj_set_width( spb_cont, LV_PCT( 100 ) );
	lv_obj_set_height( spb_cont, LV_SIZE_CONTENT );
	lv_obj_set_style_border_opa( spb_cont, 0, LV_PART_MAIN );
	lv_obj_set_style_bg_opa( spb_cont, 0, LV_PART_MAIN );
	lv_obj_set_style_pad_left( spb_cont, -2, 0 );
	// lv_obj_set_style_pad_top(spb_cont, -10, 0);

	const int btnpad = 10;

	label = lv_label_create( spb_cont );
	lv_label_set_text( label, "Number of passes" );

	lv_obj_t* btn = lv_btn_create( spb_cont );
	lv_obj_set_style_bg_img_src( btn, LV_SYMBOL_MINUS, 0 );
	lv_obj_add_event_cb( btn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, NULL );

	spinbox = lv_spinbox_create( spb_cont );
	lv_spinbox_set_range( spinbox, 1, 20 );
	lv_spinbox_set_digit_format( spinbox, 2, 0 );
	lv_spinbox_set_value( spinbox, 10 );
	lv_obj_set_width( spinbox, 100 );

	lv_coord_t h = lv_obj_get_height( spinbox );
	lv_obj_set_size( btn, h, h );
	lv_obj_align_to( spinbox, spb_cont, LV_ALIGN_TOP_MID, h + btnpad, -12 );
	lv_obj_align_to( btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -btnpad, 0 );
	// lv_obj_set_pos( spinbox, 48, 150 );
	// lv_obj_center( spinbox );

	// return;
	btn = lv_btn_create( spb_cont );
	lv_obj_set_size( btn, h, h );
	lv_obj_align_to( btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, btnpad, 0 );
	lv_obj_set_style_bg_img_src( btn, LV_SYMBOL_PLUS, 0 );
	lv_obj_add_event_cb( btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, NULL );

	// lv_example_spinbox_1( parent );

	// lv_example_bar_6();
}
