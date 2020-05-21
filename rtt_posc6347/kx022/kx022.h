//created by yangwensen@20200519

#ifndef DA270_H__
#define DA270_H__
/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************/
/* header files */

#include "kx022_defs.h"
/*********************************************************************/
/* (extern) variable declarations */
/*********************************************************************/
/* function prototype declarations */
/*!
 * @brief This API is the entry point, Call this API before using other APIs.
 * This API reads the chip-id of the sensor which is the first step to
 * verify the sensor and also it configures the read mechanism of SPI and
 * I2C interface.
 *
 * @param[in,out] dev : Structure instance of kx022_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_init(struct kx022_dev *dev);

/*!
 * @brief This API writes the given data to the register address
 * of the sensor.
 *
 * @param[in] reg_addr : Register address from where the data to be written.
 * @param[in] reg_data : Pointer to data buffer which is to be written
 *                       in the reg_addr of sensor.
 * @param[in] len      : No of bytes of data to write..
 * @param[in] dev      : Structure instance of kx022_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, const struct kx022_dev *dev);

/*!
 * @brief This API reads the data from the given register address of sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be read
 * @param[out] reg_data : Pointer to data buffer to store the read data.
 * @param[in] len       : No of bytes of data to be read.
 * @param[in] dev       : Structure instance of kx022_dev.
 *
 * @note For most of the registers auto address increment applies, with the
 * exception of a few special registers, which trap the address. For e.g.,
 * Register address - 0x14(DA270_FIFO_DATA_ADDR)
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, const struct kx022_dev *dev);

int8_t kx022_set_reg_bitfield(uint8_t reg_addr, uint8_t bit_mask, uint8_t new_bitfield, const struct kx022_dev *dev);

/*!
 * @brief This API is used to perform soft-reset of the sensor
 * where all the registers are reset to their default values except 0x4B.
 *
 * @param[in] dev       : Structure instance of kx022_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_soft_reset(const struct kx022_dev *dev);

/*!
 * @brief This API is used to set the power mode of the sensor.
 *
 * @param[in] power_mode  : Macro to select power mode of the sensor.
 * @param[in] dev         : Structure instance of kx022_dev.
 *
 * Possible value for power_mode :
 *   - DA270_NORMAL_MODE
 *   - DA270_SLEEP_MODE
 *   - DA270_LOW_POWER_MODE
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_set_power_mode(uint8_t power_mode, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the power mode of the sensor
 *
 * @param[out] power_mode  : power mode of the sensor.
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * * Possible value for power_mode :
 *   - DA270_NORMAL_MODE
 *   - DA270_SLEEP_MODE
 *   - DA270_LOW_POWER_MODE
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_power_mode(uint8_t *power_mode, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the accel data along with the sensor-time
 *
 * @param[in,out] accel    : Structure instance to store data
 * @param[in] dev          : Structure instance of kx022_dev
 *
 *
 * @note : The Accel data value are in LSB based on the range selected
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_accel_data(struct kx022_sensor_data *accel, const struct kx022_dev *dev);

/*!
 * @brief This API is used to set the sensor settings like sensor
 * configurations and interrupt configurations like
 *    - Accelerometer configurations (Like ODR,OSR,range...)
 *    - Tap configurations
 *    - Activity change configurations
 *    - Gen1/Gen2 configurations
 *    - Orient change configurations
 *    - Step counter configurations
 *
 * @param[in] conf         : Structure instance of the configuration structure
 * @param[in] n_sett       : Number of settings to be set
 * @param[in] dev          : Structure instance of kx022_dev
 *
 * @note : Fill in the value of the required configurations in the conf structure
 * (Examples are mentioned in the readme.md) before calling this API
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_set_sensor_conf(const struct kx022_sensor_conf *conf, uint16_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the sensor settings like sensor
 * configurations and interrupt configurations and store
 * them in the corresponding structure instance
 *
 * @param[in] conf         : Structure instance of the configuration structure
 * @param[in] n_sett       : Number of settings to be obtained
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Call the API and the settings structure will be updated with the
 * sensor settings
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_sensor_conf(struct kx022_sensor_conf *conf, uint16_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to set the device specific settings like
 *  - DA270_AUTOWAKEUP_TIMEOUT
 *  - DA270_AUTOWAKEUP_INT
 *  - DA270_AUTO_LOW_POWER
 *  - DA270_INT_PIN_CONF
 *  - DA270_INT_OVERRUN_CONF
 *  - DA270_FIFO_CONF
 *
 * @param[in] conf         : Structure instance of the configuration structure.
 * @param[in] n_sett       : Number of settings to be set
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Fill in the value of the required configurations in the conf structure
 * (Examples are mentioned in the readme.md) before calling this API
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_set_device_conf(const struct kx022_device_conf *conf, uint8_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the device specific settings and store
 * them in the corresponding structure instance
 *
 * @param[in] conf         : Structure instance of the configuration structure
 * @param[in] n_sett       : Number of settings to be obtained
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Call the API and the settings structure will be updated with the
 * sensor settings
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_device_conf(struct kx022_device_conf *conf, uint8_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the status of all the interrupts
 * whether they are asserted or not
 *
 * @param[in] int_status   : Interrupt status of sensor
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Interrupt status of the sensor determines which all
 * interrupts are asserted at any instant of time
 *
 * Interrupt status macros :
 *  - DA270_WAKEUP_INT_ASSERTED
 *  - DA270_ORIENT_CH_INT_ASSERTED
 *  - DA270_GEN1_INT_ASSERTED
 *  - DA270_GEN2_INT_ASSERTED
 *  - DA270_FIFO_FULL_INT_ASSERTED
 *  - DA270_FIFO_WM_INT_ASSERTED
 *  - DA270_DRDY_INT_ASSERTED
 *  - DA270_INT_OVERRUN_ASSERTED
 *  - DA270_STEP_INT_ASSERTED
 *  - DA270_S_TAP_INT_ASSERTED
 *  - DA270_D_TAP_INT_ASSERTED
 *  - DA270_ACT_CH_X_ASSERTED
 *  - DA270_ACT_CH_Y_ASSERTED
 *  - DA270_ACT_CH_Z_ASSERTED
 *
 * @note : Call the API and then use the above macros to
 * check whether the interrupt is asserted or not
 *
 * if (int_status & DA270_FIFO_FULL_INT_ASSERTED) {
 *     printf("\n FIFO FULL INT ASSERTED");
 * }
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_interrupt_status(uint16_t *int_status, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the enable/disable
 * status of the various interrupts
 *
 * @param[in] int_select   : Structure to select specific interrupts
 * @param[in] n_sett       : Number of interrupt settings enabled / disabled
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Select the needed interrupt type for which the status of it whether
 * it is enabled/disabled is to be known in the int_select->int_sel, and the
 * output is stored in int_select->conf either as DA270_ENABLE/DA270_DISABLE
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_interrupts_enabled(struct kx022_int_enable *int_select, uint8_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to enable the various interrupts
 *
 * @param[in] int_select   : Structure to enable specific interrupts
 * @param[in] n_sett       : Number of interrupt settings enabled / disabled
 * @param[in] dev          : Structure instance of kx022_dev.
 *
 * @note : Multiple interrupt can be enabled/disabled by
 *    struct interrupt_enable int_select[2];
 *
 *    int_select[0].int_sel = DA270_FIFO_FULL_INT_EN;
 *    int_select[0].conf = DA270_ENABLE;
 *
 *    int_select[1].int_sel = DA270_FIFO_WM_INT_EN;
 *    int_select[1].conf = DA270_ENABLE;
 *
 *    rslt = kx022_enable_interrupt(&int_select, 2, dev);
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_enable_interrupt(const struct kx022_int_enable *int_select, uint8_t n_sett, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the step counter output in form
 * of number of steps in the step_count value
 *
 * @param[out] step_count      : Number of step counts
 * @param[out] activity_data   : Activity data WALK/STILL/RUN
 * @param[in] dev              : Structure instance of kx022_dev.
 *
 *  activity_data   |  Status
 * -----------------|------------------
 *  0x00            | DA270_STILL_ACT
 *  0x01            | DA270_WALK_ACT
 *  0x02            | DA270_RUN_ACT
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_steps_counted(uint32_t *step_count, uint8_t *activity_data, const struct kx022_dev *dev);

/*!
 * @brief This API is used to get the temperature data output
 *
 * @note Temperature data output must be divided by a factor of 10
 * Consider temperature_data = 195 ,
 * Then the actual temperature is 19.5 degree Celsius
 *
 * @param[in,out] temperature_data   : Temperature data
 * @param[in] dev                    : Structure instance of kx022_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_get_temperature_data(int16_t *temperature_data, const struct kx022_dev *dev);

/*!
 *  @brief This API writes fifo_flush command to command register.
 *  This action clears all data in the FIFO
 *
 *  @param[in] dev           : Structure instance of kx022_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
int8_t kx022_set_fifo_flush(const struct kx022_dev *dev);

/*!
 * @brief This is used to perform self test of accelerometer in DA270
 *
 * @param[in] dev    : Structure instance of kx022_dev.
 *
 * @note The return value of this API gives us the result of self test.
 *
 * @note Performing self test does soft reset of the sensor, User can
 * set the desired settings after performing the self test.
 *
 * @return Result of API execution status
 * @retval zero -> Success  / -ve value -> Error / +ve value -> Self-test fail
 *
 *   Return value                  |   Result of self test
 * --------------------------------|---------------------------------
 *  DA270_OK                      |  Self test success
 *  DA270_W_SELF_TEST_FAIL        |  self test fail
 */
int8_t kx022_perform_self_test(const struct kx022_dev *dev);

/*!
 * @brief This API is used to set the step counter's configuration
 * parameters from the registers 0x59 to 0x71
 */
int8_t kx022_set_step_counter_param(uint8_t *sccr_conf, const struct kx022_dev *dev);

#ifdef __cplusplus
}
#endif  /* End of CPP guard */

#endif  /* DA270_H__ */
/** @}*/
