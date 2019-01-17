/*
 * PushButton.cpp
 *
 *  Created on: 20/04/2015
 *      Author: raulMrello
 */

#include "PushButton.h"



//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------
/** Macro para imprimir trazas de depuraci�n, siempre que se haya configurado un objeto
 *	Logger v�lido (ej: _debug)
 */
static const char* _MODULE_ = "[PushBtn].......";
#define _EXPR_	(_defdbg)




//------------------------------------------------------------------------------------
static void nullCallback(uint32_t id){
}


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PushButton::PushButton(PinName btn, uint32_t id, LogicLevel level, PinMode mode, bool defdbg) : _defdbg(defdbg) {
    // Crea objeto
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando PushButton en pin %d", btn);
	_iin = new InterruptIn(btn);
	MBED_ASSERT(_iin);
	_iin->mode(mode);
	_iin->rise(NULL);
	_iin->fall(NULL);
    _level = level;
    _id = id;
    _hold_us = 0;
    _hold_running = false;
    
    // Desactiva las callbacks de notificaci�n
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Desactivando callbacks");
    _pressCb = callback(&nullCallback);
    _holdCb = callback(&nullCallback);
    _releaseCb = callback(&nullCallback);


    // Crea temporizadores
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando tickers de tarea");
    _tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce, "BtnTmrFilt");
    MBED_ASSERT(_tick_filt);
    _tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic, "BtnTmrHold");
    MBED_ASSERT(_tick_hold);
}


//------------------------------------------------------------------------------------
void PushButton::enablePressEvents(Callback<void(uint32_t)>pressCb){
	MBED_ASSERT(pressCb);
	_pressCb = pressCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableHoldEvents(Callback<void(uint32_t)>holdCb, uint32_t millis){
	MBED_ASSERT(holdCb && millis);
	_holdCb = holdCb;
	_hold_us = 1000 * millis;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableReleaseEvents(Callback<void(uint32_t)>releaseCb){
	MBED_ASSERT(releaseCb);
	_releaseCb = releaseCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::disablePressEvents(){
    _pressCb = callback(&nullCallback);
}


//------------------------------------------------------------------------------------
void PushButton::disableHoldEvents(){
	if(_hold_running){
		_tick_hold->stop();
	}
	_hold_running = false;
    _holdCb = callback(&nullCallback);
    _hold_us = 0;
}


//------------------------------------------------------------------------------------
void PushButton::disableReleaseEvents(){
    _releaseCb = callback(&nullCallback);
}



 
//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void PushButton::isrRiseCallback(){
	_iin->rise(NULL);
	_iin->fall(NULL);
	_curr_value = 1;
	_tick_filt->start(GlitchFilterTimeoutUs/1000);
}


//------------------------------------------------------------------------------------
void PushButton::isrFallCallback(){
	_iin->rise(NULL);
	_iin->fall(NULL);
	_curr_value = 0;
	_tick_filt->start(GlitchFilterTimeoutUs/1000);
}

//------------------------------------------------------------------------------------
void PushButton::gpioFilterCallback(){
	// leo valor del pin
    uint8_t pin_level = (uint8_t)_iin->read();

	// En caso de glitch, descarto y vuelvo a habilitar interrupciones
	if(_curr_value != pin_level){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NOISE");
		enableRiseFallCallbacks();
        return;
	}

	// En caso de evento RELEASE
	if((pin_level == 1 && _level == PressIsLowLevel) || (pin_level == 0 && _level == PressIsHighLevel)){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_RELEASE");
		if(_hold_running){
			_tick_hold->stop();
			_hold_running = false;
		}
		_releaseCb.call(_id);
		enableRiseFallCallbacks();
		return;
	}

    // En caso de evento PRESS
    if((pin_level == 1 && _level == PressIsHighLevel) || (pin_level == 0 && _level == PressIsLowLevel)){
    	DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_PRESS");
    	// si el timming para eventos hold est� configurado, primero lo detiene y luego lo inicia
		if(_hold_running){
			_tick_hold->stop();
			_hold_running = false;
		}
        if(_hold_us > 0){
        	_tick_hold->start(_hold_us/1000);
        	_hold_running = true;
        }
        _pressCb.call(_id);
        enableRiseFallCallbacks();
        return;
    }

    // No deber�a llegar a este punto nunca, pero por si acaso, reasigna isr's
    DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_LEVEL");
	if(_hold_running){
		_tick_hold->stop();
		_hold_running = false;
	}
    enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::holdTickCallback(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_HOLD");
    _holdCb.call(_id);
}


//------------------------------------------------------------------------------------
void PushButton::enableRiseFallCallbacks(){
	if((_curr_value = _iin->read())){
		_iin->rise(NULL);
		_iin->fall(callback(this, &PushButton::isrFallCallback));
	}
	else{
		_iin->rise(callback(this, &PushButton::isrRiseCallback));
		_iin->fall(NULL);
	}
}

