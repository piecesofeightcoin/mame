// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Nascom Advanced Video Card

***************************************************************************/

#include "emu.h"
#include "avc.h"

#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NASCOM_AVC, nascom_avc_device, "nascom_avc", "Nascom Advanced Video Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nascom_avc_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16250000, 1024, 0, 768, 320, 0, 256);
	screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	MC6845(config, m_crtc, XTAL(16'000'000) / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(6);
	m_crtc->set_update_row_callback(FUNC(nascom_avc_device::crtc_update_row));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nascom_avc_device - constructor
//-------------------------------------------------

nascom_avc_device::nascom_avc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NASCOM_AVC, tag, owner, clock),
	device_nasbus_card_interface(mconfig, *this),
	m_crtc(*this, "mc6845"),
	m_palette(*this, "palette"),
	m_control(0x80)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nascom_avc_device::device_start()
{
	// allocate memory
	m_r_ram.resize(0x4000);
	m_g_ram.resize(0x4000);
	m_b_ram.resize(0x4000);

	save_item(NAME(m_r_ram));
	save_item(NAME(m_g_ram));
	save_item(NAME(m_b_ram));
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nascom_avc_device::device_reset()
{
	io_space().install_write_handler(0xb0, 0xb0, write8smo_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	io_space().install_readwrite_handler(0xb1, 0xb1, read8smo_delegate(*m_crtc, FUNC(mc6845_device::register_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	io_space().install_write_handler(0xb2, 0xb2, write8smo_delegate(*this, FUNC(nascom_avc_device::control_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW( nascom_avc_device::crtc_update_row )
{
	offs_t base_addr = (ma << 1 | ra << 6) + 2; // y * 64 + 2

	for (int x = 0; x < x_count * 6; x++)
	{
		// addr of source byte
		offs_t addr = base_addr + (x / 16);

		// msb first
		int bl = 7 - ((x / 2) & 7);
		int bh = 7 - ((x / 1) & 7);

		int r, g, b;

		// double density)
		if (BIT(m_control, 3))
		{
			// red disabled, blue low density, red/green combined to green
			r = 0;
			b = BIT(m_b_ram[addr], bl);
			g = (x & 8) ? BIT(m_r_ram[addr], bh) : BIT(m_g_ram[addr], bh);
		}
		else
		{
			// rgb color
			r = BIT(m_r_ram[addr], bl);
			g = BIT(m_g_ram[addr], bl);
			b = BIT(m_b_ram[addr], bl);
		}

		// plot the pixel
		bitmap.pix32(y, x) = m_palette->pen_color((b << 2) | (g << 1) | (r << 0));
	}
}

void nascom_avc_device::control_w(uint8_t data)
{
	logerror("nascom_avc_device::control_w: 0x%02x\n", data);

	// page video ram in?
	if (((m_control & 0x07) == 0) && (data & 0x07))
	{
		ram_disable_w(0);
		program_space().install_readwrite_handler(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(nascom_avc_device::vram_r)), write8sm_delegate(*this, FUNC(nascom_avc_device::vram_w)));
	}
	else if ((data & 0x07) == 0)
	{
		program_space().unmap_readwrite(0x8000, 0xbfff);
		ram_disable_w(1);
	}

	m_control = data;
}

uint8_t nascom_avc_device::vram_r(offs_t offset)
{
	// manual says only one plane can be read, i assume this is the order
	if (BIT(m_control, 0)) return m_r_ram[offset];
	if (BIT(m_control, 1)) return m_g_ram[offset];
	if (BIT(m_control, 2)) return m_b_ram[offset];

	// should never happen
	return 0xff;
}

void nascom_avc_device::vram_w(offs_t offset, uint8_t data)
{
	// all planes can be written at the same time
	if (BIT(m_control, 0)) m_r_ram[offset] = data;
	if (BIT(m_control, 1)) m_g_ram[offset] = data;
	if (BIT(m_control, 2)) m_b_ram[offset] = data;
}
