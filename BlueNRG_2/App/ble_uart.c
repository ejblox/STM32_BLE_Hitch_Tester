#include "ble_uart.h"
#include "bluenrg1_aci.h"
#include "gatt_db.h"
#include "app_bluenrg_2.h"   // for connection_handle / connected

uint16_t BleUartServiceHandle = 0;
uint16_t BleUartTxCharHandle  = 0;
uint16_t BleUartRxCharHandle  = 0;

extern volatile uint8_t monitor_button;


extern volatile uint8_t connected;


// 128-bit UUIDs (little-endian)
static const uint8_t BLE_UART_SERVICE_UUID[16] =
{ 0xF0,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x12 };

static const uint8_t BLE_UART_TX_UUID[16] =
{ 0xF1,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x12 };

static const uint8_t BLE_UART_RX_UUID[16] =
{ 0xF2,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12,0x34,0x12,0x78,0x56,0x34,0x12,0x56,0x12 };

// max payload per packet (default ATT MTU = 23 => 20 data bytes)
#define BLE_UART_MAX_DATA_LEN   (20)

tBleStatus BLE_UART_AddService(void)
{
  tBleStatus ret;
  Service_UUID_t  service_uuid;
  Char_UUID_t     char_uuid;

  /* Copy UUIDs */
  memcpy(service_uuid.Service_UUID_128, BLE_UART_SERVICE_UUID, 16);

  ret = aci_gatt_add_service(UUID_TYPE_128,
                          &service_uuid,
                          PRIMARY_SERVICE,
                          /* max number of attributes:
                             service + 2x(char decl + value + CCCD) = 7 */
                          7,
                          &BleUartServiceHandle);
  if (ret != BLE_STATUS_SUCCESS) return ret;

  /* TX characteristic: Notify (board → phone) */
  memcpy(char_uuid.Char_UUID_128, BLE_UART_TX_UUID, 16);
  ret = aci_gatt_add_char(BleUartServiceHandle,
                          UUID_TYPE_128,
                          &char_uuid,
                          BLE_UART_MAX_DATA_LEN,
                          CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                          ATTR_PERMISSION_NONE,
                          GATT_NOTIFY_ATTRIBUTE_WRITE,   // event mask
                          16,                            // encryKeySize
                          0,                             // isVariable
                          &BleUartTxCharHandle);
  if (ret != BLE_STATUS_SUCCESS) return ret;

  /* RX characteristic: Write Without Response (phone → board) */
  memcpy(char_uuid.Char_UUID_128, BLE_UART_RX_UUID, 16);
  ret = aci_gatt_add_char(BleUartServiceHandle,
                          UUID_TYPE_128,
                          &char_uuid,
                          BLE_UART_MAX_DATA_LEN,
                          CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE,
                          ATTR_PERMISSION_NONE,
                          GATT_NOTIFY_ATTRIBUTE_WRITE,
                          16,
                          0,
                          &BleUartRxCharHandle);
  return ret;
}

/* Send data to phone (TX notify) */
tBleStatus BLE_UART_Send(uint8_t *data, uint8_t len)
{
  if (!connected) return BLE_STATUS_FAILED;  // now 'connected' is known here
  if (len > BLE_UART_MAX_DATA_LEN) len = BLE_UART_MAX_DATA_LEN;

  return aci_gatt_update_char_value(BleUartServiceHandle,
                                    BleUartTxCharHandle,
                                    0,
                                    len,
                                    data);
}

/* Called when phone writes to RX characteristic.
   For now just weak-define; you can handle it in main code. */
void BLE_UART_RxHandler(uint8_t *data, uint16_t len)
{
    // Example protocol:
    // data[0] == 0x01  → start monitoring B1
    // (you can expand this later for more commands)

    if (len >= 1 && data[0] == 0x01)
    {
        monitor_button = 1;
        PRINT_DBG("CMD: start monitoring button\r\n");
    }
    else
    {
        PRINT_DBG("RX (ignored) %d bytes\r\n", len);
    }

    // No echo here anymore, unless you still want it
    // BLE_UART_Send(data, (uint8_t)len);
}
