/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <BLE_API.h>

#define BLE_UUID_TXRX_SERVICE            0x0000 /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_TX_CHARACTERISTIC       0x0002 /**< The UUID of the TX Characteristic. */
#define BLE_UUIDS_RX_CHARACTERISTIC      0x0003 /**< The UUID of the RX Characteristic. */

#define TXRX_BUF_LEN                     20
#define UART_RX_TIME                     APP_TIMER_TICKS(100, 0)

BLEDevice  ble;

static app_timer_id_t                    m_uart_rx_id; 
static uint8_t rx_buf[TXRX_BUF_LEN];
static uint8_t rx_buf_num, rx_state=0;

// The Nordic UART Service
static const uint8_t uart_base_uuid[] = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_tx_uuid[]   = {0x71, 0x3D, 0, 3, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_rx_uuid[]   = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_base_uuid_rev[] = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};

uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};

GattCharacteristic  txCharacteristic (uart_tx_uuid, txPayload, 1, TXRX_BUF_LEN,
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
                                      
GattCharacteristic  rxCharacteristic (uart_rx_uuid, rxPayload, 1, TXRX_BUF_LEN,
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
                                      
GattCharacteristic *uartChars[] = {&txCharacteristic, &rxCharacteristic};

GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));


void m_uart_rx_handle(void * p_context)
{
    ble.updateCharacteristicValue(rxCharacteristic.getHandle(), rx_buf, rx_buf_num);   
    memset(rx_buf, 0x00,20);
    rx_state = 0;
}

void uartCallBack(void)
{    
    uint32_t err_code = NRF_SUCCESS;
    
    if(rx_state == 0)
    {  
        rx_state = 1;
        err_code = app_timer_start(m_uart_rx_id, UART_RX_TIME, NULL);
        APP_ERROR_CHECK(err_code);   
        rx_buf_num=0;
    }
    while( Serial.available() )
    {
        rx_buf[rx_buf_num] = Serial.read();
        rx_buf_num++;
    }
}

void disconnectionCallback(void)
{
    ble.startAdvertising();
}

void onDataWritten(uint16_t charHandle)
{	
    uint8_t buf[TXRX_BUF_LEN];
    uint16_t bytesRead;
	
    if (charHandle == txCharacteristic.getHandle()) 
    {
        ble.readCharacteristicValue(txCharacteristic.getHandle(), buf, &bytesRead);
        for(uint8_t i=0; i<bytesRead; i++)
        {
            Serial.write(buf[i]);
        }
    }
}

void setup(void)
{
    uint32_t err_code = NRF_SUCCESS;
    
    uart_callback_t uart_cb;
    
    delay(500);
    Serial.begin(57600); // recommend setting 9600
    
    uart_cb = &uartCallBack;
    Serial.irq_attach(uart_cb);
    
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,(const uint8_t *)"Biscuit", sizeof("Biscuit") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));
    /* 100ms; in multiples of 0.625ms. */
    ble.setAdvertisingInterval(160); 

    ble.addService(uartService);
    
    ble.startAdvertising();
    
    err_code = app_timer_create(&m_uart_rx_id,APP_TIMER_MODE_SINGLE_SHOT, m_uart_rx_handle);
    APP_ERROR_CHECK(err_code);
}

void loop(void)
{
    ble.waitForEvent();
}
