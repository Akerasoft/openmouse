# Openmouse: NES/SNES/SNES Mouse/N64/Gamecube/Genesis/Atari to Wiimote/NES Classic adapter firmware

This is a firmware for using a variety of controllers on a Wii/Wii U through a Wii remote. The NES Classic edition
is also supported.

This specifically supports the SNES Mouse.  It is intended for an SNES Classic where
Mario Paint or a different SNES Mouse game is loaded.  Note that legal ripping
of a cartridge that you own is assumed, because Akerasoft does not support piracy.

## Project homepage

The schematic is the same as the Raphnet Extenmote, but an updated schematic will be provided.

Schematics and additional information such as button mappings are available on the project homepage:

English: [Extenmote](http://www.raphnet.net/electronique/extenmote/index_en.php)
French: [Extenmote](http://www.raphnet.net/electronique/extenmote/index.php)

## Supported micro-controller(s)

Currently supported micro-controller(s):

* Atmega8
* Atmega168

Adding support for other micro-controllers should be easy, as long as the target has enough
IO pins and enough memory (flash and SRAM).

For N64 and Gamecube controller compatiblity, the micro-controller must be clocked at 12 MHz.

## Built with

* [avr-gcc](https://gcc.gnu.org/wiki/avr-gcc)
* [avr-libc](http://www.nongnu.org/avr-libc/)
* [gnu make](https://www.gnu.org/software/make/manual/make.html)

## License

This project is licensed under the terms of the GNU General Public License, version 3.

## Acknowledgments

* Thanks to the authors of the [wiibrew wiki](http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Classic_Controller) the excellent Wiimote accessory protocol documentation.
* Thanks to Raphaël Assenat of Raphnet for his original project.
