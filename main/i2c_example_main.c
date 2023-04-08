
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <sdkconfig.h>
#include "ssd1306.h"
#include "font8x8_basic.h"
static const char *TAG = "Test-I2C";

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define DATA_LENGTH 512                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000 /*!< delay time between different test items */

#define I2C_MASTER_SCL_OLED 21               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_OLED 22              /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM_OLED I2C_NUMBER(0) /*!< I2C port number for master ssd */
#define I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   

#define ESP_SLAVE_ADDR_OLED 0x3C /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT 0 /*!< I2C master write */
#define READ_BIT 1 /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

/**
 * @brief Test code to write esp-i2c-slave
 *   
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
//-----------------------------BT 3------------------------------------------------------------
uint8_t logoUIT [1024] = {
	// 'rsz_logo-uit', 128x64px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x7f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x3f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x70, 0x00, 0x1f, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xf8, 0x00, 0x03, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x7d, 0xff, 0xfc, 0x00, 0x70, 0xff, 
0xff, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xff, 0x01, 0xfe, 0x3f, 
0xff, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0x1f, 
0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xe0, 0x0f, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0x8f, 
0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xc7, 
0xff, 0xff, 0xfe, 0x03, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 
0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 
0xff, 0xff, 0xf8, 0x3f, 0xff, 0xfe, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 
0xff, 0xff, 0xf0, 0xf7, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 
0xff, 0xff, 0xe3, 0xdf, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xfb, 
0xff, 0xff, 0xcf, 0x7b, 0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xfb, 
0xff, 0xff, 0xfd, 0xef, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xfb, 
0xff, 0xff, 0xf7, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 
0xff, 0xff, 0xfe, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 
0xff, 0xff, 0xff, 0xfe, 0x03, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 
0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0x07, 0xff, 0x03, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xdf, 0xe0, 0x1f, 0xfc, 0x01, 0xfe, 0x01, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xef, 
0xff, 0xff, 0x7f, 0x80, 0x7f, 0xf8, 0x07, 0xff, 0x80, 0x7f, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xdf, 
0xff, 0xfe, 0xff, 0x01, 0xff, 0xf0, 0x1f, 0x07, 0xe0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x03, 0xff, 0xe0, 0x7c, 0x01, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf0, 0x0f, 0xff, 0xc0, 0xf8, 0x00, 0x7c, 0x1f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xe0, 0x1f, 0xff, 0xc3, 0xe0, 0x00, 0x3e, 0x0f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x80, 0x7f, 0xff, 0x87, 0xc0, 0x00, 0x0f, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x00, 0xff, 0xff, 0x8f, 0x00, 0x00, 0x07, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xfc, 0x01, 0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 
0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 
0xff, 0xf0, 0x0f, 0xf8, 0x0f, 0x7b, 0xff, 0xcf, 0xff, 0xf3, 0x80, 0xff, 0xfb, 0xff, 0xff, 0xff, 
0xff, 0xe0, 0x1f, 0xf8, 0x0e, 0xf0, 0x1f, 0xff, 0xc0, 0x7b, 0x80, 0x7f, 0xf7, 0xff, 0xff, 0xff, 
0xff, 0xc0, 0x3f, 0xf8, 0x0f, 0xe0, 0x03, 0xff, 0x00, 0x3f, 0x80, 0x7f, 0xf7, 0xff, 0xff, 0xff, 
0xff, 0x80, 0x7f, 0xf8, 0x0f, 0xc0, 0x1f, 0xff, 0xe0, 0x1f, 0x80, 0xff, 0xef, 0xff, 0xff, 0xff, 
0xff, 0x00, 0xff, 0xfc, 0x0f, 0xc0, 0x7f, 0xff, 0xf8, 0x0f, 0x80, 0xff, 0xef, 0xff, 0xff, 0xff, 
0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 
0xfc, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 
0xf8, 0x03, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0x7f, 0xff, 0xff, 0xff, 
0xf0, 0x03, 0xff, 0xff, 0xf8, 0x01, 0xff, 0xff, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xf0, 0x07, 0xff, 0xff, 0xff, 0x00, 0x7f, 0xff, 0xf8, 0x03, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 
0xe0, 0x07, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xf0, 0x0f, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x07, 0xff, 0x80, 0x3f, 0xf0, 0x1f, 0xff, 0xc0, 0x7f, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x07, 0xff, 0xc0, 0x0f, 0xfc, 0x0f, 0xff, 0x81, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x07, 0xff, 0xf0, 0x03, 0xff, 0x83, 0xff, 0x07, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0x80, 0x07, 0xff, 0xfc, 0x00, 0x7f, 0xe1, 0xfe, 0x1f, 0xf0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x80, 0x07, 0xff, 0xff, 0x00, 0x0f, 0xf8, 0xf8, 0xff, 0x80, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x80, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x7c, 0x71, 0xf0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0x80, 0x01, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x01, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x00, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xe0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xf0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xf8, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xfe, 0x00, 0x00, 0x01, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
typedef struct {
	bool _valid; // Not using it anymore
	int _segLen; // Not using it anymore
	uint8_t _segs[128];
} PAGE_t;

typedef struct {
	int _address;
	int _width;
	int _height;
	int _pages;
	int _dc;
	bool _scEnable;
	int _scStart;
	int _scEnd;
	PAGE_t _page[8];
	int _scDirection;
	bool _flip;
} SSD1306_t;
uint8_t ssd1306_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits)
{
	ESP_LOGD(TAG, "src=%02x srcBits=%d dst=%02x dstBits=%d", src, srcBits, dst, dstBits);
	uint8_t smask = 0x01 << srcBits;
	uint8_t dmask = 0x01 << dstBits;
	uint8_t _src = src & smask;
#if 0
	if (_src != 0) _src = 1;
	uint8_t _wk = _src << dstBits;
	uint8_t _dst = dst | _wk;
#endif
	uint8_t _dst;
	if (_src != 0) {
		_dst = dst | dmask; // set bit
	} else {
		_dst = dst & ~(dmask); // clear bit
	}
	return _dst;
}

// Rotate 8-bit data
// 0x12-->0x48
uint8_t ssd1306_rotate_byte(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}
void i2c_display_image(SSD1306_t * ssd, int page, int seg, uint8_t * images, int width) {
	i2c_cmd_handle_t cmd;

	if (page >= ssd->_pages) return;
	if (seg >= ssd->_width) return;

	int _seg = seg + 0x8C;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (ssd->_flip) {
		_page = (ssd->_pages - page) - 1;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (ssd->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xB0 | _page, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (ssd->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void ssd1306_display_image(SSD1306_t * ssd, int page, int seg, uint8_t * images, int width)
{
	
		i2c_display_image(ssd, page, seg, images, width);
	// Set to internal buffer
	memcpy(&ssd->_page[page]._segs[seg], images, width);
}

void ssd1306_show_buffer(SSD1306_t * ssd)
{
		for (int page=0; page<ssd->_pages;page++) {
			i2c_display_image(ssd, page, 0, ssd->_page[page]._segs, ssd->_width);
		}
	}


void ssd1306_bitmaps(SSD1306_t * ssd, int xpos, int ypos, uint8_t * bitmap, int width, int height, bool invert)
{
	if ( (width % 8) != 0) {
		ESP_LOGE(TAG, "width must be a multiple of 8");
		return;
	}
	int _width = width / 8;
	uint8_t wk0;
	uint8_t wk1;
	uint8_t wk2;
	uint8_t page = (ypos / 8);
	uint8_t _seg = xpos;
	uint8_t dstBits = (ypos % 8);
	ESP_LOGD(TAG, "ypos=%d page=%d dstBits=%d", ypos, page, dstBits);
	int offset = 0;
	for(int _height=0;_height<height;_height++) {
		for (int index=0;index<_width;index++) {
			for (int srcBits=7; srcBits>=0; srcBits--) {
				wk0 = ssd->_page[page]._segs[_seg];
				if (ssd->_flip) wk0 = ssd1306_rotate_byte(wk0);

				wk1 = bitmap[index+offset];
				if (invert) wk1 = ~wk1;

				//wk2 = ssd1306_copy_bit(bitmap[index+offset], srcBits, wk0, dstBits);
				wk2 = ssd1306_copy_bit(wk1, srcBits, wk0, dstBits);
				if (ssd->_flip) wk2 = ssd1306_rotate_byte(wk2);

				ESP_LOGD(TAG, "index=%d offset=%d page=%d _seg=%d, wk2=%02x", index, offset, page, _seg, wk2);
				ssd->_page[page]._segs[_seg] = wk2;
				_seg++;
			}
		}
		vTaskDelay(1);
		offset = offset + _width;
		dstBits++;
		_seg = xpos;
		if (dstBits == 8) {
			page++;
			dstBits=0;
		}
	}

#if 0
	for (int _seg=ypos;_seg<ypos+width;_seg++) {
		ssd1306_dump_page(ssd, page-1, _seg);
	}
	for (int _seg=ypos;_seg<ypos+width;_seg++) {
		ssd1306_dump_page(ssd, page, _seg);
	}
#endif
	ssd1306_show_buffer(ssd);
}

//-----------------------------BT 1, 2---------------------------------------------------------------------------
/**
 * @brief i2c master initialization
 */
void i2c_master_init(SSD1306_t *ssd)
{
    int i2c_master_port = I2C_MASTER_NUM_OLED;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_OLED;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_OLED;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	ssd->_address = OLED_I2C_ADDRESS;
}

void ssd1306_init(SSD1306_t * ssd, int width, int height) {
	ssd->_width = width;
	ssd->_height = height;
	ssd->_pages = 8;
	if (ssd->_height == 32) ssd->_pages = 4;
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);
    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void task_ssd1306_display_text(const void *arg_text) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column - choose column --> 0
	i2c_master_write_byte(cmd, 0x10, true); // reset line - choose line --> 0
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x00, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}

	vTaskDelete(NULL);
}

void task_ssd1306_display_clear(void *ignore) {
	i2c_cmd_handle_t cmd;

	uint8_t clear[128];
	for (uint8_t i = 0; i < 128; i++) {
		clear[i] = 0;
	}
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, clear, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	vTaskDelete(NULL);
}

void app_main(void)
{
	SSD1306_t ssd;
    ESP_LOGI(TAG, "Initialize I2C Master"); 
    i2c_master_init(&ssd);
    ESP_LOGI(TAG, "Initialize successful");
    ESP_LOGI(TAG, "Initialize OLED");
    ssd1306_init(&ssd, 128, 64);
    xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  2048, NULL, 6, NULL);
    vTaskDelay(100/portTICK_PERIOD_MS);
    //Bt 2
	//xTaskCreate(&task_ssd1306_display_text, "ssd1306_display_text",  2048,
	//	"20520721\n20520705\n20520946", 6, NULL);
	
//-------------------------------------------------------------------------	
	 //Bt 3
	 ssd1306_bitmaps(&ssd, 0, 0, logoUIT, 128, 64, false);
//-------------------------------------------------------------------------	 
    ESP_LOGI(TAG, "FINISH");
    vTaskDelay(DELAY_TIME_BETWEEN_ITEMS_MS / portTICK_PERIOD_MS);
}