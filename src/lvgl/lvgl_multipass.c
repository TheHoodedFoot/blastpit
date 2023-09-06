/**
 * @file lvgl_multipass.c
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
#include "lvgl_hal.h"
#include "lvgl_multipass.h"
// #include "widgets/spinbox/lv_spinbox.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void MpLvglTabs( t_lv_multipass* this );
static void MpLvglInputTab( t_lv_multipass* this, lv_obj_t* parent );
static void MpLvglGenerateTab( t_lv_multipass* this, lv_obj_t* parent );
static void MpLvglExecuteTab( t_lv_multipass* this, lv_obj_t* parent );

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

// static lv_obj_t* spinbox;

static void
lv_spinbox_increment_event_cb( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	if ( code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT ) {
		// lv_spinbox_increment( spinbox );
		// TODO: How do we pass this our context?
	}
}

static void
lv_spinbox_decrement_event_cb( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	if ( code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT ) {
		// lv_spinbox_decrement( spinbox );
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

// static void
// ta_event_cb( lv_event_t* e )
// {
// 	lv_indev_t* indev = lv_indev_get_act();
// 	if ( indev == NULL ) {
// 		return;
// 	}
// 	lv_indev_type_t indev_type = lv_indev_get_type( indev );

// 	lv_event_code_t code = lv_event_get_code( e );
// 	lv_obj_t*	ta   = lv_event_get_target( e );
// 	lv_obj_t*	kb   = lv_event_get_user_data( e );

// 	if ( code == LV_EVENT_CLICKED && indev_type == LV_INDEV_TYPE_ENCODER ) {
// 		// Middle mouse button clicked - show keyboard
// 		lv_keyboard_set_textarea( kb, ta );
// 		lv_obj_clear_flag( kb, LV_OBJ_FLAG_HIDDEN );
// 		lv_group_focus_obj( kb );
// 		lv_group_set_editing( lv_obj_get_group( kb ), kb );
// 		lv_obj_set_height( tv, LV_VER_RES / 2 );
// 		lv_obj_align( kb, LV_ALIGN_BOTTOM_MID, 0, 0 );
// 	}

// 	if ( code == LV_EVENT_READY || code == LV_EVENT_CANCEL ) {
// 		lv_obj_add_flag( kb, LV_OBJ_FLAG_HIDDEN );
// 		lv_obj_set_height( tv, LV_VER_RES );
// 		printf( "Text: %s\n", lv_textarea_get_text( ta ) );
// 	}
// }

static void
my_bar_set_value( void* bar, int32_t v )
{
	lv_bar_set_value( bar, v, LV_ANIM_OFF );
}

static void
my_bar_event_cb( lv_event_t* e )
{
	lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc( e );
	if ( dsc->part != LV_PART_INDICATOR ) {
		return;
	}

	lv_obj_t* obj = lv_event_get_target( e );

	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init( &label_dsc );
	label_dsc.font = LV_FONT_DEFAULT;

	char buf[20];
	lv_snprintf( buf, sizeof( buf ), "Pass %d of %d", (int)lv_bar_get_value( obj ), (int)10 );

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
static void
event_handler( lv_event_t* e )
{
	lv_event_code_t code = lv_event_get_code( e );
	lv_obj_t*	obj  = lv_event_get_target( e );
	if ( code == LV_EVENT_VALUE_CHANGED ) {
		char buf[32];
		lv_dropdown_get_selected_str( obj, buf, sizeof( buf ) );
		LV_LOG_USER( "Option: %s", buf );
	}
}

int
main( void )
{
	t_lv_multipass this;

	// Setup LVGL
	lv_init();
	hal_setup();

	// Setup initial GUI state
	MpLvglTabs( &this );

	// Put this in its own thread
	while ( 1 ) {
		hal_loop();
	}
}

void
MpLvglTabs( t_lv_multipass* this )
{
	this->lvgl_input_group = lv_group_create();
	lv_group_set_default( this->lvgl_input_group );

	lv_indev_t* cur_drv = NULL;
	for ( ;; ) {
		cur_drv = lv_indev_get_next( cur_drv );
		if ( !cur_drv ) {
			break;
		}

		if ( cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD ) {
			lv_indev_set_group( cur_drv, this->lvgl_input_group );
		}

		if ( cur_drv->driver->type == LV_INDEV_TYPE_ENCODER ) {
			lv_indev_set_group( cur_drv, this->lvgl_input_group );
		}
	}

	this->tv = lv_tabview_create( lv_scr_act(), LV_DIR_TOP, LV_DPI_DEF / 3 );

	this->tb_input = lv_tabview_add_tab( this->tv, "Multipass" );
	MpLvglInputTab( this, this->tb_input );

	this->tb_generate = lv_tabview_add_tab( this->tv, "Generate" );
	MpLvglGenerateTab( this, this->tb_generate );

	this->tb_execute = lv_tabview_add_tab( this->tv, "Execute" );
	MpLvglExecuteTab( this, this->tb_execute );
}

static void
MpLvglInputTab( t_lv_multipass* this, lv_obj_t* parent )
{
	(void)this;

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

	this->spinbox = lv_spinbox_create( spb_cont );
	lv_spinbox_set_range( this->spinbox, 1, 20 );
	lv_spinbox_set_digit_format( this->spinbox, 2, 0 );
	lv_spinbox_set_value( this->spinbox, DEFAULT_NUM_PASSES );
	lv_obj_set_width( this->spinbox, 100 );

	lv_coord_t h = lv_obj_get_height( this->spinbox );
	lv_obj_set_size( btn, h, h );
	lv_obj_align_to( this->spinbox, spb_cont, LV_ALIGN_TOP_MID, h + btnpad, -12 );
	lv_obj_align_to( btn, this->spinbox, LV_ALIGN_OUT_LEFT_MID, -btnpad, 0 );
	// lv_obj_set_pos( spinbox, 48, 150 );
	// lv_obj_center( spinbox );

	// return;
	btn = lv_btn_create( spb_cont );
	lv_obj_set_size( btn, h, h );
	lv_obj_align_to( btn, this->spinbox, LV_ALIGN_OUT_RIGHT_MID, btnpad, 0 );
	lv_obj_set_style_bg_img_src( btn, LV_SYMBOL_PLUS, 0 );
	lv_obj_add_event_cb( btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, NULL );

	// lv_example_spinbox_1( parent );
}

static void
MpLvglGenerateTab( t_lv_multipass* this, lv_obj_t* parent )
{
	(void)this;

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

	// lv_obj_add_event_cb( ta1, ta_event_cb, LV_EVENT_ALL, kb );
	// lv_obj_add_event_cb( ta2, ta_event_cb, LV_EVENT_ALL, kb );
}

void
MpLvglExecuteTab( t_lv_multipass* this, lv_obj_t* parent )
{
	(void)this;

	lv_obj_set_flex_flow( parent, LV_FLEX_FLOW_COLUMN );
	lv_obj_set_flex_align( parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER );

	// Windows filepath display
	lv_obj_t* ta1 = lv_textarea_create( parent );
	lv_obj_set_width( ta1, LV_PCT( 100 ) );
	lv_textarea_set_one_line( ta1, true );
	lv_textarea_set_placeholder_text( ta1, "Windows filepath" );

	// Button
	static lv_style_t style_btn_red;
	lv_style_init( &style_btn_red );
	lv_style_set_bg_color( &style_btn_red, lv_color_hex( 0xff0000 ) );
	lv_style_set_opa( &style_btn_red, 50 );
	lv_obj_t* btn = lv_btn_create( parent );
	lv_obj_add_style( btn, &style_btn_red, 0 );
	lv_obj_t* label = lv_label_create( btn );
	lv_label_set_text( label, "Execute" );

	// lv_obj_set_state( btn, LV_IMGBTN_STATE_DISABLED);
	lv_obj_clear_flag( btn, LV_OBJ_FLAG_CLICKABLE );
	// lv_obj_add_flag( btn, LV_OBJ_FLAG_CLICKABLE);

	// Bar
	lv_obj_t* bar = lv_bar_create( parent );
	lv_obj_add_event_cb( bar, my_bar_event_cb, LV_EVENT_DRAW_PART_END, NULL );
	lv_obj_set_size( bar, LV_PCT( 100 ), 20 );
	// lv_obj_center( bar );
	my_bar_set_value( bar, 0 );
}

// Dropdown callback (postgres)

// Generate routine

// Execute routine

// Add 'does VLM file exist' to blastpit
