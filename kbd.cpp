#include <Energia.h>
#include <PS2Keyboard.h>

#include "config.h"
#include "Memory.h"
#include "kbd.h"

static unsigned short keymap[128] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0014, 0x0277, 0x0276, 0x0275, 0x0274, 0x0273, 0x0272, 0x0271,
	0x0267, 0x0266, 0x0264, 0x0112, 0x0021, 0x0063, 0x0057, 0x0013,
	0x0065, 0x0077, 0x0076, 0x0075, 0x0074, 0x0073, 0x0072, 0x0071,
	0x0067, 0x0066, 0x0064, 0x0012, 0x0221, 0x0263, 0x0257, 0x0213,
	0x0211, 0x0016, 0x0024, 0x0026, 0x0036, 0x0046, 0x0035, 0x0034,
	0x0033, 0x0041, 0x0032, 0x0031, 0x0056, 0x0022, 0x0023, 0x0055,
	0x0011, 0x0017, 0x0045, 0x0037, 0x0044, 0x0042, 0x0025, 0x0047,
	0x0027, 0x0043, 0x0015, 0x0231, 0xffff, 0x0222, 0x0223, 0xffff,
	0x0256, 0x0016, 0x0024, 0x0026, 0x0036, 0x0046, 0x0035, 0x0034,
	0x0033, 0x0041, 0x0032, 0x0031, 0x0056, 0x0022, 0x0023, 0x0055,
	0x0011, 0x0017, 0x0045, 0x0037, 0x0044, 0x0042, 0x0025, 0x0047,
	0x0027, 0x0043, 0x0015, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};

kbd::kbd (): Memory::Device(1024), _last(0), _delay(128) {
  for (int i=8; i--; )
    _rows[i] = 0;
  // shift-lock on
  _set(0x00, true);
  _key = 0;
  _millis = 0;
}

void kbd::operator= (byte row) {
  _last = row;
}

byte kbd::pattern() {
  int r = 255-_last;
  byte pattern = 0;
  for (int i=8; i--; r>>=1) 
    if (r & 1)
      pattern |= _rows[7-i];

  if (_key != 0) {
    long now = millis();
    if (now - _millis > 50 || _millis > now) {
      up(_key);
      _key = 0;
      _millis = 0;
    }
  }

  return pattern ^ 0xff;
}

// returns whether shift-lock is pressed
bool kbd::_map (unsigned key, unsigned short &map) {
  switch (key) {
    case 0x08: 
      map = 0x62;
      break;
    case 0x0d:
      map = 0x53;
      break;
    case 0x0a:
      map = 0x0232;
      break;
    default:
      map = key<128? keymap[key]: 0xff;
      break;    
  }
  return true;
}

void kbd::_reset (byte k) {
  byte &r = _rows[(k & 0xf0) >> 4];
  byte c = 1 << (k & 0x0f);
  r &= ~c;
}

void kbd::up (unsigned key) {
  unsigned short k;
  if (_map (key, k) && k != 0xffff) {
    if (k > 0xff) _reset (k/0xff);
      _reset (k&0xff);
  }
}

void kbd::_set (byte k, bool lock) {
  byte &r = _rows[(k & 0xf0) >> 4];
  byte c = 1 << (k & 0x0f);
  if (lock)
    r ^= c;
  else
    r |= c;
}

void kbd::down (unsigned key) {
  unsigned short k;
  bool lock = !_map (key, k);
  if (k != 0xffff) {
    if (k > 0xff) _set (k/0xff, lock);
      _set (k&0xff, lock);
  }
  _millis = millis();
  _key = key;
}

