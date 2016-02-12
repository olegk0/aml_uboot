#define DDR_DEBUG 1

#if defined(CONFIG_AML_EXT_PGM)

void print_ddr_channel(void){}
void print_ddr_size(unsigned int size){}
void print_ddr_mode(void){}

#else
void print_ddr_channel(void);
void print_ddr_size(unsigned int size);
void print_ddr_mode(void);
#if defined(CONFIG_DDR_CHANNEL_AUTO_DETECT) || defined(CONFIG_DDR_MODE_AUTO_DETECT) || defined(CONFIG_DDR_SIZE_AUTO_DETECT)
unsigned int set_dmc_row(unsigned int dmc, unsigned int channel0, unsigned int channel1);
unsigned int get_ddr_channel(unsigned int dmc);
unsigned int get_ddr_channel_num(unsigned int dmc);
unsigned int get_ddr_bus_width(unsigned int dmc);
unsigned int get_ddr_bank_set(unsigned int dmc);
unsigned int dtar_reset(struct ddr_set * timing_reg);
#endif
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
int ddr_mode_auto_detect(struct ddr_set * timing_reg);
#endif
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
void ddr_size_auto_detect(struct ddr_set * timing_reg);
#endif

void print_ddr_channel(void){
	serial_puts("DDR channel: ");
	unsigned int channel_set = 0;
	if(IS_MESON_M8M2_CPU){/*m8m2*/
		channel_set = readl(P_DMC_DDR_CTRL);
		if(0x3==((channel_set>>26)&0x3)){/*one channel*/
			if(0x1==((channel_set>>24)&0x1))/*ddr 0 only*/
//IAM				serial_puts("DDR 0 only");
				serial_puts("DDR 1 only");
    		else
				serial_puts("DDR 0 only");
//				serial_puts("DDR 1 only");
		}
		else
			serial_puts("DDR 0 + DDR 1");
	}
	else{
		channel_set = readl(P_MMC_DDR_CTRL);
		if(0x3==(channel_set>>24)&0x3)/*ddr 0 only*/
			serial_puts("DDR 0 only");
		else if(0x1==(channel_set>>24)&0x3)/*ddr 1 only*/
			serial_puts("DDR 1 only");
		else /*others, 2 channels*/
			serial_puts("DDR 0 + DDR 1");
	}
#ifdef CONFIG_DDR_CHANNEL_AUTO_DETECT
	serial_puts(" (auto)\n");
#else
	serial_puts("\n");
#endif
}

void print_ddr_size(unsigned int size){
	//serial_puts("DDR size: ");
	unsigned int mem_size = size >> 20; //MB
	(mem_size) >= 1024 ? serial_put_dec(mem_size >> 10) : serial_put_dec(mem_size);
	(((mem_size>>9)&0x1)&&((mem_size)>=1024)) ? serial_puts(".5") : 0;
	(mem_size) >= 1024 ? serial_puts("GB"):serial_puts("MB");
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
	serial_puts("(auto)");
//#else
//	serial_puts("\n");
#endif
}

void print_ddr_mode(void){
	serial_puts("DDR mode: ");
	switch(cfg_ddr_mode){
		case CFG_DDR_BUS_WIDTH_NOT_SET:
			serial_puts("Not Set"); break;
		case CFG_DDR_BUS_WIDTH_32BIT:
			serial_puts("32 bit mode"); break;
		case CFG_DDR_BUS_WIDTH_16BIT_LANE01:
			serial_puts("16 bit mode lane0+1"); break;
	}
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
	serial_puts(" (auto)\n");
#else
	serial_puts("\n");
#endif
}
#endif

#if defined(CONFIG_DDR_CHANNEL_AUTO_DETECT) || defined(CONFIG_DDR_MODE_AUTO_DETECT) || defined(CONFIG_DDR_SIZE_AUTO_DETECT)
unsigned int set_dmc_row(unsigned int dmc, unsigned int channel0, unsigned int channel1){
	dmc = dmc & ( 0xfffff3f3 ); /*clear row size bit*/
	dmc = dmc | ((channel0 & 0x3) << 2) | ((channel1 & 0x3) << 10);
	return dmc;
}
#if 0
unsigned int get_ddr_channel(unsigned int dmc){/*return 1 channel or 2 channels*/
	/*return b'01: ddr0 only, b'10: ddr1 only, b'11: ddr0+ddr1*/
	if(IS_MESON_M8M2_CPU){/*m8m2*/
		if(((dmc>>26)&0x3) == 0x3){/*one channe;*/
			if(((dmc >> 24) & 0x1))
				return 0x2; /*b'10*/ /*ddr 1 only*/
			else
				return 0x1; /*b'01*/ /*ddr 0 only*/
		}
		else
			return 0x3; /*b'11*/ /*ddr 0 + ddr 1*/
	}
	else{/*m8*/
		if((dmc >> 24) & 0x1){/*one channel*/
			if((dmc >> 24) & 0x1)
				return 0x1; /*b'01*/ /*ddr 0 only*/
			else
				return 0x2; /*b'10*/ /*ddr 1 only*/
		}
		else
			return 0x3; /*b'11*/ /*ddr 0 + ddr 1*/
	}
}
#endif
unsigned int get_ddr_channel_num(unsigned int dmc){
	if(IS_MESON_M8M2_CPU){
		unsigned int chl_switch = (dmc>>26)&0x3;
		unsigned int sub_switch = (dmc>>24)&0x1;
		if(0x0 == chl_switch)/*2 channel switch bit12*/
			return CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12;
		else if(0x1 == chl_switch)/*2 channel switch bit8*/
			return CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8;
		else if(0x2 == chl_switch){
			if(0 == sub_switch)
				return CFG_DDR_TWO_CHANNEL_DDR0_LOW;
			else
				return CFG_DDR_TWO_CHANNEL_DDR1_LOW;
		}
		else if(0x3 == chl_switch){
			if(0 == sub_switch)
				return CFG_DDR_ONE_CHANNEL_DDR0_ONLY;
			else
				return CFG_DDR_ONE_CHANNEL_DDR1_ONLY;
		}
	}
	else
		return (dmc>>24)&0x3;
}

unsigned int get_ddr_bus_width(unsigned int dmc){
	if(IS_MESON_M8M2_CPU){/*m8m2*/
		if(((dmc>>7)&0x1) == 0x1)
			return 0x1; /*16bit mode*/
	}
	return 0x0; /*other case, 32bit mode*/
}

unsigned int get_ddr_bank_set(unsigned int dmc){
	return ((dmc>>5)&0x1);
}

unsigned int dtar_reset(struct ddr_set * timing_reg){
	/*set correct row_bits and dtar address back to reg*/
	unsigned int ddr0_row_size, ddr0_row_bits, ddr1_row_size, ddr1_row_bits;
	unsigned int bus_width = get_ddr_bus_width(timing_reg->t_mmc_ddr_ctrl);
	unsigned int ddr_channel_num = get_ddr_channel_num(timing_reg->t_mmc_ddr_ctrl);
	unsigned int ddr0_size, ddr1_size;
	ddr0_size = (ddr_channel_num==CFG_DDR_ONE_CHANNEL_DDR0_ONLY)||(ddr_channel_num==CFG_DDR_ONE_CHANNEL_DDR1_ONLY)?(timing_reg->phy_memory_size):(timing_reg->phy_memory_size>>1);
	ddr1_size = (ddr_channel_num==CFG_DDR_ONE_CHANNEL_DDR0_ONLY)||(ddr_channel_num==CFG_DDR_ONE_CHANNEL_DDR1_ONLY)?(timing_reg->phy_memory_size):(timing_reg->phy_memory_size>>1);
	ddr0_row_bits = GET_DDR_ROW_BITS(ddr0_size, bus_width);
	ddr0_row_size = GET_DDR_ROW_SIZE(ddr0_row_bits);
	ddr1_row_bits = GET_DDR_ROW_BITS(ddr1_size, bus_width);
	ddr1_row_size = GET_DDR_ROW_SIZE(ddr1_row_bits);
	timing_reg->t_mmc_ddr_ctrl = set_dmc_row(timing_reg->t_mmc_ddr_ctrl, ddr0_row_size, ddr1_row_size);
	unsigned int dtar_addr_0_offset, dtar_addr_1_offset, dtar_addr_0, dtar_addr_1;
	unsigned int ddr_channel_switch;
	switch(ddr_channel_num){
		case CFG_DDR_TWO_CHANNEL_SWITCH_BIT_12:
			ddr_channel_switch = 0x0;
			dtar_addr_0_offset = 0; dtar_addr_1_offset = 1<<12; break;
		case CFG_DDR_TWO_CHANNEL_SWITCH_BIT_8:
			ddr_channel_switch = 0x1;
			dtar_addr_0_offset = 0; dtar_addr_1_offset = 1<<8; break;
		case CFG_DDR_TWO_CHANNEL_SWITCH_BIT_30:
			if(timing_reg->phy_memory_size<=0x40000000){
				serial_puts("DDR size lower than 1GB, doesn't support 2 channel switch bit30!\n");
				AML_WATCH_DOG_START();
				while(1);
			}
			else{
				dtar_addr_0_offset = 0; dtar_addr_1_offset = 0x40000000;
			}
			break;
		case CFG_DDR_TWO_CHANNEL_DDR0_LOW:
			ddr_channel_switch = 0x2;
			dtar_addr_0_offset = 0; dtar_addr_1_offset = ddr0_size; break;
		case CFG_DDR_TWO_CHANNEL_DDR1_LOW:
			ddr_channel_switch = 0x2;
			dtar_addr_0_offset = ddr0_size; dtar_addr_1_offset = 0-dtar_addr_0_offset;break;
		case CFG_DDR_ONE_CHANNEL_DDR0_ONLY:
		case CFG_DDR_ONE_CHANNEL_DDR1_ONLY:
			ddr_channel_switch = 0x3;
			dtar_addr_0_offset = 0; dtar_addr_1_offset = 0; break;
		default: break;
	}
#if (DDR_DEBUG)
	serial_puts("	ddr_channel_num: ");
	serial_put_hex(ddr_channel_num, 32);
	serial_puts("\n");
	serial_puts("	ddr0_size: ");
	serial_put_hex(ddr0_size, 32);
	serial_puts("\n");
	serial_puts("	ddr1_size: ");
	serial_put_hex(ddr1_size, 32);
	serial_puts("\n");
	serial_puts("	dtar_addr_0_offset: ");
	serial_put_hex(dtar_addr_0_offset, 32);
	serial_puts("\n");
	serial_puts("	dtar_addr_1_offset: ");
	serial_put_hex(dtar_addr_1_offset, 32);
	serial_puts("\n");
#endif
	dtar_addr_0 = CONFIG_DDR_DTAR_ADDR_BASE + dtar_addr_0_offset;
	dtar_addr_1 = dtar_addr_0 + dtar_addr_1_offset;
#if (DDR_DEBUG)
	serial_puts("re-calculate dtar.\n");
	serial_puts("	dtar0_addr: ");
	serial_put_hex(dtar_addr_0, 32);
	serial_puts("\n");
	serial_puts("	dtar1_addr: ");
	serial_put_hex(dtar_addr_1, 32);
	serial_puts("\n");
#endif

	if(IS_MESON_M8M2_CPU){
		writel(timing_reg->t_mmc_ddr_ctrl, P_DMC_DDR_CTRL);
		timing_reg->t_pub0_dtar	= ((0x0 + M8M2_DDR_DTAR_DTCOL_GET(dtar_addr_0, CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width))| \
			(M8M2_DDR_DTAR_DTROW_GET(dtar_addr_0, ddr0_row_bits,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width) <<12)| \
			(M8M2_DDR_DTAR_BANK_GET(dtar_addr_0, ddr0_row_bits,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width) << 28));
		timing_reg->t_pub1_dtar	= ((0x0 + M8M2_DDR_DTAR_DTCOL_GET(dtar_addr_1, CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width))| \
			(M8M2_DDR_DTAR_DTROW_GET(dtar_addr_1, ddr1_row_bits,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width) <<12)| \
			(M8M2_DDR_DTAR_BANK_GET(dtar_addr_1, ddr1_row_bits,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_switch,bus_width) << 28));
	}
	else{
		writel(timing_reg->t_mmc_ddr_ctrl, P_MMC_DDR_CTRL);
		timing_reg->t_pub0_dtar = ((0x0 + M8_DDR_DTAR_DTCOL_GET(dtar_addr_0,CONFIG_DDR0_COL_BITS))| \
			(M8_DDR_DTAR_DTROW_GET(dtar_addr_0,ddr0_row_bits,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_num) << 12)| \
			(M8_DDR_DTAR_BANK_GET(dtar_addr_0,ddr0_row_bits,CONFIG_DDR0_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_num) << 28));
		timing_reg->t_pub1_dtar = ((0x0 + M8_DDR_DTAR_DTCOL_GET(dtar_addr_1,CONFIG_DDR1_COL_BITS))| \
			(M8_DDR_DTAR_DTROW_GET(dtar_addr_1,ddr1_row_bits,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_num) << 12)| \
			(M8_DDR_DTAR_BANK_GET(dtar_addr_1,ddr1_row_bits,CONFIG_DDR1_COL_BITS,CONFIG_DDR_BANK_SET,ddr_channel_num) << 28));
	}

	//enable pctl clk
	writel(readl(P_DDR0_CLK_CTRL) | 0x1, P_DDR0_CLK_CTRL);
	writel(readl(P_DDR1_CLK_CTRL) | 0x1, P_DDR1_CLK_CTRL);
	__udelay(1000);

	writel((0x0  + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR0);
	writel((0x08 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR1);
	writel((0x10 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR2);
	writel((0x18 + timing_reg->t_pub0_dtar), P_DDR0_PUB_DTAR3);
	__udelay(10);
	writel((0x0  + timing_reg->t_pub1_dtar), P_DDR1_PUB_DTAR0);
	writel((0x08 + timing_reg->t_pub1_dtar), P_DDR1_PUB_DTAR1);
	writel((0x10 + timing_reg->t_pub1_dtar), P_DDR1_PUB_DTAR2);
	writel((0x18 + timing_reg->t_pub1_dtar), P_DDR1_PUB_DTAR3);
	__udelay(10);

	//disable pctl clk
	writel(readl(P_DDR0_CLK_CTRL) & (~1), P_DDR0_CLK_CTRL);
	writel(readl(P_DDR1_CLK_CTRL) & (~1), P_DDR1_CLK_CTRL);
}
#endif

/*Following code is for DDR MODE AUTO DETECT*/
#ifdef CONFIG_DDR_MODE_AUTO_DETECT
static inline unsigned ddr_init(struct ddr_set * timing_reg);

int ddr_mode_auto_detect(struct ddr_set * timing_reg){
	int ret = 0;
#ifdef CONFIG_DDR_MODE_AUTO_DETECT_SKIP_32BIT
	int try_times = 2;
#else
	int try_times = 1;
#endif
	for(; try_times <= 2; try_times++){
		serial_puts("DDR mode test:");
		serial_put_hex(try_times, 2);
		serial_puts("\n");
		cfg_ddr_mode = try_times;
		ret = ddr_init(timing_reg);
		if(!ret){
			print_ddr_mode();
			return 0;
		}
	}
	serial_puts("\nAll ddr mode test failed, reset...\n");
	__udelay(10000); 
	AML_WATCH_DOG_START();
}
#endif

/*Following code is for DDR SIZE AUTO DETECT*/
#define DDR_SIZE_AUTO_DETECT_DEBUG 0
#ifdef CONFIG_DDR_SIZE_AUTO_DETECT
#define DDR_SIZE_AUTO_DETECT_PATTERN	0xABCDEF00
void ddr_size_auto_detect(struct ddr_set * timing_reg){
/*
  memory size auto detect function
  please make sure ddr capacity of each channel are equal,
  that means doesn't support 1.5G 3G etc auto detection. 
*/
    unsigned int dmc_reg_setting = timing_reg->t_mmc_ddr_ctrl;//readl(P_MMC_DDR_CTRL);
//IAM add for exception avoid while test
    dmc_reg_setting |= (1<<12)|(1<<4);//0x0c012622

	/*set max row size, then use "ROW ADDRESS MASK" to detect memory size*/
	dmc_reg_setting = set_dmc_row(dmc_reg_setting, 0x0, 0x0);
	
	if(IS_MESON_M8M2_CPU)
		writel(dmc_reg_setting, P_DMC_DDR_CTRL);
	else
		writel(dmc_reg_setting, P_MMC_DDR_CTRL);

	/*start detection*/
	unsigned int row_mask_bit_offset = 25; /*just start from a little row size*/
	unsigned int cur_mask_addr = 0;
	int loop = 0;
#if (DDR_DEBUG)
    serial_puts(" ddr_ctrl: ");
    serial_put_hex(dmc_reg_setting, 32);
    serial_puts("\n");
#endif

	for(loop = 0; ; loop++){
		cur_mask_addr = (1 << (row_mask_bit_offset + loop));
		writel(0, PHYS_MEMORY_START);
		writel(DDR_SIZE_AUTO_DETECT_PATTERN, cur_mask_addr);
#if (DDR_DEBUG)
		serial_puts("	write address: ");
		serial_put_hex(cur_mask_addr, 32);
		serial_puts("	with 0xABCDEF00\n	read: ");
        serial_put_hex(readl(cur_mask_addr), 32);
        serial_puts("\n	read(0): ");
		serial_put_hex(readl(0), 32);
		serial_puts("\n");
#endif
		asm volatile("DSB"); /*sync ddr data*/
		if(readl(PHYS_MEMORY_START) == DDR_SIZE_AUTO_DETECT_PATTERN){
#if (DDR_DEBUG)
			serial_puts("	find match size: ");
			serial_put_hex(cur_mask_addr, 32);
			serial_puts("\n");
#endif
			break;
		}
		if(cur_mask_addr>=0x80000000){
			serial_puts("DDR size auto detect failed! Reset...\n: ");
			AML_WATCH_DOG_START();
			while(1);
		}
	}

//IAM restore
//    timing_reg->t_mmc_ddr_ctrl = (3<<26)|(0<<24)|(1<<16)|(0<<7)|(1<<5)|(0<<4)|(0<<2)|(2<<0);//for my device
    dmc_reg_setting = timing_reg->t_mmc_ddr_ctrl;
    
	if(IS_MESON_M8M2_CPU)
		writel(dmc_reg_setting, P_DMC_DDR_CTRL);
	else
		writel(dmc_reg_setting, P_MMC_DDR_CTRL);

#ifdef CONFIG_DDR_MEM_TEST_ONLY
    AML_WATCH_DOG_DISABLE();
    
#define DDR_MEM_TEST_PATTERN 0xaaaaaaaa
#define DDR_MEM_TEST_MSKIP 0x800000
cur_mask_addr = 0x40000000;
    unsigned int taddr = 0,taddr1, tdt, rdt,ts;
/*    serial_puts("++++++++++++++Address map test+++++++++++++++++++\n");
    while(taddr < cur_mask_addr){//clear
        writel(0, taddr);
        if(taddr < 4) taddr = 4;
        else taddr = taddr << 1;
    }
    taddr = 0;
    while(taddr < cur_mask_addr){//clear
        writel(0xffffffff, taddr);
        asm volatile("DSB");//?
        serial_puts("	Test, ADDR:0x");
        serial_put_hex(taddr, 32);
        serial_puts("\n");

        taddr1 = 0;
        while(taddr1 < cur_mask_addr){//check
            if(taddr != taddr1){
                rdt = readl(taddr1);
                if(rdt){
                    serial_puts("	Error, ADDR:0x");
                    serial_put_hex(taddr, 32);
                    serial_puts(" - ADDR1:0x");
                    serial_put_hex(taddr1, 32);
                    serial_puts(" - read:0x");
                    serial_put_hex(rdt, 32);
                    serial_puts("\n");
                }
            }
            if(taddr1 < 4) taddr1 = 4;
            else taddr1 = taddr1 << 1;
        }
        writel(0, taddr);
        if(taddr < 4) taddr = 4;
        else taddr = taddr << 1;
    }
  */  
    serial_puts("++++++++++++++Full Read/Write test+++++++++++++++++++\n");
#if 1 // long time test

    taddr = 4;
    ts=1;
    serial_puts("--- Stage 1: Fill memory\n");
    writel(0, 0);
    while(taddr < cur_mask_addr){
        writel(taddr, taddr);
       
        if((taddr & DDR_MEM_TEST_MSKIP) != ts)
        {
            ts = taddr & DDR_MEM_TEST_MSKIP;
            serial_puts("	Write ADDR:0x");
            serial_put_hex(taddr, 32);
            serial_puts("\n");
        }
        taddr += 4;
    }
    serial_puts("--- Stage 2: Read and check\n");

    taddr = 4;
    ts=1;
    rdt = readl(0);
    if(rdt){
        serial_puts("	Error, ADDR(0):0x");
        serial_put_hex(rdt, 32);
        serial_puts("\n");
    }

    while(taddr < cur_mask_addr){
        rdt = readl(taddr);

        if(rdt != taddr){
            serial_puts("	Error, ADDR:0x");
            serial_put_hex(taddr, 32);
            serial_puts("\n	write:0x");
            serial_put_hex(taddr, 32);
            serial_puts(" - read:0x");
            serial_put_hex(rdt, 32);
            serial_puts("\n");
        }
        
        if((taddr & DDR_MEM_TEST_MSKIP) != ts)
        {
            ts = taddr & DDR_MEM_TEST_MSKIP;
            serial_puts("	Read, ADDR:0x");
            serial_put_hex(taddr, 32);
            serial_puts("\n");
//            writel(0, P_WATCHDOG_RESET);
        }
        taddr += 4;
    }

    serial_puts("--- Stage 3: Serial Read/Write test\n");
#endif
    taddr = 4;
    ts=1;
    writel(0, 0);
    while(taddr < cur_mask_addr){
        tdt = DDR_MEM_TEST_PATTERN;
        do{
            writel(tdt, taddr);
            asm volatile("DSB");
            rdt = readl(taddr);

            if(tdt != rdt){
                serial_puts("	Error, ADDR:0x");
                serial_put_hex(taddr, 32);
                serial_puts("\n	write:0x");
                serial_put_hex(tdt, 32);
                serial_puts(" - read:0x");
                serial_put_hex(rdt, 32);
                serial_puts("\n");
            }
            tdt = ~tdt;
        }while(tdt != DDR_MEM_TEST_PATTERN);

        rdt = readl(0);
        if(rdt){
            serial_puts("	Error, ADDR(0):0x");
            serial_put_hex(rdt, 32);
            serial_puts("\n");
        }
        
        if((taddr & DDR_MEM_TEST_MSKIP) != ts)
        {
            ts = taddr & DDR_MEM_TEST_MSKIP;
            serial_puts("	Test, ADDR:0x");
            serial_put_hex(taddr, 32);
            serial_puts("\n");
//            writel(0, P_WATCHDOG_RESET);
        }
        taddr += 4;
    }

    serial_puts("Test end\n");
	while(1);
#endif

	/*calculate real memory size*/
	unsigned int mem_size_offset = 0;
	unsigned int bus_width = get_ddr_bus_width(dmc_reg_setting);
	if(IS_MESON_M8M2_CPU)
		mem_size_offset = ((dmc_reg_setting >> 26) & 0x3)==0x2?2:1;
	else
		mem_size_offset = ((dmc_reg_setting >> 24) & 0x3)==0x2?2:1;

//need rework	cur_mask_addr = cur_mask_addr << (mem_size_offset + (((dmc_reg_setting>>5)&0x1)?0:1) - (bus_width?1:0));
#if (DDR_DEBUG)
	serial_puts("	mem_size_offset: ");
	serial_put_hex(mem_size_offset, 32);
	serial_puts("\n");

	serial_puts("	bus_width: ");
	serial_put_hex(bus_width, 32);
	serial_puts("\n");

	serial_puts("	cur_mask_addr: ");
	serial_put_hex(cur_mask_addr, 32);
	serial_puts("\n");
#endif
   
	timing_reg->phy_memory_size = cur_mask_addr;
	dtar_reset(timing_reg);
    
}
#endif