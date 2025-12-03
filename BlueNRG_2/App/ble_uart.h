/*
 * ble_uart.h
 *
 *  Created on: Nov 25, 2025
 *      Author: ethan
 */

#ifndef APP_BLE_UART_H_
#define APP_BLE_UART_H_



#include <stdint.h>
#include "bluenrg1_types.h"

extern uint16_t BleUartServiceHandle;
extern uint16_t BleUartTxCharHandle;
extern uint16_t BleUartRxCharHandle;

tBleStatus BLE_UART_AddService(void);
tBleStatus BLE_UART_Send(uint8_t *data, uint8_t len);
void BLE_UART_RxHandler(uint8_t *data, uint16_t len);




#endif /* APP_BLE_UART_H_ */
