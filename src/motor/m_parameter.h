#ifndef __M_PARAMETER_H__
#define __M_PARAMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_data.h"

typedef union
{
	uint32_t u32;
	struct
	{
		uint16_t low;
		uint16_t high;
	}words;
}union_u32;


typedef union
{
	int32_t s32;
	struct
	{
		int16_t low;
		int16_t high;
	}words;
}union_s32;


#define		M_PI		3.14159265358f

#define     ANGLE_0     0
#define     ANGLE_30    5461
#define     ANGLE_60    10922
#define     ANGLE_90    16384
#define     ANGLE_120   21845
#define     ANGLE_150   27306
#define     ANGLE_180   32768
#define     ANGLE_210   38229
#define     ANGLE_240   43690
#define     ANGLE_270   49151
#define     ANGLE_300   54613
#define     ANGLE_330   60074
#define     ANGLE_360   0

// 电机旋转方向
#define 	CW			0
#define		CCW			1


// Timer Clock Frequency
#define GPT_CLK									(120000000.0f)
// PWM Frequency
#define PWM_FREQUENCY							(20000.0f)
// Tpwm
#define T_PWM									(float)(1.0f / (float)PWM_FREQUENCY)
// PWM Period Count
#define PWM_PERIOD_VALUE						(uint32_t)(GPT_CLK / PWM_FREQUENCY)   // 6000
// Timer Reload Value	
#define GPT_RELOAD_VALUE						(PWM_PERIOD_VALUE >> 1)		// 3000


// 电机极对数
#define 	POLAR								2


// 60°电角度时间计算
// scale表示计数一次的时间是多少us
#define HALL_TIME_CALCULATE(speed, scale) 		(uint32_t)(60000000.0f / (6.0f * (float)speed * 2.0f * (float)scale))
// 两对极
#define MAX_SPEED_HALL_TIME_VALUE					HALL_TIME_CALCULATE(3000, 1)
#define MIN_SPEED_HALL_TIME_VALUE					HALL_TIME_CALCULATE(50, 1)


// 滤波函数
#define LPF_CALC(X_in, Y_out)						(X_in >> 2) + (Y_out >> 1) + (Y_out >> 2)

// Q轴启动电流
#define IQ_START_VALUE								2048



/**************************************与计数周期相关的参数*****************************************/

// Us模长更新时间
#define SPD_TICK_CNT								10			// 10 * 50us  => 0.5ms
// 自举电容预充电时间
#define BOOST_CHARGING_TICK_CNT						200			// 200 * 50us => 10ms
// 霍尔值未更新时间阈值
#define HALL_VALUE_UPDATE_THRESHOLD_TIME			(uint16_t)(1000.0f * 20.0f)
// 差分运放静态误差采样时间
#define CURRENT_STATIC_ERROR_TICK_CNT				1000 * 20 	// 20 * 1000 * 50us => 20 * 50ms => 1000ms => 1s
// 打印三相电流采样的时间间隔
#define TEST_CURRENT_TICK_CNT						1000 * 10 	// 10 * 1000 * 50us => 500ms
// 速度环目标转速更新时间
#define SPEED_SET_VALUE_TICK_CNT					20 * 20	// 20 * 20 * 50us => 20ms


/**************************************限幅相关参数*****************************************/

// 占空比限幅
#define DEAD_TIME									240		// 240/(120 * 10^6) => 2us
// 这里的最大最小占空比是相反的概念，设置的最小占空比其实是最大占空比，但其实逻辑是互斥的，也没多大关系
#define MIN_DUTY_VALUE								DEAD_TIME
#define MAX_DUTY_VALUE								(GPT_RELOAD_VALUE - (DEAD_TIME >> 1) - DEAD_TIME)

// 电位器ADC采集限幅值  （这个限幅值只是为了电位器旋转到一定位置时，电机停机）
#define MIN_US_TARGET_VALUE							655
#define MAX_US_TARGET_VALUE							58982

// 转速限幅值（单位：rad/min）
#define MIN_SPEED_VALUE								1000
#define MAX_SPEED_VALUE								2600

// 速度环输出的Q轴电流限幅值
#define IQ_MIN_VALUE								50

// Uq限幅值所对应的Us的Q15限幅值
#define Q15_US_MAX_VALUE							29491	// 32768 * 0.9	即90%的最大值限幅


// 浮点数Q16格式
// 2 / 3
#define _2DIV3										43691
#define SQRT3DIV2									56756
#define SQRT3DIV3									37837


#define DR_DIFF_CACULATE(us)					((uint32_t)(50.0f * 60.0f * 65536.0f / ((float)us * 360.0f)))
#define DR_DIFF_VALUE							DR_DIFF_CACULATE(1)	


/**********************************************************************无感部分宏定义参数******************************************************************/

/**************************************与计数周期相关的参数*****************************************/
// 角度变化量计算的周期计数
#define	DELTA_ANGLE_CNT								20	
// T_ANGLE
#define	T_ANGLE										(float)(DELTA_ANGLE_CNT * T_PWM)	// 50us * 20 -> 1ms	
// 1个角速度计算周期的时间的倒数
#define DELTA_ANGLE_TIME_INVERSE					(uint16_t)(1.0f / (float)T_ANGLE)


/**************************************与delta_theta_e低通滤波相关的参数*****************************************/
// delta_theta_e 低通滤波的截止频率
// 300RPM所对应的截止频率
#define DELTA_ANGLE_FILTER_FC						(float)(300.0f / 60.0f * (float)POLAR)
// delta_theta_e 低通滤波的滤波系数 2π * Tpwm * fc
#define DELTA_ANGLE_FILTER_K						(int16_t)((2.0f * M_PI * T_PWM * DELTA_ANGLE_FILTER_FC) * 32768.0f)
// 考虑是否对于delta_theta的低通滤波的滤波系数计算应该采用角速度计算周期
#define DELTA_ANGLE_FILTER_K_1						(int16_t)((2.0f * M_PI * (T_PWM * (float)DELTA_ANGLE_CNT) * DELTA_ANGLE_FILTER_FC) * 32768.0f)

/**************************************与反电动势一阶低通滤波相关的参数*****************************************/
// 一阶低通滤波器的自适应滤波系数的辅助系数((π * Tpwm) / δT) * 65536
#define Q16_LPF_SUB_K								(uint16_t)(((float)(M_PI * T_PWM) / T_ANGLE) * 65536.0f)
// 最低转速300RPM所对应的δθ
/*
	RPM = 300 => RPS = 300 / 60 = 5;
	eRPS = fc = RPS * Polar = 10;
	所以 fc = 10;
	
	fc = (δθ/360) / δT，其中fc=10
	fc * δT = (δθ/ 180) / 2
	fc * δT * 2 = (δθ/ 180) = 10 * 0.001 * 2 = 0.02
	(δθ/ 180) = 0.02
	转换到q15就是0.02 * 32768 = 655
	
	需要注意的是，我们为什么要凑出(δθ/ 180) ，因为我们通过反正切计算出的角度，归一化的基准就是180°（π）
	还是那句话，q格式数据就是一个归一化的数字，0.02表示什么呢，表示的就是0.02个半圈的角度值
*/
#define MIN_FC_DELTA_THETA							(int16_t)(DELTA_ANGLE_FILTER_FC * T_ANGLE * 2.0f * 32768.0f)


/**************************************与电流估算方程相关的参数*****************************************/
// 电机线电阻
#define LINE_RESISTANCE_U							13.2f		// Ω
#define LINE_RESISTANCE_V							13.2f		// Ω
#define LINE_RESISTANCE_W							13.3f		// Ω
// 电机相电阻
#define PHASE_RESISTANCE							(float)((LINE_RESISTANCE_U + LINE_RESISTANCE_V + LINE_RESISTANCE_W) / 3.0f / 2.0f)	// Ω

// 电机线电感
#define LINE_INDUCTANCE_U							12.011f * 1000.0f	// uH
#define LINE_INDUCTANCE_V							13.352f * 1000.0f	// uH
#define LINE_INDUCTANCE_W							12.850f * 1000.0f	// uH
// 电机相电感
#define	PHASE_INDUCTANCE							(float)((LINE_INDUCTANCE_U + LINE_INDUCTANCE_V + LINE_INDUCTANCE_W) / 3.0f / 2.0f)	// uH

// 电流估计方程的F值
// F_COEFF = 1 - R * Tpwm / L
// T_PWM * 1000000 是为了将0.00005s 转换成 50us
// Ω * us / uH => Ω * (1 / Ω) => 无量纲，就是个系数，最后乘以32768变成q15格式数据
#define F_COEFF										(int16_t)((1.0f - PHASE_RESISTANCE * T_PWM * 1000000.0f / PHASE_INDUCTANCE) * 32768.0f)

// G_COEFF = Tpwm / L
#define G_COEFF										(int16_t)((T_PWM * 1000000.0f / PHASE_INDUCTANCE) * 32768.0f)

/**************************************与滑膜观测器相关的参数*****************************************/
// 滑膜观测器SAT函数的边界层（最大误差值）
#define OBS_Z_MAX									170
// 滑膜观测器增益
#define OBS_K_GAIN									30000


/**************************************与转子预定位相关的参数*****************************************/
// 第一次预定位的Us模长大小	
#define ROTOR_PREPOS_US1							19660
#define ROTOR_PREPOS1_TICK_CNT						10000	// 50us * 10000 => 500ms

// 第二次预定位的Us模长大小	
#define ROTOR_PREPOS_US2							19660
#define ROTOR_PREPOS2_TICK_CNT						10000	// 50us * 10000 => 500ms


/**************************************与开环强拖加速相关的参数*****************************************/
// 角加速度步进值
#define DRAG_ACC_STEP								5
// 开环强拖周期计数值
#define DRAG_TICK_CNT								10
// 开环强拖周期
#define T_DRAG										(float)(T_PWM * (float)(DRAG_TICK_CNT))	// 500us
// 切换到闭环滑膜控制的机械转速
#define DRAG_MECH_SPEED								500

// 角速度的最大值限幅
/*
	我们开环强拖的角速度，也就是每个PWM周期递增的角度值不能无限大，出现了明显的反电动势特征就应该切换到
	闭环滑膜控制，因此需要设置角度递增的最大值，通常来讲电机旋转至500RPM时即可由开环切换到闭环
	因此我们需要计算500RPM机械转速对应的电转速下的一个开环强拖周期(500us)中的q16_delta_theta
	eRPM = POLAR * RPM
	eRPS = eRPM / 60
	delta_theta = eRPS * 360 * 0.0005
	q16_delta_theta = (delta_theta / 360) * 65536 = 546
	
*/
#define Q16_DRAG_W_MAX_VALUE						(uint16_t)(((((float)DRAG_MECH_SPEED * (float)POLAR / 60.0f) * 360.0f * T_DRAG) / 360.0f) * 65536.0f)
// 开环切换到闭环的角度差值满足条件的计数阈值
#define OPEN_TO_CLOSE_CNT_THRESHOLD					3
// 启动电流Iq衰减周期计数值
#define IQ_CALIB_TICK_CNT							100		// 50us * 100 => 5ms


/**************************************与速度环相关的参数*****************************************/
// 速度环执行周期计数变量
#define SPEED_LOOP_TICK_CNT							200		// 50us * 200 => 10ms



#ifdef _cplusplus
}
#endif

#endif