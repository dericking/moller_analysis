#!/bin/bash

SETFILE="/w/halla-scifs17exp/moller/disk1/a-molana/moller_data/mollerrun_${1}.set"

echo "Looking for settings file ${SETFILE}..."

ID_SET_RUN=$(sed -n -e "s#\(Run Number : \)##p" $SETFILE)
SET_RUN_TYPE=$(sed -n -e "s#\(Run Type   : \)##p" $SETFILE)

SET_RUN_START=$(sed -n -e "s#\(Date       : \)##p" $SETFILE)
SET_RUN_END=$(sed -n -e "s#\(End Run Time : \)##p" $SETFILE)

  #CONVERT TO FORMAT THAT SQL LIKES
  read dayname month datnum clock zone year <<< ${SET_RUN_START}
  SET_RUN_START=$(date --date="$(printf "${SET_RUN_START}")" +"%Y-%m-%d %H:%M:%S")
  read dayname month datnum clock zone year <<< ${SET_RUN_END}
  SET_RUN_END=$(date --date="$(printf "${SET_RUN_END}")" +"%Y-%m-%d %H:%M:%S")

#WE CAN CALCULATE THIS ONE
#SET_RUN_LENGTH=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
SET_RUN_LENGTH="00:02:12"

#SET_TRIG_THRESH_CH0=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
#SET_TRIG_THRESH_CH1=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
SET_TRIG_THRESH_CH0=0
SET_TRIG_THRESH_CH1=1

SET_IHWP_IN=$(sed -n -e "s#\(Laser 1/2 wave plate                 IGL1I00OD16_16       : \)##p" $SETFILE)

#SET_TARGET=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
SET_TARGET=2

SET_E_BEAM=$(sed -n -e "s#\( Beam energy, MeV Hall A             HALLA:p              : \)##p" $SETFILE)
SET_E_BEAM=$(printf "%.14f" $SET_E_BEAM)
SET_E_INJ=$(sed -n -e "s#\(Injector energy, MeV                 MMSINJEGAIN          : \)##p" $SETFILE)
SET_E_SLINAC=$(sed -n -e "s#\(South linac energy, MeV              MMSLIN1EGAIN         : \)##p" $SETFILE)
SET_E_NLINAC=$(sed -n -e "s#\(North linac energy, MeV              MMSLIN2EGAIN         : \)##p" $SETFILE)
SET_N_PASS=$(sed -n -e "s#\(Passes Hall A                        MMSHLAPASS           : \)##p" $SETFILE)
SET_BCM_AVG=$(sed -n -e "s#\(Beam Current Average                 hac_bcm_average      : \)##p" $SETFILE)
SET_UNSER=$(sed -n -e "s#\(Current on Unser monitor             hac_unser_read       : \)##p" $SETFILE)
SET_BCM_US=$(sed -n -e "s#\(Current on Upstream bcm              hac_bcm_dvm1_current : \)##p" $SETFILE)
SET_BCM_DS=$(sed -n -e "s#\(Current on Downstream bcm            hac_bcm_dvm2_current : \)##p" $SETFILE)
SET_INJ_BCM_TOT=$(sed -n -e "s#\(Injector Full Current Monitor 02     IBC0L02Current       : \)##p" $SETFILE)
SET_INJ_BCM_HALLA=$(sed -n -e "s#\(Injector Current Monitor Hall A      IBC1H04CRCUR2        : \)##p" $SETFILE)
SET_BPM01_X=$(sed -n -e "s#\(Beam Position BPM01  X, mm           IPM1H01.XPOS         : \)##p" $SETFILE)
SET_BPM01_Y=$(sed -n -e "s#\(Beam Position BPM01  Y, mm           IPM1H01.YPOS         : \)##p" $SETFILE)
SET_BPM04_X=$(sed -n -e "s#\(Beam Position BPM04  X, mm           IPM1H04.XPOS         : \)##p" $SETFILE)
SET_BPM04_Y=$(sed -n -e "s#\(Beam Position BPM04  Y, mm           IPM1H04.YPOS         : \)##p" $SETFILE)
SET_BPM04A_X=$(sed -n -e "s#\(Beam Position BPM04A  X, mm          IPM1H04A.XPOS        : \)##p" $SETFILE)
SET_BPM04A_Y=$(sed -n -e "s#\(Beam Position BPM04A  Y, mm          IPM1H04A.YPOS        : \)##p" $SETFILE)
SET_Q1_CUR=$(sed -n -e "s#\(Quad Q1 (Amps)                       MQO1H02M             : \)##p" $SETFILE)
SET_Q2_CUR=$(sed -n -e "s#\(Quad Q2 (Amps)                       MQM1H02M             : \)##p" $SETFILE)
SET_Q3_CUR=$(sed -n -e "s#\(Quad Q3 (Amps)                       MQO1H03M             : \)##p" $SETFILE)
SET_Q4_CUR=$(sed -n -e "s#\(Quad Q4 (Amps)                       MQO1H03AM            : \)##p" $SETFILE)
SET_DIP_CUR=$(sed -n -e "s#\(Dipole  (Amps)                       MMA1H01M             : \)##p" $SETFILE)
SET_TGT_ANGLE=$(sed -n -e "s#\(Target Rotary Position(V)            HAHFMROTENC          : \)##p" $SETFILE)
SET_TGT_ANGLE_DEG=$(sed -n -e "s#\(Rotary Position in deg(from controll HAHFMROTSM.RBV       : \)##p" $SETFILE)
SET_TGT_LIN_POS=$(sed -n -e "s#\(Target Linear Position(V)            HAHFMLINENC          : \)##p" $SETFILE)
SET_TGT_LIN_POS_MM=$(sed -n -e "s#\(Linear Position in mm                HAHFMLINSM.RBV       : \)##p" $SETFILE)
SET_LAS_MODE_HALLA=$(sed -n -e "s#\(Laser mode  Hall A                   IGL1I00HALLAMODE     : \)##p" $SETFILE)
SET_LAS_MODE_HALLB=$(sed -n -e "s#\(Laser mode  Hall B                   IGL1I00HALLBMODE     : \)##p" $SETFILE)
SET_LAS_MODE_HALLC=$(sed -n -e "s#\(Laser mode  Hall C                   IGL1I00HALLCMODE     : \)##p" $SETFILE)
SET_LAS_POW_HALLA=$(sed -n -e "s#\(Laser power Hall A                   IGL1I00DAC0          : \)##p" $SETFILE)
SET_LAS_POW_HALLB=$(sed -n -e "s#\(Laser power Hall B                   IGL1I00DAC2          : \)##p" $SETFILE)
SET_LAS_POW_HALLC=$(sed -n -e "s#\(Laser power Hall C                   IGL1I00DAC4          : \)##p" $SETFILE)
SET_LAS_ATTN_HALLA=$(sed -n -e "s#\(Laser attenuation Hall A             psub_aa_pos          : \)##p" $SETFILE)
SET_LAS_ATTN_HALLB=$(sed -n -e "s#\(Laser attenuation Hall B             psub_ab_pos          : \)##p" $SETFILE)
SET_LAS_ATTN_HALLC=$(sed -n -e "s#\(Laser attenuation Hall C             psub_ac_pos          : \)##p" $SETFILE)
SET_SLIT_HALLA=$(sed -n -e "s#\(Slit Position Hall A                 SMRPOSA              : \)##p" $SETFILE)
SET_SLIT_HALLB=$(sed -n -e "s#\(Slit Position Hall B                 SMRPOSB              : \)##p" $SETFILE)
SET_SLIT_HALLC=$(sed -n -e "s#\(Slit Position Hall C                 SMRPOSC              : \)##p" $SETFILE)
SET_IHWP=$(sed -n -e "s#\(Laser 1/2 wave plate                 IGL1I00OD16_16       : \)##p" $SETFILE)
SET_RHWP=$(sed -n -e "s#\(Rotating 1/2 wave plate              psub_pl_pos          : \)##p" $SETFILE)
SET_VWIEN_ANGLE=$(sed -n -e "s#\(VWien filter angle, deg              VWienAngle           : \)##p" $SETFILE)
SET_SOL_PHI_FG=$(sed -n -e "s#\(Solenoids angle, deg                 Phi_FG               : \)##p" $SETFILE)
SET_HWIEN_ANGLE=$(sed -n -e "s#\(HWien filter angle, deg              HWienAngle           : \)##p" $SETFILE)
SET_HEL_PATTERN=$(sed -n -e "s#\(Helicity Mode ON/OFF Random/Toggle   HELPATTERNd          : \)##p" $SETFILE)
SET_HEL_FREQ=$(sed -n -e "s#\(Helicity frequency                   HELFREQ              : \)##p" $SETFILE)
SET_HEL_DELAY=$(sed -n -e "s#\(Helicity delay                       HELDELAYd            : \)##p" $SETFILE)
SET_T_SETTLE=$(sed -n -e "s#\(MPS signal, usec                     HELTSETTLEd          : \)##p" $SETFILE)
SET_T_STABLE=$(sed -n -e "s#\(Helicity window, usec                HELTSTABLEd          : \)##p" $SETFILE)
SET_BPM02A_X=$(sed -n -e "s#\(Beam Position BPM02A X, mm           IPM1P02A.XPOS        : \)##p" $SETFILE)
SET_BPM02A_Y=$(sed -n -e "s#\(Beam Position BPM02A Y, mm           IPM1P02A.YPOS        : \)##p" $SETFILE)
SET_MOL_MAG_CUR_SET=$(sed -n -e "s#\(   AM430 Current Setpoint (A)        hamolpol:am430:targe : \)##p" $SETFILE)
SET_MOL_MAG_CUR_MEAS=$(sed -n -e "s#\(   AM430 Measured Current (A)        hamolpol:am430:magCu : \)##p" $SETFILE)
SET_MOL_MAG_V_MEAS=$(sed -n -e "s#\(   AM430 Measured Voltage (A)        hamolpol:am430:magVo : \)##p" $SETFILE)
SET_MOL_MAG_FIELD_MEAS=$(sed -n -e "s#\(   AM430 Measured Field (T)          hamolpol:am430:magFi : \)##p" $SETFILE)
SET_MOL_MAG_RAMP_STATE=$(sed -n -e "s#\(   AMS430 Ramp State                 hamolpol:am430:rampS : \)##p" $SETFILE)
SET_MOL_COOLER_TEMP=$(sed -n -e "s#\(   Cryocooler Temperature (K)        hamolpol:lk218_1:tem : \)##p" $SETFILE)
SET_MOL_MAG_T2TEMP=$(sed -n -e "s#\(   Magnet(T2) Temperature(K)         hamolpol:lk218_1:tem : \)##p" $SETFILE)
SET_MOL_MAG_LEAD1_TEMP=$(sed -n -e "s#\(   Magnet Lead \#1 (T6) Temperature(K hamolpol:lk218_1:tem : \)##p" $SETFILE)
SET_MOL_MAG_LEAD2_TEMP=$(sed -n -e "s#\(   Magnet Lead \#2 (T7) Temperature(K hamolpol:lk218_1:tem : \)##p" $SETFILE)
SET_DET_HV_CH1=$(sed -n -e "s#\(Moller Detector measured HV ch 1     hatsv5.2003.S5.0     : \)##p" $SETFILE)
SET_DET_HV_CH2=$(sed -n -e "s#\(Moller Detector measured HV ch 2     hatsv5.2003.S5.1     : \)##p" $SETFILE)
SET_DET_HV_CH3=$(sed -n -e "s#\(Moller Detector measured HV ch 3     hatsv5.2003.S5.2     : \)##p" $SETFILE)
SET_DET_HV_CH4=$(sed -n -e "s#\(Moller Detector measured HV ch 4     hatsv5.2003.S5.3     : \)##p" $SETFILE)
SET_DET_HV_CH5=$(sed -n -e "s#\(Moller Detector measured HV ch 5     hatsv5.2003.S5.4     : \)##p" $SETFILE)
SET_DET_HV_CH6=$(sed -n -e "s#\(Moller Detector measured HV ch 6     hatsv5.2003.S5.5     : \)##p" $SETFILE)
SET_DET_HV_CH7=$(sed -n -e "s#\(Moller Detector measured HV ch 7     hatsv5.2003.S5.6     : \)##p" $SETFILE)
SET_DET_HV_CH8=$(sed -n -e "s#\(Moller Detector measured HV ch 8     hatsv5.2003.S5.7     : \)##p" $SETFILE)
SET_DET_AP_CH1=$(sed -n -e "s#\(Moller Detector measured HV Ap 1     hatsv5.2003.S6.0     : \)##p" $SETFILE)
SET_DET_AP_CH2=$(sed -n -e "s#\(Moller Detector measured HV Ap 2     hatsv5.2003.S6.1     : \)##p" $SETFILE)
SET_DET_AP_CH3=$(sed -n -e "s#\(Moller Detector measured HV Ap 3     hatsv5.2003.S6.2     : \)##p" $SETFILE)
SET_DET_AP_CH4=$(sed -n -e "s#\(Moller Detector measured HV Ap 4     hatsv5.2003.S6.3     : \)##p" $SETFILE)
SET_DET_AP_CH5=$(sed -n -e "s#\(Moller Detector measured HV Ap 5     hatsv5.2003.S6.4     : \)##p" $SETFILE)
SET_DET_AP_CH6=$(sed -n -e "s#\(Moller Detector measured HV Ap 6     hatsv5.2003.S6.5     : \)##p" $SETFILE)
SET_DET_AP_CH7=$(sed -n -e "s#\(Moller Detector measured HV Ap 7     hatsv5.2003.S6.6     : \)##p" $SETFILE)
SET_DET_AP_CH8=$(sed -n -e "s#\(Moller Detector measured HV Ap 8     hatsv5.2003.S6.7     : \)##p" $SETFILE)

#SET_TRIG_THRESH=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
#SET_TRIG_TYPE=$(sed -n -e "s#\(REPLACEME\)##p" $SETFILE)
SET_TRIG_THRESH=0
SET_TRIG_TYPE=0000

SET_MOL_POW_SUP_CUR=$(sed -n -e "s#\(Hcoils current (Amps)                                     : \)##p" $SETFILE)

DB_COMMAND="REPLACE INTO moller_settings \
             (id_set_run,\
              set_run_type,\
              set_run_start,\
              set_run_end,\
              set_run_length,\
              set_trig_thresh_ch0,\
              set_trig_thresh_ch1,\
              set_ihwp_in,\
              set_target,\
              set_E_beam,\
              set_E_inj,\
              set_E_Slinac,\
              set_E_Nlinac,\
              set_n_pass,\
              set_bcm_avg,\
              set_unser,\
              set_bcm_us,\
              set_bcm_ds,\
              set_inj_bcm_tot,\
              set_inj_bcm_halla,\
              set_bpm01_X,\
              set_bpm01_Y,\
              set_bpm04_X,\
              set_bpm04_Y,\
              set_bpm04a_X,\
              set_bpm04a_Y,\
              set_q1_cur,\
              set_q2_cur,\
              set_q3_cur,\
              set_q4_cur,\
              set_dip_cur,\
              set_tgt_angle,\
              set_tgt_angle_deg,\
              set_tgt_lin_pos,\
              set_tgt_lin_pos_mm,\
              set_las_mode_halla,\
              set_las_mode_hallb,\
              set_las_mode_hallc,\
              set_las_pow_halla,\
              set_las_pow_hallb,\
              set_las_pow_hallc,\
              set_las_attn_halla,\
              set_las_attn_hallb,\
              set_las_attn_hallc,\
              set_slit_halla,\
              set_slit_hallb,\
              set_slit_hallc,\
              set_ihwp,\
              set_rhwp,\
              set_vwien_angle,\
              set_sol_phi_fg,\
              set_hwien_angle,\
              set_hel_pattern,\
              set_hel_freq,\
              set_hel_delay,\
              set_t_settle,\
              set_t_stable,\
              set_bpm02a_X,\
              set_bpm02a_Y,\
              set_mol_mag_cur_set,\
              set_mol_mag_cur_meas,\
              set_mol_mag_v_meas,\
              set_mol_mag_field_meas,\
              set_mol_mag_ramp_state,\
              set_mol_cooler_temp,\
              set_mol_mag_T2temp,\
              set_mol_mag_lead1_temp,\
              set_mol_mag_lead2_temp,\
              set_det_hv_ch1,\
              set_det_hv_ch2,\
              set_det_hv_ch3,\
              set_det_hv_ch4,\
              set_det_hv_ch5,\
              set_det_hv_ch6,\
              set_det_hv_ch7,\
              set_det_hv_ch8,\
              set_det_ap_ch1,\
              set_det_ap_ch2,\
              set_det_ap_ch3,\
              set_det_ap_ch4,\
              set_det_ap_ch5,\
              set_det_ap_ch6,\
              set_det_ap_ch7,\
              set_det_ap_ch8,\
              set_trig_thresh,\
              set_trig_type,\
              set_mol_pow_sup_cur) \
              VALUES \
             (${ID_SET_RUN},\
             \"$SET_RUN_TYPE\",\
             \"$SET_RUN_START\",\
             \"$SET_RUN_END\",\
             \"$SET_RUN_LENGTH\",\
             $SET_TRIG_THRESH_CH0,\
             $SET_TRIG_THRESH_CH1,\
             \"$SET_IHWP_IN\",\
             $SET_TARGET,\
             $SET_E_BEAM,\
             $SET_E_INJ,\
             $SET_E_SLINAC,\
             $SET_E_NLINAC,\
             \"$SET_N_PASS\",\
             $SET_BCM_AVG,\
             $SET_UNSER,\
             $SET_BCM_US,\
             $SET_BCM_DS,\
             $SET_INJ_BCM_TOT,\
             $SET_INJ_BCM_HALLA,\
             $SET_BPM01_X,\
             $SET_BPM01_Y,\
             $SET_BPM04_X,\
             $SET_BPM04_Y,\
             $SET_BPM04A_X,\
             $SET_BPM04A_Y,\
             $SET_Q1_CUR,\
             $SET_Q2_CUR,\
             $SET_Q3_CUR,\
             $SET_Q4_CUR,\
             $SET_DIP_CUR,\
             $SET_TGT_ANGLE,\
             $SET_TGT_ANGLE_DEG,\
             $SET_TGT_LIN_POS,\
             $SET_TGT_LIN_POS_MM,\
             \"$SET_LAS_MODE_HALLA\",\
             \"$SET_LAS_MODE_HALLB\",\
             \"$SET_LAS_MODE_HALLC\",\
             $SET_LAS_POW_HALLA,\
             $SET_LAS_POW_HALLB,\
             $SET_LAS_POW_HALLC,\
             $SET_LAS_ATTN_HALLA,\
             $SET_LAS_ATTN_HALLB,\
             $SET_LAS_ATTN_HALLC,\
             $SET_SLIT_HALLA,\
             $SET_SLIT_HALLB,\
             $SET_SLIT_HALLC,\
             \"$SET_IHWP\",\
             $SET_RHWP,\
             $SET_VWIEN_ANGLE,\
             $SET_SOL_PHI_FG,\
             $SET_HWIEN_ANGLE,\
             \"$SET_HEL_PATTERN\",\
             $SET_HEL_FREQ,\
             \"$SET_HEL_DELAY\",\
             $SET_T_SETTLE,\
             $SET_T_STABLE,\
             $SET_BPM02A_X,\
             $SET_BPM02A_Y,\
             $SET_MOL_MAG_CUR_SET,\
             $SET_MOL_MAG_CUR_MEAS,\
             $SET_MOL_MAG_V_MEAS,\
             $SET_MOL_MAG_FIELD_MEAS,\
             $SET_MOL_MAG_RAMP_STATE,\
             $SET_MOL_COOLER_TEMP,\
             $SET_MOL_MAG_T2TEMP,\
             $SET_MOL_MAG_LEAD1_TEMP,\
             $SET_MOL_MAG_LEAD2_TEMP,\
             $SET_DET_HV_CH1,\
             $SET_DET_HV_CH2,\
             $SET_DET_HV_CH3,\
             $SET_DET_HV_CH4,\
             $SET_DET_HV_CH5,\
             $SET_DET_HV_CH6,\
             $SET_DET_HV_CH7,\
             $SET_DET_HV_CH8,\
             $SET_DET_AP_CH1,\
             $SET_DET_AP_CH2,\
             $SET_DET_AP_CH3,\
             $SET_DET_AP_CH4,\
             $SET_DET_AP_CH5,\
             $SET_DET_AP_CH6,\
             $SET_DET_AP_CH7,\
             $SET_DET_AP_CH8,\
             $SET_TRIG_THRESH,\
             \"$SET_TRIG_TYPE\",\
             $SET_MOL_POW_SUP_CUR)"

echo ${DB_COMMAND}

mysql -h ${MOLANA_DB_HOST} --user="${MOLANA_DB_USER}" --password="${MOLANA_DB_PASS}" --database="${MOLANA_DB_NAME}" -e "${DB_COMMAND}"

