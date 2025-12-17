#include "appdefs.h"
#include "init.h"
#include "display.h"
#include "cs.h"
#include "metronom.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/spi.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/exti.h"
#include "libopencm3/stm32/timer.h"
#include "libopencm3/stm32/usart.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>

#include "gui.h"

#include "fs_stream.h"

#include "abstractmenu.h"
#include "menuplayer.h"

#include "player.h"

// Main top-level unit
MenuPlayer* menuPlayer;
Player player;

dac_sample_t dac_sample;
dac_sample_t dac_sample1;

uint8_t sys_param[64];

uint8_t ctrl_param[32];
uint8_t pc_param[256];
uint8_t midi_pc = 0;

wav_sample_t sound_buff[Player::wav_buff_size];
wav_sample_t click_buff[Player::wav_buff_size];

const target_t first_target =
{
	(char*) sound_buff,
	(char*) click_buff,
	Player::wav_buff_size / 2 * sizeof(wav_sample_t)
};

const target_t second_target =
{
	(char*) (sound_buff + Player::wav_buff_size / 2),
	(char*) (click_buff + Player::wav_buff_size / 2),
	Player::wav_buff_size / 2 * sizeof(wav_sample_t)
};

volatile uint8_t encoder_state, encoder_state1, encoder_key, key_ind;

volatile uint32_t click_size;
volatile uint32_t count_down;
volatile uint32_t count_up;

volatile uint32_t play_point1 = 0;
volatile uint32_t play_point2 = 0;
volatile uint8_t pause_fl = 0;

uint32_t song_size;

uint16_t key_reg_in[2];
uint16_t key_reg_out[2] =
{ 0, 0 };
uint8_t key_val;

//-----------------------------------------------------------------
void init(void)
{
	rcc_periph_clock_enable (RCC_DMA1);
	rcc_periph_clock_enable (RCC_DMA2);
	rcc_periph_clock_enable (RCC_SPI1);
	rcc_periph_clock_enable (RCC_SPI2);
	rcc_periph_clock_enable (RCC_SPI3);
	rcc_periph_clock_enable (RCC_GPIOA);
	rcc_periph_clock_enable (RCC_GPIOB);
	rcc_periph_clock_enable (RCC_GPIOC);
	rcc_periph_clock_enable (RCC_GPIOD);
	rcc_periph_clock_enable (RCC_SYSCFG);
	rcc_periph_clock_enable (RCC_TIM3);
	rcc_periph_clock_enable (RCC_TIM4);
	rcc_periph_clock_enable (RCC_TIM5);
	rcc_periph_clock_enable (RCC_TIM6);
	rcc_periph_clock_enable (RCC_TIM9);
	rcc_periph_clock_enable (RCC_USART1);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO5 | GPIO7 | GPIO15);     // SPI1 CLK | MOSI | I2S3 WS
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
			GPIO5 | GPIO7 | GPIO15);
	gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO7);
	gpio_set_af(GPIOA, GPIO_AF6, GPIO15);

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO3 | GPIO4 | GPIO5 | GPIO10 | GPIO12 | GPIO15 | GPIO6 | GPIO7); // I2S3 CK | SPI1 MISO | I2S3 SD
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
			GPIO3 | GPIO4 | GPIO5 | GPIO10 | GPIO12 | GPIO15 | GPIO6 | GPIO7); // I2S2 CK | I2S2 WS   | I2S2 SD | USART1 TX | USART1 RX
	gpio_set_af(GPIOB, GPIO_AF5, GPIO4);
	gpio_set_af(GPIOB, GPIO_AF6, GPIO3);
	gpio_set_af(GPIOB, GPIO_AF6, GPIO5);
	gpio_set_af(GPIOB, GPIO_AF5, GPIO10);
	gpio_set_af(GPIOB, GPIO_AF5, GPIO12);
	gpio_set_af(GPIOB, GPIO_AF5, GPIO15);
	gpio_set_af(GPIOB, GPIO_AF7, GPIO6);
	gpio_set_af(GPIOB, GPIO_AF7, GPIO7);

	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);    // I2S2 MCLK
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO6);
	gpio_set_af(GPIOC, GPIO_AF5, GPIO6);

	gpio_clear(GPIOC, GPIO14);
	gpio_clear(GPIOC, GPIO7);
	gpio_set(GPIOC, GPIO15);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO7 | GPIO14 | GPIO15);  // DAC PWD | LED CS | KEY CS
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
			GPIO7 | GPIO14 | GPIO15);

	gpio_set(GPIOA, GPIO4);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);  // DAC CS
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO4);

	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO8); // SD chek
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO11); // encoder but
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO4); // encoder a
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO5); // encoder b
	gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO0);   // esc

	exti_select_source(EXTI4, GPIOC);
	exti_set_trigger(EXTI4, EXTI_TRIGGER_FALLING);
	exti_enable_request (EXTI4);

	exti_select_source(EXTI8, GPIOB);
	exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
	exti_enable_request (EXTI8);

	nvic_enable_irq (NVIC_EXTI4_IRQ);
	nvic_set_priority(NVIC_EXTI4_IRQ,
			configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);

	timer_reset (RST_TIM9);
	TIM9_CR1 = TIM_CR1_ARPE | TIM_CR1_OPM;
	TIM9_CNT = TIM9_ARR = 0xffff;
	TIM9_PSC = 0x6;
	timer_direction_up (TIM9);
	TIM9_CR1 |= TIM_CR1_CEN;

	timer_reset (RST_TIM6);
	TIM6_CR1 = TIM_CR1_ARPE | TIM_CR1_OPM;
	TIM6_CNT = TIM6_ARR = 0xffff;
	TIM6_PSC = 0x1ff;
	timer_direction_up (TIM6);
	TIM6_CR1 |= TIM_CR1_CEN;

	timer_reset (RST_TIM3);
	TIM3_CR1 = TIM_CR1_ARPE | TIM_CR1_OPM;
	TIM3_CNT = TIM3_ARR = 0xffff;
	TIM3_PSC = 0x1fff;
	timer_direction_up (TIM3);
	TIM3_CR1 |= TIM_CR1_CEN;

	timer_reset (RST_TIM4);
	TIM4_CR1 = TIM_CR1_ARPE | TIM_CR1_OPM;
	TIM4_CNT = TIM4_ARR = 0xfff;
	TIM4_PSC = 0;
	timer_direction_up (TIM4);
	timer_interrupt_source(TIM4, TIM_SR_UIF);
	timer_enable_irq(TIM4, TIM_DIER_UIE);

	nvic_enable_irq (NVIC_TIM3_IRQ);
	nvic_enable_irq (NVIC_TIM4_IRQ);
	nvic_set_priority(NVIC_TIM3_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);
	//-----------------------------------------------------------------------------------------------------------------
	RCC_CR &= ~RCC_CR_PLLI2SON;
	while (RCC_CR &RCC_CR_PLLI2SON)
		;

	RCC_PLLI2SCFGR = /*N*/(271 << 6) | /*R*/(2 << 28); // Fs = 44100   Fclk = 64 * Fs = 2822400

	RCC_CR |= RCC_CR_PLLI2SON;
	while (!(RCC_CR & RCC_CR_PLLI2SON))
		;

	SPI2_CR1 = SPI2_CR2 = SPI2_I2SCFGR/* = SPI3ext_I2SCFGR*/= 0;
	SPI2_I2SCFGR = SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_1
			| SPI_I2SCFGR_I2SCFG_1 | SPI_I2SCFGR_I2SMOD;
	//SPI3ext_I2SCFGR = SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_1 | SPI_I2SCFGR_I2SCFG_0 | SPI_I2SCFGR_I2SMOD;
	SPI2_CR2 |= SPI_CR2_TXDMAEN;
	//SPI3ext_CR2 |= SPI_CR2_RXDMAEN;
	SPI2_I2SPR = 6 | (0 << 8) | (1 << 9); // Fs = 44100
	//SPI3ext_I2SPR = 6 | (0 << 8) | (1 << 9);
	SPI2_I2SCFGR |= SPI_I2SCFGR_I2SE;

	SPI3_CR1 = SPI3_CR2 = SPI3_I2SCFGR/* = SPI3ext_I2SCFGR*/= 0;
	SPI3_I2SCFGR = SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_1
			| SPI_I2SCFGR_I2SMOD;
	//SPI3ext_I2SCFGR = SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_1 | SPI_I2SCFGR_I2SCFG_0 | SPI_I2SCFGR_I2SMOD;
	SPI3_CR2 |= SPI_CR2_TXDMAEN;
	//SPI3ext_CR2 |= SPI_CR2_RXDMAEN;
	SPI3_I2SPR = 6 | (0 << 8); // Fs = 44100
	//SPI3ext_I2SPR = 6 | (0 << 8) | (1 << 9);
	SPI3_I2SCFGR |= SPI_I2SCFGR_I2SE;

	nvic_enable_irq (NVIC_DMA1_STREAM4_IRQ);
	nvic_set_priority(NVIC_DMA1_STREAM4_IRQ,
			configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);

	dma_stream_reset(DMA1, DMA_STREAM4);
	dma_set_priority(DMA1, DMA_STREAM4, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA1, DMA_STREAM4, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM4, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM4);
	dma_disable_peripheral_increment_mode(DMA1, DMA_STREAM4);
	dma_set_transfer_mode(DMA1, DMA_STREAM4, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA1, DMA_STREAM4, (uint32_t) &SPI2_DR);
	dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) &dac_sample);
	dma_enable_circular_mode(DMA1, DMA_STREAM4);
	dma_set_number_of_data(DMA1, DMA_STREAM4, 4);
	dma_channel_select(DMA1, DMA_STREAM4, DMA_SxCR_CHSEL_0);
	dma_enable_stream(DMA1, DMA_STREAM4);

	dma_stream_reset(DMA1, DMA_STREAM7);
	dma_set_priority(DMA1, DMA_STREAM7, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA1, DMA_STREAM7, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM7, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM7);
	dma_disable_peripheral_increment_mode(DMA1, DMA_STREAM7);
	dma_set_transfer_mode(DMA1, DMA_STREAM7, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA1, DMA_STREAM7, (uint32_t) &SPI3_DR);
	dma_set_memory_address(DMA1, DMA_STREAM7, (uint32_t) &dac_sample1);
	dma_enable_circular_mode(DMA1, DMA_STREAM7);
	dma_set_number_of_data(DMA1, DMA_STREAM7, 4);
	dma_channel_select(DMA1, DMA_STREAM7, DMA_SxCR_CHSEL_0);
	dma_enable_stream(DMA1, DMA_STREAM7);

	for (int i = 0; i < 0xff; i++)
		NOP();

	nvic_enable_irq (NVIC_DMA2_STREAM0_IRQ);
	nvic_set_priority(NVIC_DMA2_STREAM0_IRQ,
			configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);

	spi_reset (SPI1);
	spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_256,
			SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_2,
			SPI_CR1_DFF_16BIT, SPI_CR1_MSBFIRST);
	spi_set_full_duplex_mode(SPI1);
	spi_enable(SPI1);

	for (uint32_t i = 0; i < 0x3fffff; i++)
		NOP();

	gpio_set(GPIOC, GPIO7);
	for (int i = 0; i < 0xfffff; i++)
		NOP();
	const uint16_t fff[2] =
	{ 0x2087, 0xa103 };
	for (int i = 0; i < 1; i++)
	{
		gpio_clear(GPIOA, GPIO4);
		while (!(SPI1_SR & SPI_SR_TXE))
			;
		SPI1_DR = fff[i];
		while (!(SPI1_SR & SPI_SR_TXE))
			;
		while (SPI1_SR &SPI_SR_BSY)
			;
		gpio_set(GPIOA, GPIO4);
		for (int i = 0; i < 0xff; i++)
			NOP();
	}

	spi_enable_rx_dma(SPI1);
	spi_enable_tx_dma(SPI1);

	dma_stream_reset(DMA2, DMA_STREAM0);
	dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
	dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM0);
	dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
	dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t) &SPI1_DR);
	dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t) &key_reg_in);
	dma_enable_circular_mode(DMA2, DMA_STREAM0);
	dma_set_number_of_data(DMA2, DMA_STREAM0, 2);
	dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_3);
	dma_enable_stream(DMA2, DMA_STREAM0);

	dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);

	for (uint16_t i = 0; i < 0xffff; i++)
		NOP();

	dma_stream_reset(DMA2, DMA_STREAM5);
	dma_set_priority(DMA2, DMA_STREAM5, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA2, DMA_STREAM5, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM5, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA2, DMA_STREAM5);
	dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM5);
	dma_set_transfer_mode(DMA2, DMA_STREAM5, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA2, DMA_STREAM5, (uint32_t) &SPI1_DR);
	dma_set_memory_address(DMA2, DMA_STREAM5, (uint32_t) &key_reg_out);
	dma_set_number_of_data(DMA2, DMA_STREAM5, 2);
	dma_channel_select(DMA2, DMA_STREAM5, DMA_SxCR_CHSEL_3);
	dma_enable_stream(DMA2, DMA_STREAM5);
	//---------------------------------------------------------------------------------------------------------

	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
#define clock_pll       84000000
#define baudrate_midi   31250
	USART1_BRR = (uint16_t) (((2 * clock_pll) + baudrate_midi)
			/ (2 * baudrate_midi));
	usart_enable_rx_interrupt (USART1);
	nvic_enable_irq (NVIC_USART1_IRQ);
	nvic_set_priority(NVIC_USART1_IRQ,
			configMAX_SYSCALL_INTERRUPT_PRIORITY + 16);
	usart_enable_tx_dma(USART1);
	usart_enable(USART1);

	for (int i = 0; i < 0xff; i++)
		NOP();
	//---------------------------------------------------------bypass unselect pin---------------------------------------------
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO8 | GPIO11 | GPIO12 /*| GPIO13 | GPIO14*/);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
			GPIO6 | GPIO8 | GPIO11 | GPIO12 | GPIO13 | GPIO14);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO1);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
			GPIO0 | GPIO1);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO13);

	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);

}
//---------------------------------------------------------------------------

void i2s_dma_interrupt_enable()
{
	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
}
void i2s_dma_interrupt_disable()
{
	dma_disable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
}

size_t sound_point = 0;
uint16_t msec_tik = 0;
volatile uint32_t samp_point;
volatile uint32_t sample_pos = 0;

volatile uint32_t metronom_int;
uint16_t metronom_counter = 0;
uint8_t metronom_fl = 0;
uint8_t metronom_start = 0;
uint16_t temp_counter = 0;
uint32_t tap_temp;
uint32_t tap_temp1;
uint32_t tap_temp2;

volatile size_t abs_sample_count = 0;

extern "C" void DMA1_Stream4_IRQHandler()
{
	dma_clear_interrupt_flags(DMA1, DMA_STREAM4, DMA_TCIF);

	dac_sample.left = dac_sample1.left = 0;
	dac_sample.right = dac_sample1.right = 0;

	if(player.state() == Player::PLAYER_PLAYING)//if(play_fl)
	{
		dac_sample.left = sound_buff[sound_point].left;
		dac_sample.right = sound_buff[sound_point].right;

		if (sample_pos < click_size)
		{
				dac_sample1.left = click_buff[sound_point].left;
				dac_sample1.right = click_buff[sound_point].right;
		}
		sample_pos = FsStreamTask->pos();

		if (count_up >= song_size)
		{
			player.stopPlay();
			if(FsStreamTask->selectedSong.playNext)
			{
				if(currentMenu)
				{
					menuPlayer->requestPlayNext();
				}
			}
			else
			{
				if (!sys_param[auto_next_track])
					menuPlayer->keyStop();
				else
					menuPlayer->requestPlayNext();
			}
			CSTask->Give();
		}
		else
		{
			if (sound_point == 0)
				FsStreamTask->data_notify(&second_target);

			if (sound_point == Player::wav_buff_size / 2)
				FsStreamTask->data_notify(&first_target);
		}

		msec_tik++;
		if (msec_tik == 4410)
		{
			count_up++;
			count_down--;

			if (currentMenu->menuType() == MENU_PLAYER)
			{
				if (sys_param[direction_counter])
					DisplayTask->Sec_Print(count_down);
				else
					DisplayTask->Sec_Print(count_up);
			}
			msec_tik = 0;
		}

		if (sys_param[loop_points] && menuPlayer->loopModeActive())
		{
			if (play_point2 > play_point1)
				if ((count_up * 4410) >= play_point2)
				{
					key_ind = key_return;
					CSTask->Give();
				}
			if (play_point1 > play_point2)
				if ((count_up * 4410) >= play_point1)
				{
					key_ind = key_forward;
					CSTask->Give();
				}
		}

		FsStreamTask->MidiEventProcess();

		//инкремент с заворачиванием индекса
		sound_point++;
		sound_point &= Player::wav_buff_size - 1;
	}

	if(player.state() == Player::PLAYER_LOADING_SONG)
	{
		player.songInitiated();
	}

	if (metronom_start)
	{
		if (metronom_fl)
		{
			//dac_sample1.left = metronom_cod[metronom_counter];
			dac_sample1.right = metronom_cod[metronom_counter];
			metronom_counter++;
			if (metronom_counter == 3935)
			{
				metronom_fl = 0;
				metronom_counter = 0;
			}
		}
		if (!temp_counter++)
			metronom_fl = 1;
		else
		{
			if (temp_counter == metronom_int)
				temp_counter = 0;
		}
	}

	if (tap_temp < 132300)
		tap_temp++;
	if (tap_temp1 < 150001)
		tap_temp1++;
	if (tap_temp2 < 170001)
		tap_temp2++;
}

extern "C" void EXTI4_IRQHandler()
{
	nvic_clear_pending_irq (NVIC_EXTI4_IRQ);
	if (!lock_fl)
	{
		if (drebezg(EXTI4) == 1)
		{
			if (GPIOC_IDR &GPIO5)
				encoder_state = 1;
			else
				encoder_state = 2;
			encoder_state1 = 1;
			CSTask->Give();
		}
	}
	exti_reset_request (EXTI4);
}
extern "C" void DMA2_Stream0_IRQHandler()
{
	dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
	if (!gpio_get(GPIOC, GPIO0))
		key_val = 1;
	else
	{
		if (!gpio_get(GPIOB, GPIO11))
			key_val = 2;
		else
		{
			if (!gpio_get(GPIOB, GPIO13))
				key_val = 3;
			else
			{
				if (!gpio_get(GPIOB, GPIO14))
					key_val = 4;
				else
					key_val = key_reg_in[1] >> 8;
			}
		}
	}
	gpio_clear(GPIOC, GPIO15);
	TIM4_CR1 |= TIM_CR1_CEN;
}

extern "C" void TIM4_IRQHandler()
{
	timer_clear_flag(TIM4, TIM_SR_UIF);
	gpio_set(GPIOC, GPIO15);
	gpio_set(GPIOC, GPIO14);
	dma_clear_interrupt_flags(DMA2, DMA_STREAM5, DMA_TCIF);
	dma_disable_stream(DMA2, DMA_STREAM5);
	gpio_clear(GPIOC, GPIO14);
	dma_set_number_of_data(DMA2, DMA_STREAM5, 2);
	dma_enable_stream(DMA2, DMA_STREAM5);
}
