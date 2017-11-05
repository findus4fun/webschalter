/*
 * timer.h
 *
 *  Created on: 05.11.2017
 *      Author: jan
 */

#ifndef TIMER_H_
#define TIMER_H_

void Ereignis_Timers_Zeigen();
//void TimerNr_speichern(int datensatz);
void Ereignis_DeleteTimer();
void Ereignis_NeueTimer();
void TimerNr_speichern(int datensatz);
void Timers_pruefen(unsigned long* Zeit);


#endif /* TIMER_H_ */
