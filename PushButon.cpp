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
/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */
static const char* _MODULE_ = "[PushBtn].......";
#define _EXPR_	(_defdbg && !IS_ISR())


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PushButton::PushButton(PinName32 btn, uint32_t id, LogicLevel level, PinMode mode, uint32_t filter_us, bool defdbg) : _defdbg(defdbg) {
    // Crea objeto
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando PushButton en pin %d", btn);
	_iin = new InterruptIn((PinName)btn);
	MBED_ASSERT(_iin);
	_iin->mode(mode);
	_iin->rise(NULL);
	_iin->fall(NULL);
    _level = level;
    _id = id;
    _hold_us = 0;
    _hold_running = false;
    _endis_gfilt = true;
    _filter_timeout_us = filter_us;
    
    // Desactiva las callbacks de notificación
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Desactivando callbacks");
    _pressCb = (Callback<void(uint32_t)>) NULL;
    _holdCb = (Callback<void(uint32_t)>) NULL;
    _releaseCb = (Callback<void(uint32_t)>) NULL;
    _pressCb2 = (Callback<void()>) NULL;
    _holdCb2 = (Callback<void()>) NULL;
    _releaseCb2 = (Callback<void()>) NULL;


    // Crea temporizadores
    DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando tickers de tarea");
	#if __MBED__==1
    _tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce);
    MBED_ASSERT(_tick_filt);
    _tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic);
    MBED_ASSERT(_tick_hold);
	#elif ESP_PLATFORM==1
    _tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce, "BtnTmrFilt");
    MBED_ASSERT(_tick_filt);
    _tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic, "BtnTmrHold");
    MBED_ASSERT(_tick_hold);
	#endif
    sprintf(_th_name,"pushb_%x", (uint32_t)this);
    _th = new Thread(osPriorityNormal, OS_STACK_SIZE, NULL, _th_name);
    MBED_ASSERT(_th);
    _th->start(callback(this, &PushButton::_task));

}

PushButton::PushButton(uint32_t id, LogicLevel level, uint32_t filter_us, bool defdbg){
	// Crea objeto
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando PushButton SIN control de pin con ID %d", id);
	_iin = NULL;
//	_iin = new InterruptIn((PinName)btn);
//	MBED_ASSERT(_iin);
//	_iin->mode(mode);
//	_iin->rise(NULL);
//	_iin->fall(NULL);
	_level = level;
	_id = id;
	_hold_us = 0;
	_hold_running = false;
	_endis_gfilt = true;
	_filter_timeout_us = filter_us;
	_pin_level = 0;

	// Desactiva las callbacks de notificación
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Desactivando callbacks");
	_pressCb = (Callback<void(uint32_t)>) NULL;
	_holdCb = (Callback<void(uint32_t)>) NULL;
	_releaseCb = (Callback<void(uint32_t)>) NULL;
	_pressCb2 = (Callback<void()>) NULL;
	_holdCb2 = (Callback<void()>) NULL;
	_releaseCb2 = (Callback<void()>) NULL;


	// Crea temporizadores
	DEBUG_TRACE_I(_EXPR_, _MODULE_, "Creando tickers de tarea");
	#if __MBED__==1
	_tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce);
	MBED_ASSERT(_tick_filt);
	_tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic);
	MBED_ASSERT(_tick_hold);
	#elif ESP_PLATFORM==1
	_tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce, "BtnTmrFilt");
	MBED_ASSERT(_tick_filt);
	_tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic, "BtnTmrHold");
	MBED_ASSERT(_tick_hold);
	#endif
	sprintf(_th_name,"pushb_%x", (uint32_t)this);
	_th = new Thread(osPriorityNormal, OS_STACK_SIZE, NULL, _th_name);
	MBED_ASSERT(_th);
	_th->start(callback(this, &PushButton::_task));
}


//------------------------------------------------------------------------------------
PushButton::~PushButton() {
	delete(_tick_filt);
	delete(_tick_hold);
	if(_iin)
		delete(_iin);
	delete(_th);
}


//------------------------------------------------------------------------------------
void PushButton::enablePressEvents(Callback<void(uint32_t)>pressCb){
	if(!pressCb){
		disablePressEvents();
		return;
	}
	_pressCb = pressCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enablePressEvents(Callback<void()>pressCb){
	if(!pressCb){
		disablePressEvents();
		return;
	}
	_pressCb2 = pressCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableHoldEvents(Callback<void(uint32_t)>holdCb, uint32_t millis){
	if(!holdCb || millis == 0){
		disableHoldEvents();
		return;
	}
	_holdCb = holdCb;
	_hold_us = 1000 * millis;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableHoldEvents(Callback<void()>holdCb, uint32_t millis){
	if(!holdCb || millis == 0){
		disableHoldEvents();
		return;
	}
	_holdCb2 = holdCb;
	_hold_us = 1000 * millis;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableReleaseEvents(Callback<void(uint32_t)>releaseCb){
	if(!releaseCb){
		disableReleaseEvents();
		return;
	}
	_releaseCb = releaseCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enableReleaseEvents(Callback<void()>releaseCb){
	if(!releaseCb){
		disableReleaseEvents();
		return;
	}
	_releaseCb2 = releaseCb;
	enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::disablePressEvents(){
    _pressCb = (Callback<void(uint32_t)>) NULL;
    _pressCb2 = (Callback<void()>) NULL;
}


//------------------------------------------------------------------------------------
void PushButton::disableHoldEvents(){
	if(_hold_running){
		_tick_hold->stop();
	}
	_hold_running = false;
    _holdCb = (Callback<void(uint32_t)>) NULL;
    _holdCb2 = (Callback<void()>) NULL;
    _hold_us = 0;
}


//------------------------------------------------------------------------------------
void PushButton::disableReleaseEvents(){
    _releaseCb = (Callback<void(uint32_t)>) NULL;
    _releaseCb2 = (Callback<void()>) NULL;
}


//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void PushButton::_task(){
	for(;;){
		osEvent oe = _th->signal_wait(osFlagsWaitAny, osWaitForever);

		// Evalua rise
		if(oe.status == osEventSignal &&  (oe.value.signals & EvRise) != 0){
			_curr_value = 1;
			if(_endis_gfilt){
				_tick_filt->start(_filter_timeout_us/1000);
			}
			else{
				gpioFilterCallback();
			}
		}
		// Evalua fall
		if(oe.status == osEventSignal &&  (oe.value.signals & EvFall) != 0){
			_curr_value = 0;
			if(_endis_gfilt){
				_tick_filt->start(_filter_timeout_us/1000);
			}
			else{
				gpioFilterCallback();
			}
		}
	}
}



//------------------------------------------------------------------------------------
void PushButton::isrRiseCallback(){
	if(_iin){
		_iin->rise(NULL);
		_iin->fall(NULL);
	}
	_th->signal_set(EvRise);
}


//------------------------------------------------------------------------------------
void PushButton::isrFallCallback(){
	if(_iin){
		_iin->rise(NULL);
		_iin->fall(NULL);
	}
	_th->signal_set(EvFall);
}


//------------------------------------------------------------------------------------
void PushButton::holdTickCallback(){
	DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_HOLD");
	if(_holdCb){
		_holdCb.call(_id);
	}
	if(_holdCb2){
		_holdCb2.call();
	}
}


//------------------------------------------------------------------------------------
void PushButton::gpioFilterCallback(){
	// leo valor del pin
    uint8_t pin_level = _pin_level;
    if(_iin){
    	pin_level = (uint8_t)_iin->read();
    }

	// En caso de glitch, descarto y vuelvo a habilitar interrupciones
	if(_curr_value != pin_level){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NOISE");
		if(_iin)enableRiseFallCallbacks();
        return;
	}

	// En caso de evento RELEASE
	if((pin_level == 1 && _level == PressIsLowLevel) || (pin_level == 0 && _level == PressIsHighLevel)){
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_RELEASE");
		if(_hold_running){
			_tick_hold->stop();
			_hold_running = false;
		}
		if(_releaseCb){
			_releaseCb.call(_id);
		}
		if(_releaseCb2){
			_releaseCb2.call();
		}
		if(_iin)enableRiseFallCallbacks();
		return;
	}

    // En caso de evento PRESS
    if((pin_level == 1 && _level == PressIsHighLevel) || (pin_level == 0 && _level == PressIsLowLevel)){
    	DEBUG_TRACE_D(_EXPR_, _MODULE_, "EV_PRESS");
    	// si el timming para eventos hold está configurado, primero lo detiene y luego lo inicia
		if(_hold_running){
			_tick_hold->stop();
			_hold_running = false;
		}
        if(_hold_us > 0){
        	_tick_hold->start(_hold_us/1000);
        	_hold_running = true;
        }
        if(_pressCb){
        	_pressCb.call(_id);
        }
        if(_pressCb2){
        	_pressCb2.call();
        }
        if(_iin)enableRiseFallCallbacks();
        return;
    }

    // No debería llegar a este punto nunca, pero por si acaso, reasigna isr's
    DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_LEVEL");
	if(_hold_running){
		_tick_hold->stop();
		_hold_running = false;
	}
    if(_iin)enableRiseFallCallbacks();
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

