# AVR Firmware Uploader

August 17 2022 - v1.0.1

Yes, it's just a simple GUI for avrdude.

It is based on https://github.com/maddsua/arduinouploader

Features:
- Upload firmware files by just defining your Arduino Board and selecting the firmware file
- Sometimes the ProMicro gets a time out from AVRDude. In this case just start again flashing

<img src="info/interface-101.jpg" width="356"/>


## List of arduino boards
- Arduino Mega2560
- ProMicro
- Uno
- Nano (with old and new bootloader)
- Mini
- Pro/Mini (with separate USB adapter)

## Erase EEPROM
For the above mentioned boards also an erase EEPROM .hex file is included.
Choose the file according your board.
If you upload this firmware, the existing firmware and the eeprom content will be erased.
Afterwords upload your firmware file again.

---

### This package includes next open-source binaries:

 - avrdude 6.3.0 which is included in the download file under build-win

Just download the zip package, extract it in an own folder and run MobiDude.exe


[Download package 📦](build-win/MobiDude-1.0.1.zip)
