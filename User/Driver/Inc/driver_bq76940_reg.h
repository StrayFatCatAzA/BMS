#ifndef __DRIVER_BQ76940_REG_H__
#define __DRIVER_BQ76940_REG_H__

/* =========================== 寄存器地址定义 =========================== */

#define BQ76940_SYS_STAT 0x00
#define BQ76940_CELLBAL1 0x01
#define BQ76940_CELLBAL2 0x02
#define BQ76940_CELLBAL3 0x03
#define BQ76940_SYS_CTRL1 0x04
#define BQ76940_SYS_CTRL2 0x05
#define BQ76940_PROTECT1 0x06
#define BQ76940_PROTECT2 0x07
#define BQ76940_PROTECT3 0x08
#define BQ76940_OV_TRIP 0x09
#define BQ76940_UV_TRIP 0x0A
#define BQ76940_CC_CFG 0x0B

/* =========================== 电池电压寄存器 =========================== */

#define BQ76940_VC1_HI 0x0C
#define BQ76940_VC1_LO 0x0D
#define BQ76940_VC2_HI 0x0E
#define BQ76940_VC2_LO 0x0F
#define BQ76940_VC3_HI 0x10
#define BQ76940_VC3_LO 0x11
#define BQ76940_VC4_HI 0x12
#define BQ76940_VC4_LO 0x13
#define BQ76940_VC5_HI 0x14
#define BQ76940_VC5_LO 0x15
#define BQ76940_VC6_HI 0x16
#define BQ76940_VC6_LO 0x17
#define BQ76940_VC7_HI 0x18
#define BQ76940_VC7_LO 0x19
#define BQ76940_VC8_HI 0x1A
#define BQ76940_VC8_LO 0x1B
#define BQ76940_VC9_HI 0x1C
#define BQ76940_VC9_LO 0x1D
#define BQ76940_VC10_HI 0x1E
#define BQ76940_VC10_LO 0x1F
#define BQ76940_VC11_HI 0x20
#define BQ76940_VC11_LO 0x21
#define BQ76940_VC12_HI 0x22
#define BQ76940_VC12_LO 0x23
#define BQ76940_VC13_HI 0x24
#define BQ76940_VC13_LO 0x25
#define BQ76940_VC14_HI 0x26
#define BQ76940_VC14_LO 0x27
#define BQ76940_VC15_HI 0x28
#define BQ76940_VC15_LO 0x29
/* =========================== 总电压、温度、电流寄存器 =========================== */

#define BQ76940_BAT_HI 0x2A
#define BQ76940_BAT_LO 0x2B
#define BQ76940_TS1_HI 0x2C
#define BQ76940_TS1_LO 0x2D
#define BQ76940_TS2_HI 0x2E
#define BQ76940_TS2_LO 0x2F
#define BQ76940_TS3_HI 0x30
#define BQ76940_TS3_LO 0x31
#define BQ76940_CC_HI 0x32
#define BQ76940_CC_LO 0x33

/* =========================== 校准寄存器 =========================== */

#define BQ76940_ADCGAIN1 0x50
#define BQ76940_ADCOFFSET 0x51
#define BQ76940_ADCGAIN2 0x59


/* =========================== SYS_CTRL1 位掩码=========================== */

#define BQ76940_ADC_EN_MASK   0x02
#define BQ76940_TEMP_SEL_MASK 0x04

/* =========================== CC_CFG 位掩码=========================== */

#define BQ76940_CC_EN_MASK    0x10

#endif
