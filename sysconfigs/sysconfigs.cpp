#include <QSettings>

#include "logger/logger.h"
#include "sysconfigs/sysconfigs.h"
#include "literal_strings/literal_strings.h"

#undef ENUM_NAME_DEF
#define ENUM_NAME_DEF(e, ...) <<e

static const int gs_min_bit_per_px = 1;
const int g_max_bit_per_px = 16;

static const char* gs_sysconfigs_file_fpn = "./configs/configs.ini";

static const char* gs_ini_grp_sys_cfgs = "sys_cfgs";
static const char* gs_ini_key_log_level = "log_level";

static const char* gs_ini_grp_data_proc_cfg = "data_proc_cfg";
static const char* gs_ini_key_bit_per_px = "bit_per_px";

sys_configs_struct_t g_sys_configs_block;

static const int gs_def_log_level = LOG_ERROR;
static const int gs_def_bit_per_px = 12;

static RangeChecker<int> gs_cfg_file_log_level_ranger((int)LOG_DEBUG, (int)LOG_ERROR, "",
                     EDGE_INCLUDED, EDGE_INCLUDED);

static RangeChecker<int> gs_cfg_file_value_ge0_int_ranger(0, 0, "",
                           EDGE_INCLUDED, EDGE_INFINITE);

static RangeChecker<double> gs_cfg_file_value_ge0_double_ranger(0, 0, "",
                       EDGE_INCLUDED, EDGE_INFINITE);

static RangeChecker<int> gs_cfg_file_value_gt0_int_ranger(0, 0, "",
                       EDGE_EXCLUDED, EDGE_INFINITE);

static RangeChecker<int> gs_cfg_file_value_01_int_ranger(0, 1, "",
                       EDGE_INCLUDED, EDGE_INCLUDED);

static RangeChecker<double> gs_cfg_file_value_gt0_double_ranger(0, 0, "",
                       EDGE_EXCLUDED, EDGE_INFINITE);

RangeChecker<int> g_ip_port_ranger(0, 65535, "", EDGE_INCLUDED, EDGE_INCLUDED);
RangeChecker<int> g_12bitpx_value_ranger(0, g_12bitpx_max_v, "", EDGE_INCLUDED, EDGE_INCLUDED);
RangeChecker<double> g_12bitpx_stre_factor_ranger(1, g_12bitpx_max_v, "",
                       EDGE_INCLUDED, EDGE_INCLUDED);


/*the __VA_ARGS__ should be empty or a type converter like (cust_type).*/
#define GET_INF_CFG_NUMBER_VAL(settings, key, type_func, var, def, factor, checker, ...)\
{\
    (var) = __VA_ARGS__((factor) * ((settings).value((key), (def)).type_func()));\
    if((checker) && !((checker)->range_check((var))))\
    {\
        (var) = (def);\
    }\
}

#define BEGIN_INT_RANGE_CHECK(low, up, low_inc, up_inc)\
{\
    RangeChecker<int> int_range_checker((low), (up), "", low_inc, up_inc);
#define END_INT_RANGE_CHECK \
}

#define CHECK_ENUM(title_str, e_v, e_set, str_func) \
    cfg_ret = true; ret_str += (ret_str.isEmpty() ? "" : "\n");\
    if(!e_set.contains(e_v))\
    {\
        cfg_ret = false;\
        ret_str += QString(title_str) + gs_str_should_be_one_val_of + "\n{";\
        auto it = e_set.constBegin();\
        while(it != e_set.constEnd()) {ret_str += str_func(*it) + ", "; ++it;}\
        ret_str.chop(2);\
        ret_str += "}\n";\
        ret_str += QString(gs_str_actual_val) + str_func(e_v) + "\n";\
    }\
    ret = ret && cfg_ret;

/*check the validation of config parameters.*/
#define CHECK_LIMIT_RANGE(l_name, min_l, max_l, checker, unit_str) \
    cfg_ret = true; \
    if(((checker) && (!((checker)->range_check(min_l)) || !((checker)->range_check(max_l)))) \
        || ((min_l) > (max_l)))\
    {\
        cfg_ret = false;\
        ret_str += QString((l_name)) + \
                   " [" + QString::number((min_l)) + ", " + QString::number((max_l)) + "] " +\
                   (unit_str) + "\n";\
    }\
    ret = ret && cfg_ret;

bool fill_sys_configs(QString * ret_str_ptr)
{
    bool ret = true;
    QString ret_str;
    QSettings settings(gs_sysconfigs_file_fpn, QSettings::IniFormat);

    /*--------------------*/
    settings.beginGroup(gs_ini_grp_sys_cfgs);
    GET_INF_CFG_NUMBER_VAL(settings, gs_ini_key_log_level, toInt,
                           g_sys_configs_block.log_level, gs_def_log_level,
                           1, &gs_cfg_file_log_level_ranger);
    settings.endGroup();

    /*--------------------*/
    settings.beginGroup(gs_ini_grp_data_proc_cfg);
    GET_INF_CFG_NUMBER_VAL(settings, gs_ini_key_bit_per_px, toInt,
                           g_sys_configs_block.bit_per_px, gs_def_bit_per_px,
                           1, (RangeChecker<int>*)0);
    if(g_sys_configs_block.bit_per_px > g_max_bit_per_px
            || g_sys_configs_block.bit_per_px < gs_min_bit_per_px)
    {
        ret = false;
        ret_str += QString("%1：%2 应在[%3, %4]范围内，%5")
                .arg(g_str_param_in_cfg_file_error, gs_ini_key_bit_per_px)
                .arg(gs_min_bit_per_px).arg(g_max_bit_per_px).arg(g_str_plz_check);
    }

    settings.endGroup();
    /*--------------------*/
    if(ret_str_ptr) *ret_str_ptr = ret_str;
    return ret;

#undef CHECK_ENMU
}
