#ifndef SYSCONFIGS_H
#define SYSCONFIGS_H

#include <QString>
#include "common_tools/common_tool_func.h"

//__ARGS__ should be empty or as form "=5"
#define ENUM_NAME_DEF(e, ...) e __VA_ARGS__,
#define MB_TUBE_CURRENT_UNIT_E \
    ENUM_NAME_DEF(MB_TUBE_CURRENT_UNIT_UA)\
    ENUM_NAME_DEF(MB_TUBE_CURRENT_UNIT_MA)\
    ENUM_NAME_DEF(MB_TUBE_CURRENT_UNIT_A)
typedef enum
{
    MB_TUBE_CURRENT_UNIT_E
}mb_tube_current_unit_e_t;

#define MB_DURA_UNIT_E \
    ENUM_NAME_DEF(MB_DURA_UNIT_MS) \
    ENUM_NAME_DEF(MB_DURA_UNIT_SEC) \
    ENUM_NAME_DEF(MB_DURA_UNIT_MIN)
typedef enum
{
    MB_DURA_UNIT_E
}mb_dura_unit_e_t;

#define UI_DISP_TUBE_OR_OILBOX_E \
    ENUM_NAME_DEF(UI_DISP_TUBE_NO) \
    ENUM_NAME_DEF(UI_DISP_OILBOX_NO)
typedef enum
{
    UI_DISP_TUBE_OR_OILBOX_E
}ui_disp_tube_or_oilbox_str_e_t;

#define HV_EXPO_S_AND_E_MODE_E \
    ENUM_NAME_DEF(HV_EXPO_S_AND_E_MODE_SET_TRIPLE) \
    ENUM_NAME_DEF(HV_EXPO_S_AND_E_MODE_SET_OP_REG)
typedef enum
{
    HV_EXPO_S_AND_E_MODE_E
}hv_expo_s_and_e_mode_e_t;

typedef struct
{
    QString com_port_s;
    int boudrate, databits, parity, stopbits;
}serial_port_params_struct_t;

typedef struct
{
    bool start_gpio_monitor_th;
    int left_btn_gpio, right_btn_gpio;
    int light_ctrl_gpio;
    int btn_press_val, btn_release_val;
    int dbl_click_btn_int_ms;
    int light_on_val, light_off_val;
    int btn_smooth_dur_ms;
    int gpio_btn_scan_period_ms;
}btn_gpio_cfg_blk_struct_t;

typedef struct
{
    int log_level;

    int bit_per_px;
    /*----------*/
    double cool_dura_factor;
    int extra_cool_time_ms;
    int expo_prepare_time_ms, consec_rw_wait_ms;

    int tube_volt_kv_min;
    int tube_volt_kv_max;
    double tube_current_ma_min;
    double tube_current_ma_max;
    double dura_ms_min;
    double dura_ms_max;

    double coil_current_a_min, coil_current_a_max;

    int mb_reconnect_wait_ms, mb_err_retry_wait_ms;
    int test_time_stat_grain_sec;
    int mb_one_cmd_round_time_ms;

    mb_tube_current_unit_e_t mb_tube_current_intf_unit, ui_current_unit;
    mb_dura_unit_e_t mb_dura_intf_unit, ui_mb_dura_unit;
    double sw_to_dev_extra_factor_volt, sw_to_dev_extra_factor_current, sw_to_dev_extra_factor_dura;

    int test_proc_monitor_period_ms;

    int distance_group_disp, sw_ver_disp, hw_ver_disp, hv_ctrl_board_no_disp;
    ui_disp_tube_or_oilbox_str_e_t tube_or_oilbox_no_disp;
    int test_params_settings_disp, pause_test_disp;

    int max_pt_number, all_bytes_per_pt, pkt_idx_byte_cnt;

    int expo_to_coll_max_allowed_delay_ms, expo_to_coll_min_allowed_delay_ms;
    int pb_monitor_period_ms, pb_self_chk_to_ms;
    bool pb_monitor_log;

    int scrn_w, scrn_h;

    int scan_dura_allowed_min_sec, scan_dura_allowed_max_sec;
    int conn_data_src_tmo_allowed_min_sec, conn_data_src_tmo_allowed_max_sec;

    int ini_disp_img_line_cnt_allowed_min, ini_disp_img_line_cnt_allowed_max;
    int merg_disp_img_line_cnt_allowed_min, merg_disp_img_line_cnt_allowed_max;
    int cali_bg_line_cnt_allowed_min, cali_bg_line_cnt_allowed_max;
    int cali_stre_factor_line_cnt_allowed_min, cali_stre_factor_line_cnt_allowed_max;

    bool limit_recvd_line_number;
    int recvd_line_number_limit_allowed_min, recvd_line_number_limit_allowed_max;

    int data_src_port;

    bool enable_self_check;
    bool skip_pwr_self_chk, skip_x_src_self_chk, skip_detector_self_chk, skip_storage_self_chk;
    bool enable_pb_monitor, enable_hv_monitor, enable_hv_auto_reconn;

    bool disable_monitor_during_scan;

    gray_pixel_data_type def_scan_bg_value;
    double def_scan_stre_factor_value;

    hv_expo_s_and_e_mode_e_t hv_expo_s_and_e_mode;

    bool scan_without_x;
    int hv_monitor_period_ms;

    btn_gpio_cfg_blk_struct_t btn_gpio_cfg;
}sys_configs_struct_t;

extern sys_configs_struct_t g_sys_configs_block;

bool fill_sys_configs(QString *);

#endif // SYSCONFIGS_H
