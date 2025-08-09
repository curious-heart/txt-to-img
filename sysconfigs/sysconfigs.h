#ifndef SYSCONFIGS_H
#define SYSCONFIGS_H

#include <QString>
#include "common_tools/common_tool_func.h"

typedef struct
{
    int log_level;

    int bit_per_px;
}sys_configs_struct_t;

extern sys_configs_struct_t g_sys_configs_block;
extern  const int g_max_bit_per_px;

bool fill_sys_configs(QString *);

#endif // SYSCONFIGS_H
