
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#define LGFX_WT32_SC01  
#define LGFX_USE_V1         // set to use new version of library
//#define LGFX_AUTODETECT

#include <WiFi.h>
#include <WebServer.h>

#include <LovyanGFX.hpp>    // main library
#include <LGFX_AUTODETECT.hpp>

#include <lvgl.h>
#include "lv_conf.h"


// Constantes
#define PASSWORD_LENGTH 6
static char password[PASSWORD_LENGTH + 1];  // +1 para el carácter nulo

/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

// Funciones

//void password_screen_event_cb(lv_obj_t * obj, lv_event_t event);
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void create_password_screen(void);
static LGFX lcd;



void setup() {
  // Inicialización de LVGL
  lcd.init();
  lv_init();
   // Initialize LovyanGFX
 /* LVGL : Setting up buffer to use for display */

   // Setting display to landscape
  if (lcd.width() < lcd.height())
    lcd.setRotation(lcd.getRotation() ^ 1);

  /* LVGL : Setting up buffer to use for display */
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*** LVGL : Setup & Initialize the display device driver ***/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*** LVGL : Setup & Initialize the input device driver ***/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Crear la pantalla de introducción de contraseña
  create_password_screen();
}

void loop() {
  lv_timer_handler();
  delay(5);  // Pequeño retraso para evitar el uso intensivo de la CPU
}

/*** Display callback to flush the buffer to screen ***/
  void display_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p)
  {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
    lcd.endWrite();

    lv_disp_flush_ready(disp);
  }

  /*** Touchpad callback to read the touchpad ***/
  void touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
  {
    uint16_t touchX, touchY;
    bool touched = lcd.getTouch(&touchX, &touchY);

    if (!touched)
    {
      data->state = LV_INDEV_STATE_REL;
    }
    else
    {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;

      // Serial.printf("Touch (x,y): (%03d,%03d)\n",touchX,touchY );
    }
  }  



  static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t*) lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}


void create_password_screen(void) {
  // Crear una nueva pantalla
  lv_obj_t * pwd_ta = lv_textarea_create(lv_scr_act());
  lv_textarea_set_text(pwd_ta, "");
  lv_textarea_set_password_mode(pwd_ta, true);
  lv_textarea_set_one_line(pwd_ta, true);
  lv_obj_set_width(pwd_ta, screenWidth-100);
  lv_obj_set_height(pwd_ta, 40);
  lv_obj_set_pos(pwd_ta, 70, 50);
  lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, NULL);

 /*Create the password label*/
  lv_obj_t * pwd_label = lv_label_create(lv_scr_act());
  lv_label_set_text(pwd_label, "Clave:");
  lv_obj_align_to(pwd_label, pwd_ta, LV_ALIGN_OUT_LEFT_TOP, -10, 10);

 /*Create a keyboard*/
  lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
  lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);

  lv_keyboard_set_textarea(kb, pwd_ta); /*Focus it on one of the text areas to start*/
}


