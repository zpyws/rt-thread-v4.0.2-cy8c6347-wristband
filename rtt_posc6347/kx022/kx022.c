//created by yangwensen@20200519

#include "kx022.h"
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
	uint8_t chip_id = 0;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	/* Proceed if null check is fine */
	if (rslt == KX022_OK) {
		/* Initial power-up time */
		dev->delay_ms(5);
		/* Assigning dummy byte value, TODO */
		if (dev->intf == KX022_SPI_INTF) {
			/* Dummy Byte availability */
			dev->dummy_byte = 1;
			/* Dummy read of Chip-ID in SPI mode */
			rslt = kx022_get_regs(KX022_CHIP_ID_ADDR, &chip_id, 1, dev);
		} else {
			dev->dummy_byte = 0;
		}
		if (rslt == KX022_OK) {
			/* Chip ID of the sensor is read */
			rslt = kx022_get_regs(KX022_CHIP_ID_ADDR, &chip_id, 1, dev);
			/* Proceed if everything is fine until now */
			if (rslt == KX022_OK) {
				/* Check for chip id validity */
				if (chip_id == KX022_CHIP_ID) {
					/* Store the chip ID in dev structure */
					dev->chip_id = chip_id;
				} else {
					rslt = KX022_E_DEV_NOT_FOUND;
				}
			}
		}
	}

	return rslt;
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

/*!
 * @brief This API is used to perform soft-reset of the sensor
 * where all the registers are reset to their default values.
 */
int8_t kx022_soft_reset(const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data = KX022_SOFT_RESET_CMD;

	/* Null-pointer check */
	rslt = null_ptr_check(dev);
	if (rslt == KX022_OK) {
		/* Reset the device */
		rslt = kx022_get_regs(0x00, &data, 1, dev);
		data = KX022_SET_BITS_POS_0(data, KX022_SOFT_RESET_CMD, KX022_SOFT_RESET_CMD);
		rslt = kx022_set_regs(KX022_SPI_CONFIG, &data, 1, dev);
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
		rslt = kx022_get_regs(KX022_ACCEL_MODE_BW, &reg_data, 1, dev);
	}

	if (rslt == KX022_OK) {
		reg_data = KX022_SET_BITS(reg_data, KX022_POWER_MODE, power_mode);
		/* Set the power mode of sensor */
		rslt = kx022_set_regs(KX022_ACCEL_MODE_BW, &reg_data, 1, dev);
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
	uint8_t data_array[3] = { 0, 0, 0 };

	/* Update the accel configurations from the user structure
	 * accel_conf */
	rslt = kx022_get_regs(KX022_ACCEL_RESOLUTION_RANGE, data_array, 3, dev);
	if (rslt == KX022_OK) {
		data_array[0] = KX022_SET_BITS(data_array[0], KX022_ACCEL_RANGE, accel_conf->range);
		data_array[1] = KX022_SET_BITS_POS_0(data_array[1], KX022_ACCEL_ODR, accel_conf->odr);
		data_array[2] = KX022_SET_BITS(data_array[2], KX022_BW, accel_conf->bw);
		/* Set the accel configurations in the sensor */
		rslt = kx022_set_regs(KX022_ACCEL_RESOLUTION_RANGE, data_array, 3, dev);
	}

	return rslt;
}

/*!
 * @brief This internal API is used to set the accel configurations in sensor
 */
static int8_t get_accel_conf(struct kx022_acc_conf *accel_conf, const struct kx022_dev *dev)
{
	int8_t rslt;
	uint8_t data_array[3];

	rslt = kx022_get_regs(KX022_ACCEL_RESOLUTION_RANGE, data_array, 3, dev);
	if (rslt == KX022_OK) {
		accel_conf->range = KX022_GET_BITS(data_array[0], KX022_ACCEL_RANGE);
		accel_conf->odr = KX022_GET_BITS_POS_0(data_array[1], KX022_ACCEL_ODR);
		accel_conf->bw = KX022_GET_BITS(data_array[2], KX022_BW);		
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
		accel->x = ((int16_t)(data_array[1] << 8 | data_array[0]))>> 4;
		accel->y = ((int16_t)(data_array[3] << 8 | data_array[2]))>> 4;
		accel->z = ((int16_t)(data_array[5] << 8 | data_array[4]))>> 4;
	}

	return rslt;
}
