/*
 * test_Driver_PushButton.cpp
 *
 *	Test unitario para el módulo Driver_PushButton
 */



//------------------------------------------------------------------------------------
//-- TEST HEADERS --------------------------------------------------------------------
//------------------------------------------------------------------------------------

#include "mbed.h"
#include "AppConfig.h"

#if ESP_PLATFORM == 1 || (__MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_PushButton))

/** Requerido para test unitarios ESP-MDF */
#if ESP_PLATFORM == 1
#include "unity.h"
#include "Heap.h"
void (*syslog_print)(const char*level, const char* tag, const char* format, ...) = NULL;
#define PinName32_LOCAL			GPIO_NUM_21

/** Requerido para test unitarios STM32 */
#elif __MBED__ == 1 && defined(ENABLE_TEST_DEBUGGING) && defined(ENABLE_TEST_PushButton)
#include "unity.h"
#include "Heap.h"
#include "unity_test_runner.h"
/// Configuración MBED_API_uSerial

// Configuración btnzer
#define PinName32_LOCAL			PC_1//PD_8
#endif

#include "PushButton.h"


//------------------------------------------------------------------------------------
//-- SPECIFIC COMPONENTS FOR TESTING -------------------------------------------------
//------------------------------------------------------------------------------------

static const char* _MODULE_ = "[TEST_PushB].....";
#define _EXPR_	(true)


//------------------------------------------------------------------------------------
//-- REQUIRED HEADERS & COMPONENTS FOR TESTING ---------------------------------------
//------------------------------------------------------------------------------------

/** PushButton local  a verificar */
static PushButton* btn = NULL;

/** Control de eventos */
static uint8_t press_count = 0, hold_count = 0, release_count = 0;

/** callbacks */
static void onPressed(uint32_t uid){
	press_count++;
}
static void onHold(uint32_t uid){
	hold_count++;

}
static void onReleased(uint32_t uid){
	release_count++;
}
static void onPressed2(){
	press_count++;
}
static void onHold2(){
	hold_count++;

}
static void onReleased2(){
	release_count++;
}

//------------------------------------------------------------------------------------
//-- TEST FUNCTIONS ------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
static void test_btn_new_local(){
	TEST_ASSERT_NULL(btn);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando btn en cpu local");
	btn = new PushButton(PinName32_LOCAL, 0, PushButton::PressIsLowLevel, PullNone, true);
	TEST_ASSERT_NOT_NULL(btn);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "PushButton OK!");
}

//------------------------------------------------------------------------------------
static void test_btn_enable_callbacks_uid(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Instala button callbacks UID");

	btn->enablePressEvents(callback(&onPressed));
	btn->enableHoldEvents(callback(&onHold), 1000);
	btn->enableReleaseEvents(callback(&onReleased));
}

//------------------------------------------------------------------------------------
static void test_btn_enable_callbacks_void(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Instala button callbacks VOID");

	btn->enablePressEvents(callback(&onPressed2));
	btn->enableHoldEvents(callback(&onHold2), 1000);
	btn->enableReleaseEvents(callback(&onReleased2));
}

//------------------------------------------------------------------------------------
static void test_btn_api(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Espera 10 eventos de cada tipo");

	press_count = 0;
	hold_count = 0;
	release_count = 0;
	uint8_t _press_count = 0, _hold_count = 0, _release_count = 0;

	while(press_count < 10 || hold_count < 10 || release_count < 10){
		if(_press_count < press_count){
			DEBUG_TRACE_D(_EXPR_, _MODULE_, "PRESS %d", press_count);
			_press_count = press_count;
		}
		if(_hold_count < hold_count){
			DEBUG_TRACE_D(_EXPR_, _MODULE_, "HOLD %d", hold_count);
			_hold_count = hold_count;
		}
		if(_release_count < release_count){
			DEBUG_TRACE_D(_EXPR_, _MODULE_, "RELEASE %d", release_count);
			_release_count = release_count;
		}
	}
}

//------------------------------------------------------------------------------------
static void test_btn_disable_callbacks(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Desinstala button callbacks");

	btn->disablePressEvents();
	btn->disableHoldEvents();
	btn->disableReleaseEvents();
}


//------------------------------------------------------------------------------------
static void test_btn_destroy(){
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Destruyendo btn");
	TEST_ASSERT_NOT_NULL(btn);
	delete(btn);
	btn = NULL;
}

//------------------------------------------------------------------------------------
//-- TEST CASES ----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
TEST_CASE("Crea btn en CPU local", "[Driver_PushButton]") {
	test_btn_new_local();
}


//------------------------------------------------------------------------------------
TEST_CASE("Instala callbacks uid", "[Driver_PushButton]") {
	test_btn_enable_callbacks_uid();
}


//------------------------------------------------------------------------------------
TEST_CASE("Instala callbacks void", "[Driver_PushButton]") {
	test_btn_enable_callbacks_void();
}

//------------------------------------------------------------------------------------
TEST_CASE("Verifica funcionamiento", "[Driver_PushButton]") {
	test_btn_api();
}

//------------------------------------------------------------------------------------
TEST_CASE("Desinstala callbacks", "[Driver_PushButton]") {
	test_btn_disable_callbacks();
}

//------------------------------------------------------------------------------------
TEST_CASE("Destruye el btn", "[Driver_PushButton]") {
	test_btn_destroy();
}




//------------------------------------------------------------------------------------
//-- TEST ENRY POINT -----------------------------------------------------------------
//------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void firmwareStart(){
	esp_log_level_set(_MODULE_, ESP_LOG_DEBUG);
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Inicio del programa");
	Heap::setDebugLevel(ESP_LOG_DEBUG);
	unity_run_menu();
}


#endif
