//created by yangwensen@20200519

#include "kx022.h"

#define ARRAY_SIZE(ar)     (sizeof(ar)/sizeof(ar[0]))

struct kx022_register_value
{
    uint8_t reg_addr;
    uint8_t reg_val;
};

struct kx022_register_value KX022_INIT_TAB[] = 
{
    {KX022_CNTL1_ADDR,      0x00},              //standby mode, resulution=8bits, availability of new acceleration data is not reflected as an interrupt, range=+-2g, Directional Tap disable, wakeup disable, Tilt Position disable
    {KX022_ODCNTL_ADDR,     0x03},              //iir filter applied, low-pass filter corner frequency set to ODR/9, odr=100hz
    {KX022_BUF_CNTL1_ADDR,  60},                //the number of samples that will trigger a watermark interrupt,max=84 when BUF_RES = 0
    {KX022_BUF_CNTL2_ADDR,  0xa2},              //sample buffer active, 8-bit sample, buffer full interrupt enable,buffer mode=TRIGGER
    {KX022_INC1_ADDR,       0x20},              //INT1 pin enable, active low, interrupt pin latches until cleared by reading INT_REL, self test polarity=negative, SPI disable
    {KX022_INC2_ADDR,       0x3f},              //all axis and direction of detected motion can cause an interrupt
    {KX022_INC3_ADDR,       0x3f},              //all axis and direction of tap/double tap can cause an interrupt
    {KX022_INC4_ADDR,       0x60},              //buffer full int enable, watermark interrupt enable, data ready int disable, tap/double tap int disable, wakeup interrupt disable, tilt position int disable
    {KX022_TDTRC_ADDR,      0x03},              //update interrupts on single tap and double tap events
    {KX022_TDTC_ADDR,       0x25},              //the minimum time sepatation between the first tap and the second tap in a double tap event.default is 0.3 seconds.
    {KX022_TTH_ADDR,        0xcb},              //This register represents the 8-bit jerk high threshold to determine if a tap is detected.default is 203.
    {KX022_TTL_ADDR,        0x1a},              //This register represents the 8-bit jerk low threshold to determine if a tap is detected.default is 26.
    {KX022_FTD_ADDR,        0xa2},              //This register contains counter information for the detection of any tap event
    {KX022_STD_ADDR,        0x24},              //This register contains counter information for the detection of any tap event
    {KX022_TLT_ADDR,        0x28},              //This register contains counter information for the detection of any tap event
    {KX022_TWS_ADDR,        0xa0},              //This register contains counter information for the detection of any tap event
};

struct kx022_register_value KX022_INT_INIT_TAB[] = 
{
    {KX022_CNTL1_ADDR,      0x00},              //standby mode, resulution=8bits, availability of new acceleration data is not reflected as an interrupt, range=+-2g, Directional Tap disable, wakeup disable, Tilt Position disable
    {KX022_ODCNTL_ADDR,     0x03},              //iir filter applied, low-pass filter corner frequency set to ODR/9, odr=100hz
    {KX022_BUF_CNTL1_ADDR,  60},                //the number of samples that will trigger a watermark interrupt,max=84 when BUF_RES = 0
    {KX022_BUF_CNTL2_ADDR,  0xa2},              //sample buffer active, 8-bit sample, buffer full interrupt enable,buffer mode=TRIGGER
};
/************************** Internal macros *******************************/
/********************** Static function declarations ************************/
/*!
 * @brief This internal API is used to validate the device pointer for
 * null conditions.
 *
 * @param[in] dev : Structure instance of kx022_dev.
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
static int8_t null_ptr_check(const struct kx022_dev *dev);

/*!
 * @brief This internal API is used to set the accel configurations in sensor
 *
 * @param[in] accel_conf : Structure instance with accel configurations
 * @param[in] dev             : Structure instance of kx022_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
static int8_t set_accel_conf(const struct kx022_acc_conf *accel_conf, const struct kx022_dev *dev);

/*!
 * @brief This API reads accel data along with sensor time
 *
 * @param[in,out] accel  : Structure instance to store the accel data
 * @param[in] dev        : Structure instance of kx022_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
static int8_t get_accel_data(struct kx022_sensor_data *accel, const struct kx022_dev *dev);

/*!
 * @brief This internal API is used to get the accel configurations in sensor
 *
 * @param[in,out] accel_conf  : Structure instance of basic
 *                              accelerometer configuration
 * @param[in] dev             : Structure instance of kx022_dev
 *
 * @return Result of API execution status
 * @retval zero -> Success / +ve value -> Warning / -ve value -> Error
 */
static int8_t get_accel_conf(struct kx022_acc_conf *accel_conf, const struct kx022_dev *dev);

/********************** Global function definitions ************************/
/*!
 *  @brief This API is the entry point, Call this API before using other APIs.
 *  This API reads the chip-id of the sensor which is the first step to
 *  verify the sensor and updates the trim parameters of the sensor.
 */
int8_t kx022_init(struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t i = 0;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if (rslt != KX022_OK)
        return -1;
    
	/* Initial power-up time */
	dev->delay_ms(5);
	/* Assigning dummy byte value, TODO */
	if (dev->intf == KX022_SPI_INTF) {
		/* Dummy Byte availability */
		dev->dummy_byte = 1;
		/* Dummy read of Chip-ID in SPI mode */
	} else {
		dev->dummy_byte = 0;
	}
    
    if( kx022_soft_reset(dev) != KX022_OK)
        return -2;
    
	rslt = kx022_get_regs(KX022_CHIP_ID_ADDR, &i, 1, dev);
	if (rslt != KX022_OK)
        return -3;
    
	/* Check for chip id validity */
	if (i != KX022_CHIP_ID) 
        return -4;
    
	/* Store the chip ID in dev structure */
	dev->chip_id = i;
    
    for(i=0; i<ARRAY_SIZE(KX022_INIT_TAB); i++)
    {
        rslt = dev->write(dev->intf_ptr, dev->dev_id, KX022_INIT_TAB[i].reg_addr, &KX022_INIT_TAB[i].reg_val, 1);
		if (rslt != KX022_OK)
            return -5;
	    dev->delay_ms(10);
    }
    
	return 0;
}

/*!
 * @brief This API writes the given data to the register address
 * of the sensor.
 */
int8_t kx022_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t count;

	/* Check for null pointer in the device structure */
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if ((rslt == KX022_OK) && (reg_data != NULL)) {
		/* Write the data to the reg_addr */
		/* SPI write requires to set The MSB of reg_addr as 0
		   but in default the MSB is always 0 */
		if (len == 1) {
			rslt = dev->write(dev->intf_ptr, dev->dev_id, reg_addr, reg_data, len);
			if (rslt != KX022_OK) {
				/* Failure case */
				rslt = KX022_E_COM_FAIL;
			}
		}
		/* Burst write is not allowed thus we split burst case write
		 * into single byte writes Thus user can write multiple bytes
		 * with ease */
		if (len > 1) {
			for (count = 0; count < len; count++) {
				rslt = dev->write(dev->intf_ptr, dev->dev_id, reg_addr, &reg_data[count], 1);
				reg_addr++;
			}
		}
	} else {
		rslt = KX022_E_NULL_PTR;
	}

	return rslt;
}

/*!
 * @brief This API reads the data from the given register address of the sensor.
 */
int8_t kx022_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint8_t len, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint16_t index;
	uint16_t temp_len = len;
	uint8_t temp_buff[temp_len];

	/* Check for null pointer in the device structure */
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if ((rslt == KX022_OK) && (reg_data != NULL)) {
		if (dev->intf != KX022_I2C_INTF) {
			/* If interface selected is SPI */
			reg_addr = reg_addr | KX022_SPI_RD_MASK;
		}
		/* Read the data from the reg_addr */
		rslt = dev->read(dev->intf_ptr, dev->dev_id, reg_addr, temp_buff, temp_len);
		if (rslt == KX022_OK) {
			for (index = 0; index < len; index++) {
				/* Parse the data read and store in "reg_data"
				 * buffer so that the dummy byte is removed
				 * and user will get only valid data
				 */
				reg_data[index] = temp_buff[index];
			}
		}
		if (rslt != KX022_OK) {
			/* Failure case */
			rslt = KX022_E_COM_FAIL;
		}
	} else {
		rslt = KX022_E_NULL_PTR;
	}

	return rslt;
}

//by yangwensen@20200521
int8_t kx022_set_reg_bitfield(uint8_t reg_addr, uint8_t bit_mask, uint8_t new_bitfield, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t reg_val;

	/* Check for null pointer in the device structure */
	rslt = null_ptr_check(dev);
    
    if(rslt != KX022_OK)
        return KX022_E_NULL_PTR;
    
	if (dev->intf != KX022_I2C_INTF) {
		/* If interface selected is SPI */
		reg_addr = reg_addr | KX022_SPI_RD_MASK;
	}
    
	rslt = dev->read(dev->intf_ptr, dev->dev_id, reg_addr, &reg_val, 1);
    if(rslt != KX022_OK)
        return KX022_E_COM_FAIL;
    
    reg_val = (reg_val & bit_mask) | new_bitfield;
	rslt = dev->write(dev->intf_ptr, dev->dev_id, reg_addr, &reg_val, 1);
    if(rslt != KX022_OK)
        return KX022_E_COM_FAIL;

	return rslt;
}

/*!
 * @brief This API is used to perform soft-reset of the sensor
 * where all the registers are reset to their default values.
 */
int8_t kx022_soft_reset(const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data;

	/* Null-pointer check */
	rslt = null_ptr_check(dev);
	if (rslt == KX022_OK) {
		/* Reset the device */
		rslt = kx022_get_regs(KX022_CNTL2_ADDR, &data, 1, dev);
		data = KX022_SET_BITS(data, KX022_SOFT_RESET_CMD, KX022_SOFT_RESET);
		rslt = kx022_set_regs(KX022_CNTL2_ADDR, &data, 1, dev);
		dev->delay_ms(KX022_SOFT_RESET_DELAY_MS);
		if ((rslt == KX022_OK) && (dev->intf == KX022_SPI_INTF)) {
			/* Dummy read of 0x7F register to enable SPI Interface
			 * if SPI is used
			 */
			rslt = kx022_get_regs(0x7F, &data, 1, dev);
		}
	}

	return rslt;
}

/*!
 * @brief This API is used to set the power mode of the sensor.
 */
int8_t kx022_set_power_mode(uint8_t power_mode, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t reg_data = 0;

	rslt = null_ptr_check(dev);

	if (rslt == KX022_OK) {
		rslt = kx022_get_regs(KX022_CNTL1_ADDR, &reg_data, 1, dev);
	}

	if (rslt == KX022_OK) {
		reg_data = KX022_SET_BITS(reg_data, KX022_POWER_MODE, power_mode);
		/* Set the power mode of sensor */
		rslt = kx022_set_regs(KX022_CNTL1_ADDR, &reg_data, 1, dev);
		if (power_mode == KX022_LOW_POWER_MODE) {
			/* A delay of 1/ODR is required to switch power modes*/
			dev->delay_ms(100);
		} else {
			dev->delay_ms(100);
		}
	}

	return rslt;
}

/*!
 * @brief This API is used to get the power mode of the sensor.
 */
int8_t kx022_get_power_mode(uint8_t *power_mode, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t reg_data;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if (rslt == KX022_OK) {
		rslt = kx022_get_regs(KX022_STATUS_ADDR, &reg_data, 1, dev);
		*power_mode = KX022_GET_BITS(reg_data, KX022_POWER_MODE_STATUS);
	}

	return rslt;
}

/*!
 * @brief This API is used to get the accel data along with the sensor-time
 */
int8_t kx022_get_accel_data(struct kx022_sensor_data *accel, const struct kx022_dev *dev)
{
	int8_t rslt;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if ((rslt == KX022_OK) && (accel != NULL)) {
		/* Read and store the accel data */
		rslt = get_accel_data(accel, dev);
	} else {
		rslt = KX022_E_NULL_PTR;
	}

	return rslt;
}

/*!
 * @brief This API is used to set the sensor settings like sensor
 * configurations and interrupt configurations
 */
int8_t kx022_set_sensor_conf(const struct kx022_sensor_conf *conf, uint16_t n_sett, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint16_t idx = 0;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
    if (rslt == KX022_OK) {
        for (idx = 0; idx < n_sett; idx++) {
            switch (conf[idx].type) {
            case KX022_ACCEL:
                /* Setting Accel configurations */
                rslt = set_accel_conf(&conf[idx].param.accel, dev);
                if (rslt == KX022_OK) {
                    /* Set the INT pin mapping */
                }
                break;
		    default:
				rslt = KX022_E_INVALID_CONFIG;
			}
		}
	}
	return rslt;
}

/*!
 * @brief This API is used to get the sensor settings like sensor
 * configurations and interrupt configurations and store
 * them in the corresponding structure instance
 */
int8_t kx022_get_sensor_conf(struct kx022_sensor_conf *conf, uint16_t n_sett, const struct kx022_dev *dev)
{
	int8_t rslt = KX022_OK;
	uint16_t idx = 0;

	if (conf == NULL) {
		rslt = KX022_E_NULL_PTR;
	}

	for (idx = 0; (idx < n_sett) && (rslt == KX022_OK); idx++) {
		switch (conf[idx].type) {
		case KX022_ACCEL:
			/* Accel configuration settings */
			rslt = get_accel_conf(&conf[idx].param.accel, dev);
			if (rslt == KX022_OK) {
				/* Get the INT pin mapping */
			}
			break;
		default:
			rslt = KX022_E_INVALID_CONFIG;
		}
	}

	return rslt;
}

/*!
 * @brief This API is used to get the device specific settings and store
 * them in the corresponding structure instance
 */
int8_t kx022_get_device_conf(struct kx022_device_conf *conf, uint8_t n_sett, const struct kx022_dev *dev)
{
	int8_t rslt;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if (rslt == KX022_OK) {

	}

	return rslt;
}

/****************************************************************************/
/**\name	INTERNAL APIs                                               */
/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
static int8_t null_ptr_check(const struct kx022_dev *dev)
{
	int8_t rslt;

	if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_ms == NULL)) {
		/* Device structure pointer is not valid */
		rslt = KX022_E_NULL_PTR;
	} else {
		/* Device structure is fine */
		rslt = KX022_OK;
	}

	return rslt;
}

/*!
 * @brief This internal API is used to set the accel configurations in sensor
 */
static int8_t set_accel_conf(const struct kx022_acc_conf *accel_conf, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data_array[4] = { 0, 0, 0, 0 };

	/* Update the accel configurations from the user structure
	 * accel_conf */
	rslt = kx022_get_regs(KX022_CNTL1_ADDR, data_array, 4, dev);
	if (rslt == KX022_OK) {
		data_array[0] = KX022_SET_BITS(data_array[0], KX022_ACCEL_RANGE, accel_conf->range);
		data_array[3] = KX022_SET_BITS_POS_0(data_array[3], KX022_ACCEL_ODR, accel_conf->odr);
		data_array[3] = KX022_SET_BITS(data_array[3], KX022_BW, accel_conf->bw);
		/* Set the accel configurations in the sensor */
		rslt = kx022_set_regs(KX022_CNTL1_ADDR, data_array, 4, dev);
	}

	return rslt;
}

/*!
 * @brief This internal API is used to set the accel configurations in sensor
 */
static int8_t get_accel_conf(struct kx022_acc_conf *accel_conf, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data_array[4];

	rslt = kx022_get_regs(KX022_CNTL1_ADDR, data_array, 4, dev);
	if (rslt == KX022_OK) {
		accel_conf->range = KX022_GET_BITS(data_array[0], KX022_ACCEL_RANGE);   //CTRL1
		accel_conf->odr = KX022_GET_BITS_POS_0(data_array[3], KX022_ACCEL_ODR); //ODCNTL
		accel_conf->bw = KX022_GET_BITS(data_array[3], KX022_BW);		
	}

	return rslt;
}

/*!
 * @brief This API reads accel data along with sensor time
 */
static int8_t get_accel_data(struct kx022_sensor_data *accel, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data_array[6] = { 0 };

	/* Read the sensor data registers only */
	rslt = kx022_get_regs(KX022_ACCEL_DATA_ADDR, data_array, 6, dev);

	if (rslt == KX022_OK) {
		accel->x = ((int16_t)(data_array[1] << 8 | data_array[0]))>> 0;
		accel->y = ((int16_t)(data_array[3] << 8 | data_array[2]))>> 0;
		accel->z = ((int16_t)(data_array[5] << 8 | data_array[4]))>> 0;
	}

	return rslt;
}
