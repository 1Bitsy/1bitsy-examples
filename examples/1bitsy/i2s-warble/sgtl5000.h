#ifndef SGTL5000_included
#define SGTL5000_included

#include "i2c.h"

extern void init_sgtl5000(const i2c_channel *i2c);

// volume: -51.5dB .. +12dB in 1/2 dB steps
extern void sgtl_set_volume(const i2c_channel *i2c, float left, float right);

#endif /* !SGTL5000_included */

