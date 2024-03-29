/*
 * PushButton.h
 *
 *  Created on: Sep 2017
 *      Author: raulMrello
 *
 *	PushButton es el m�dulo encargado de gestionar los eventos generados por un pulsador asociado a un GPIO de entrada
 *  Podr� generar eventos de pulsaci�n "Press", de mantenimiento "Hold" y de liberaci�n "Release", para los que proporcionar�
 *  una callback.
 *  Puesto que es un m�dulo que responde a eventos hardware, no tendr� asociado un hilo de ejecuci�n de propio y deber� ser
 *  el m�dulo controlador, el que se encargue de procesar las callbacks asociadas a los eventos de pulsaci�n.
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
    ~PushButton();
  
  
	/** Instala callback para procesar los eventos de pulsaci�n. La callback
	 *  se ejecutar� en contexto de tarea
     *  @param pressCb Callback a instalar
     */
    void enablePressEvents(Callback<void(uint32_t)>pressCb);
    void enablePressEvents(Callback<void()>pressCb);
  
    
	/** Instala callback para procesar los eventos de mantenimiento. La callback
	 *  se ejecutar� en contexto de tarea
     *  @param holdCb Callback a instalar
     *  @param millis Milisegundos tras los que se genera el evento peri�dico
     */
    void enableHoldEvents(Callback<void(uint32_t)>holdCb, uint32_t millis);
    void enableHoldEvents(Callback<void()>holdCb, uint32_t millis);

    
	/** Instala callback para procesar los eventos de liberaci�n. La callback
	 *  se ejecutar� en contexto de tarea
     *  @param releaseCb Callback a instalar
     */
    void enableReleaseEvents(Callback<void(uint32_t)>releaseCb);
    void enableReleaseEvents(Callback<void()>releaseCb);
  
  
	/** disablePressEvents
     *  Desinstala callback para procesar los eventos de pulsaci�n
     */
    void disablePressEvents();
  
	/** disableHoldEvents
     *  Desinstala callback para procesar los eventos de mantenimiento
     */
    void disableHoldEvents();
  
	/** disableReleaseEvents
     *  Desinstala callback para procesar los eventos de liberaci�n
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


  private:

    /** Eventos de teclado */
    static const uint32_t EvRise 	= (1<<0);
    static const uint32_t EvFall 	= (1<<1);

    uint32_t _filter_timeout_us;
    InterruptIn* _iin;						/// InterruptIn asociada
    LogicLevel _level;                      /// Nivel l�gico
    Callback<void(uint32_t)> _pressCb;      /// Callback para notificar eventos de pulsaci�n
    Callback<void()> 		 _pressCb2;     /// Callback para notificar eventos de pulsaci�n
    Callback<void(uint32_t)> _holdCb;       /// Callback para notificar eventos de mantenimiento
    Callback<void()> 		 _holdCb2;      /// Callback para notificar eventos de mantenimiento
    Callback<void(uint32_t)> _releaseCb;    /// Callback para notificar eventos de liberaci�n
    Callback<void()> 		 _releaseCb2;   /// Callback para notificar eventos de liberaci�n
    RtosTimer* _tick_filt;
    RtosTimer* _tick_hold;
    bool _hold_running;						/// flag para indicar si el timer hold est� en curso
    uint32_t _hold_us;                      /// Microsegundos entre eventos hold
    uint32_t _id;                           /// Identificador del pulsador
    bool _defdbg;							/// Flag para activar las trazas de depuraci�n por defecto
    uint8_t _curr_value;					/// Valor recien le�do del InterruptIn
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
     *  ISR para procesar eventos de temporizaci�n tras filtrado de glitches en cambios de nivel
     */
    void gpioFilterCallback();
  
	/** tickCallback
     *  ISR para procesar eventos de temporizaci�n
     */
    void holdTickCallback();

	/** enableRiseFallCallbacks
     *  Ajsuta las callbacks en funci�n del nivel l�gico actual
     */
    void enableRiseFallCallbacks();


    /**
     * Hilo de control
     */
    void _task();
  
};
     


#endif /*__PushButton__H */

/**** END OF FILE ****/


