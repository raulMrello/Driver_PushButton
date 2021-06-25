/*
 * PushButton.h
 *
 *  Created on: Sep 2017
 *      Author: raulMrello
 *
 *	PushButton es el módulo encargado de gestionar los eventos generados por un pulsador asociado a un GPIO de entrada
 *  Podrá generar eventos de pulsación "Press", de mantenimiento "Hold" y de liberación "Release", para los que proporcionará
 *  una callback.
 *  Puesto que es un módulo que responde a eventos hardware, no tendrá asociado un hilo de ejecución de propio y deberá ser
 *  el módulo controlador, el que se encargue de procesar las callbacks asociadas a los eventos de pulsación.
 */
 
#ifndef __PushButton__H
#define __PushButton__H

#include "mbed.h"
#if __MBED__==1
#include "mdf_api_cortex.h"
#endif


   
class PushButton {
  public:
	static const uint32_t GlitchFilterTimeoutUs = 500000;    /// Por defecto 20ms de timeout antiglitch desde el cambio de nivel

    enum LogicLevel{
        PressIsLowLevel,
        PressIsHighLevel
    };
    
	/** Constructor y Destructor por defecto */
    PushButton(PinName32 btn, uint32_t id, LogicLevel level, PinMode mode, uint32_t filter_us = GlitchFilterTimeoutUs, bool defdbg = false);
    PushButton(uint32_t id, LogicLevel level, uint32_t filter_us = GlitchFilterTimeoutUs, bool defdbg = false);
    ~PushButton();
  
  
	/** Instala callback para procesar los eventos de pulsación. La callback
	 *  se ejecutará en contexto de tarea
     *  @param pressCb Callback a instalar
     */
    void enablePressEvents(Callback<void(uint32_t)>pressCb);
    void enablePressEvents(Callback<void()>pressCb);
  
    
	/** Instala callback para procesar los eventos de mantenimiento. La callback
	 *  se ejecutará en contexto de tarea
     *  @param holdCb Callback a instalar
     *  @param millis Milisegundos tras los que se genera el evento periódico
     */
    void enableHoldEvents(Callback<void(uint32_t)>holdCb, uint32_t millis);
    void enableHoldEvents(Callback<void()>holdCb, uint32_t millis);

    
	/** Instala callback para procesar los eventos de liberación. La callback
	 *  se ejecutará en contexto de tarea
     *  @param releaseCb Callback a instalar
     */
    void enableReleaseEvents(Callback<void(uint32_t)>releaseCb);
    void enableReleaseEvents(Callback<void()>releaseCb);
  
  
	/** disablePressEvents
     *  Desinstala callback para procesar los eventos de pulsación
     */
    void disablePressEvents();
  
	/** disableHoldEvents
     *  Desinstala callback para procesar los eventos de mantenimiento
     */
    void disableHoldEvents();
  
	/** disableReleaseEvents
     *  Desinstala callback para procesar los eventos de liberación
     */
    void disableReleaseEvents();

    /** Habilita el filtro anti-glitch
     *
     */
    void enableGlitchFilter(){ _endis_gfilt = true; }


    /** Desactiva el filtro anti-glitch
     *
     */
    void disableGlitchFilter() { _endis_gfilt = false; }

    /*
     * Setea el último valor leído
     * */

    void setValue(bool val){_pin_level = val;}


  private:

    /** Eventos de teclado */
    static const uint32_t EvRise 	= (1<<0);
    static const uint32_t EvFall 	= (1<<1);

    uint32_t _filter_timeout_us;
    InterruptIn* _iin;						/// InterruptIn asociada
    LogicLevel _level;                      /// Nivel lógico
    Callback<void(uint32_t)> _pressCb;      /// Callback para notificar eventos de pulsación
    Callback<void()> 		 _pressCb2;     /// Callback para notificar eventos de pulsación
    Callback<void(uint32_t)> _holdCb;       /// Callback para notificar eventos de mantenimiento
    Callback<void()> 		 _holdCb2;      /// Callback para notificar eventos de mantenimiento
    Callback<void(uint32_t)> _releaseCb;    /// Callback para notificar eventos de liberación
    Callback<void()> 		 _releaseCb2;   /// Callback para notificar eventos de liberación
    RtosTimer* _tick_filt;
    RtosTimer* _tick_hold;
    bool _hold_running;						/// flag para indicar si el timer hold está en curso
    uint32_t _hold_us;                      /// Microsegundos entre eventos hold
    uint32_t _id;                           /// Identificador del pulsador
    bool _defdbg;							/// Flag para activar las trazas de depuración por defecto
    uint8_t _curr_value;					/// Valor recien leído del InterruptIn
    bool _endis_gfilt;						/// Flag de control del filtro anti-glitch
    Thread* _th;							/// Controlador del hilo
    char _th_name[24];

	/** isrRiseCallback
     *  ISR para procesar eventos de cambio de nivel
     */
    void isrRiseCallback();
    
	/** isrFallCallback
     *  ISR para procesar eventos de cambio de nivel
     */
    void isrFallCallback();
  
	/** gpioFilterCallback
     *  ISR para procesar eventos de temporización tras filtrado de glitches en cambios de nivel
     */
    void gpioFilterCallback();
  
	/** tickCallback
     *  ISR para procesar eventos de temporización
     */
    void holdTickCallback();

	/** enableRiseFallCallbacks
     *  Ajsuta las callbacks en función del nivel lógico actual
     */
    void enableRiseFallCallbacks();


    /**
     * Hilo de control
     */
    void _task();

    uint32_t _pin_level;
  
};
     


#endif /*__PushButton__H */

/**** END OF FILE ****/


