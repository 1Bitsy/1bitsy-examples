#include "debounce.h"

#include "systick.h"

void init_debounce(debounce *db, const gpio_pin *gp, uint32_t debounce_msec)
{
    gpio_init_pin(gp);
    db->db_port          = gp->gp_port;
    db->db_pin           = gp->gp_pin;
    db->db_previous_msec = 0;
    db->db_debounce_msec = debounce_msec;
    db->db_state         = gpio_get(db->db_port, db->db_pin) != 0;
    db->db_state_changed = false;
}

bool debounce_update(debounce *db)
{
    bool new_state = gpio_get(db->db_port, db->db_pin) != 0;
    if (new_state != db->db_state) {
        uint32_t now = system_millis;
        uint32_t dt = (uint32_t)((int32_t)now - (int32_t)db->db_previous_msec);
        if (dt >= db->db_debounce_msec) {
            db->db_previous_msec = now;
            db->db_state = new_state;
            db->db_state_changed = true;
            return true;
        }
    }
    db->db_state_changed = false;
    return false;
}

