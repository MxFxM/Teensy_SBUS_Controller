#include <Arduino.h>
#include <usb_joystick.h>

#define SBUS Serial2
uint8_t serial_buffer[30];
uint8_t serial_buffer_index = 0;

uint8_t databits[187]; // all data bits in one list
uint16_t channels[17]; // channels are 0 to 15
boolean failsafe = false;
boolean frame_lost = false;
boolean ch18 = false; // special channels 17 and 18 are digital
boolean ch17 = false;

void nop(void);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  SBUS.begin(100000, SERIAL_8E2);
  Joystick.useManualSend(true);
}

void loop()
{
  // put your main code here, to run repeatedly:
  uint32_t new_bytes = SBUS.available();
  if (new_bytes > 0)
  {
    for (uint32_t count = 0; count < new_bytes; count++)
    {
      serial_buffer[serial_buffer_index] = SBUS.read();
      serial_buffer_index++;

      switch (serial_buffer[0])
      {
      case 0x0F: // header byte
        if (serial_buffer_index == 25)
        { // length of one frame
          if (serial_buffer[24] == 0x00)
          { // footer byte
            // frame is complete
            for (uint8_t databyte = 0; databyte < 24; databyte++)
            { // for every byte except header and footer
              for (uint8_t bitnum = 0; bitnum < 8; bitnum++)
              { // for every bit
                // sort the bit in the list
                databits[databyte * 8 + bitnum] = (serial_buffer[1 + databyte] >> bitnum) & 0x01;
              }
            }

            for (uint8_t channelno = 0; channelno < 17; channelno++)
            { // for every channel

              // value is received through the bits from the list
              channels[channelno] = (databits[channelno * 11 + 0] << 0) +
                                    (databits[channelno * 11 + 1] << 1) +
                                    (databits[channelno * 11 + 2] << 2) +
                                    (databits[channelno * 11 + 3] << 3) +
                                    (databits[channelno * 11 + 4] << 4) +
                                    (databits[channelno * 11 + 5] << 5) +
                                    (databits[channelno * 11 + 6] << 6) +
                                    (databits[channelno * 11 + 7] << 7) +
                                    (databits[channelno * 11 + 8] << 8) +
                                    (databits[channelno * 11 + 9] << 9) +
                                    (databits[channelno * 11 + 10] << 10);
            }

            // special bits:
            failsafe = channels[16] & 0x08;
            frame_lost = channels[16] & 0x04;
            ch18 = channels[16] & 0x02;
            ch17 = channels[16] & 0x01;
          }

          // frame is done
          serial_buffer_index = 0;
        }
        break;
      default:
        // not the start byte
        serial_buffer_index = 0;
      }
    }
  }

  Joystick.X(map(channels[0], 192, 1792, 0, 1023));
  Joystick.Y(map(channels[1], 192, 1792, 0, 1023));
  Joystick.Z(map(channels[2], 192, 1792, 0, 1023));
  Joystick.Zrotate(map(channels[3], 192, 1792, 0, 1023));

  if (channels[4] > 853)
  {
    Joystick.button(1, 1);
  }
  else
  {
    Joystick.button(1, 0);
  }
  if (channels[5] > 853)
  {
    Joystick.button(2, 1);
  }
  else
  {
    Joystick.button(2, 0);
  }

  Joystick.send_now();
}

void nop(void)
{
}