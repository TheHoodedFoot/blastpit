#ifndef LVGL_DEMO_H
#define LVGL_DEMO_H

#ifdef __cplusplus
extern "C"
{
#endif

	/*********************
	 *      INCLUDES
	 *********************/
#include "lvgl.h"

	/*********************
	 *      DEFINES
	 *********************/
#define DEFAULT_NUM_PASSES 10

	/**********************
	 *      TYPEDEFS
	 **********************/
	typedef struct
	{
		lv_group_t* lvgl_input_group;
		lv_obj_t*   tv;
		lv_obj_t*   tb_input;
		lv_obj_t*   tb_generate;
		lv_obj_t*   tb_execute;
		lv_obj_t*   spinbox;
	} t_lv_multipass;

	/**********************
	 * GLOBAL PROTOTYPES
	 **********************/

	// void ta_event_cb( lv_event_t* e );

	/**********************
	 *      MACROS
	 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_DEMO_H*/
