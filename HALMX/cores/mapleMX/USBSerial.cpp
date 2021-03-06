/****************************************************************************
 *
 * USBSerial core library for Arduino STM32 + HAL + CubeMX (HALMX).
 *
 * Copyright (c) 2016 by Vassilis Serasidis <info@serasidis.gr>
 * Home: http://www.serasidis.gr
 * email: avrsite@yahoo.gr
 * 
 * Arduino_STM32 forum: http://www.stm32duino.com
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * Some functions follow the sam and samd arduino core libray files.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 *
 ****************************************************************************/

#include  <chip.h>

#ifdef USE_USBSerial
#include <USBSerial.h>
#include "variant.h"

// Constructors ////////////////////////////////////////////////////////////////
USBSerial::USBSerial(){
  // Make sure Rx ring buffer is initialized back to empty.
  rx_buffer.iHead = rx_buffer.iTail = 0;
  //tx_buffer.iHead = tx_buffer.iTail = 0;
}


void USBSerial::init(void){
 
  /* Re-enumerate the USB */ 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  
  
#if defined(USB_DISC_PORT) 
	GPIO_InitStruct.Pin = USB_DISC_PIN;
	HAL_GPIO_Init(USB_DISC_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(USB_DISC_PORT, USB_DISC_PIN, GPIO_PIN_SET);
	for(volatile unsigned int i=0;i<512;i++);
	HAL_GPIO_WritePin(USB_DISC_PORT, USB_DISC_PIN, GPIO_PIN_RESET);
#else
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	for(volatile unsigned int i=0;i<512;i++);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
#endif
  
	MX_USB_DEVICE_Init();
}


void USBSerial::begin(uint32_t baud_count){
  init();
  // suppress "unused parameter" warning
	(void)baud_count;
}

void USBSerial::begin(uint32_t baud_count, uint8_t config){
  init();
	//suppress "unused parameter" warning
	(void)baud_count;
	(void)config;
}

void USBSerial::end(void){
#if defined(USB_DISC_PORT) 
	HAL_GPIO_WritePin(USB_DISC_PORT, USB_DISC_PIN, GPIO_PIN_SET);
#endif  
}


int USBSerial::availableForWrite(void){
  return (CDC_SERIAL_BUFFER_SIZE - available());
}


int USBSerial::available(void){
  return (uint32_t)(CDC_SERIAL_BUFFER_SIZE + rx_buffer.iHead - rx_buffer.iTail) % CDC_SERIAL_BUFFER_SIZE;
}

int USBSerial::peek(void)
{
  if ( rx_buffer.iHead == rx_buffer.iTail )
    return -1;

  return rx_buffer.buffer[rx_buffer.iTail];
}

int USBSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if ( rx_buffer.iHead == rx_buffer.iTail )
    return -1;

  uint8_t uc = rx_buffer.buffer[rx_buffer.iTail];
  rx_buffer.iTail = (unsigned int)(rx_buffer.iTail + 1) % CDC_SERIAL_BUFFER_SIZE;
  
  return uc;
}

void USBSerial::flush(void){
  //It's not implemented yet.
}
/*
size_t USBSerial::write(const uint8_t *buffer, size_t size){
  HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){
    CDC_Transmit_FS((uint8_t*)buffer, size);
  }
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  return 1;
}
*/

size_t USBSerial::write(const uint8_t *buffer, size_t size)
{
	unsigned long timeout=millis()+5;
  if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
  { 
    while(millis()<timeout)
	{
      if(CDC_Transmit_FS((uint8_t*)buffer, size) == USBD_OK)
	  {
         return size;
      }
    }
  }
  return 0;
}


size_t USBSerial::write(uint8_t c) {
	return write(&c, 1);
}

void USBSerial::CDC_RxHandler (uint8_t* Buf, uint16_t Len){

  for(uint16_t i=0;i<Len;i++){
    if(available() < (CDC_SERIAL_BUFFER_SIZE - 1)){
      rx_buffer.buffer[rx_buffer.iHead] = *Buf++;
      rx_buffer.iHead = (uint16_t)(rx_buffer.iHead + 1) % CDC_SERIAL_BUFFER_SIZE;
    }else
      break;
  }
}

void USBSerial::CDC_TxHandler(void){
  
/*   if(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED){
    if (tx_buffer.iHead != tx_buffer.iTail)	{
      unsigned char c = tx_buffer.buffer[tx_buffer.iTail];
      tx_buffer.iTail = (uint16_t)(tx_buffer.iTail + 1) % SERIAL_BUFFER_SIZE;
      CDC_Transmit_FS((uint8_t*)&c, 1);
    }
  } */
}
// This operator is a convenient way for a sketch to check whether the
// port has actually been configured and opened by the host (as opposed
// to just being connected to the host).  It can be used, for example, in
// setup() before printing to ensure that an application on the host is
// actually ready to receive and display the data.
// We add a short delay before returning to fix a bug observed by Federico
// where the port is configured (lineState != 0) but not quite opened.
USBSerial::operator bool()
{
	// this is here to avoid spurious opening after upload
	if (millis() < 500)
		return false;

	bool result = false;

/* 	if (_usbLineInfo.lineState > 0)
	{
		result = true;
	}

	delay(10); */
	return result;
}

uint32_t USBSerial::baud() {
	//return _usbLineInfo.dwDTERate;
  return 0;
}

uint8_t USBSerial::stopbits() {
	//return _usbLineInfo.bCharFormat;
  return 0;
}

uint8_t USBSerial::paritytype() {
	//return _usbLineInfo.bParityType;
  return 0;
}

uint8_t USBSerial::numbits() {
	//return _usbLineInfo.bDataBits;
  return 0;
}

bool USBSerial::dtr() {
	//return _usbLineInfo.lineState & 0x1;
  return 0;
}

bool USBSerial::rts() {
	//return _usbLineInfo.lineState & 0x2;
  return 0;
}

#endif
