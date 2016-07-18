#ifndef DEBOUNCE_included
#define DEBOUNCE_included

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"

typedef struct debounce {
    uint32_t db_port;
    uint16_t db_pin;
    uint32_t db_previous_msec;
    uint32_t db_debounce_msec;
    uint8_t  db_state;
    bool     db_state_changed;
} debounce;

extern void init_debounce(debounce *, const gpio_pin *, uint32_t debounce_msec);

extern bool debounce_update(debounce *);
static inline bool debounce_is_rising_edge(const debounce *);
static inline bool debounce_is_falling_edge(const debounce *);

// Implementation

static inline bool debounce_is_rising_edge(const debounce *db)
{
    return db->db_state_changed && db->db_state;
}

static inline bool debounce_is_falling_edge(const debounce *db)
{
    return db->db_state_changed && !db->db_state;
}

#endif /* !DEBOUNCE_included */
