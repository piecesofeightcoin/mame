// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Carlos A. Lozano, Rob Rosenbrock

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

#define MCU_BUFFER_MAX 6

class renegade_state : public driver_device
{
public:
	renegade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_rombank(*this, "rombank"),
		m_adpcmrom(*this, "adpcm")  { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_memory_bank m_rombank;

	required_region_ptr<uint8_t> m_adpcmrom;

	uint32_t m_adpcm_pos;
	uint32_t m_adpcm_end;
	bool m_adpcm_playing;

	bool m_mcu_sim;
	int m_from_main;
	int m_from_mcu;
	int m_main_sent;
	int m_mcu_sent;
	uint8_t m_ddr_a;
	uint8_t m_ddr_b;
	uint8_t m_ddr_c;
	uint8_t m_port_a_out;
	uint8_t m_port_b_out;
	uint8_t m_port_c_out;
	uint8_t m_port_a_in;
	uint8_t m_port_b_in;
	uint8_t m_port_c_in;
	uint8_t m_mcu_buffer[MCU_BUFFER_MAX];
	uint8_t m_mcu_input_size;
	uint8_t m_mcu_output_byte;
	int8_t m_mcu_key;
	int m_mcu_checksum;
	const uint8_t *m_mcu_encrypt_table;
	int m_mcu_encrypt_table_len;
	int32_t m_scrollx;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(mcu_reset_r);
	DECLARE_WRITE8_MEMBER(mcu_w);
	DECLARE_READ8_MEMBER(mcu_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	void mcu_process_command();
	DECLARE_READ8_MEMBER(_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(fg_videoram_w);
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(scroll_lsb_w);
	DECLARE_WRITE8_MEMBER(scroll_msb_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_status_r);
	DECLARE_WRITE8_MEMBER(adpcm_start_w);
	DECLARE_WRITE8_MEMBER(adpcm_addr_w);
	DECLARE_WRITE8_MEMBER(adpcm_stop_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_bg_tilemap_info);
	TILE_GET_INFO_MEMBER(get_fg_tilemap_info);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	DECLARE_DRIVER_INIT(kuniokun);
	DECLARE_DRIVER_INIT(kuniokunb);
	DECLARE_DRIVER_INIT(renegade);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
