/*
********************************************************************************************************************
*                                              usb controller
*
*                              (c) Copyright 2007-2009,
*										All	Rights Reserved
*
* File Name 	: usbc.c
*
* Author 		: daniel
*
* Version 		: 1.0
*
* Date 			: 2009.09.01
*
* Description 	: ������suniiƽ̨��USB������������
*
* History 		:
*
********************************************************************************************************************
*/

#include  "usbc_i.h"


static __u32 usbc_base_address[USBC_MAX_CTL_NUM];       /* usb base address */
static __usbc_otg_t usbc_otg_array[USBC_MAX_OPEN_NUM];  /* usbc �ڲ�ʹ��, ��������USB�˿� */
static __fifo_info_t usbc_info_g;

/*
***********************************************************************************
*                     USBC_GetVbusStatus
*
* Description:
*    ��õ�ǰvbus��״̬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*    ���ص�ǰvbus��״̬
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_GetVbusStatus(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u8 reg_val = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	reg_val = USBC_Readb(USBC_REG_DEVCTL(usbc_otg->base_addr));
	reg_val = reg_val >> USBC_BP_DEVCTL_VBUS;
    switch(reg_val & 0x03){
		case 0x00:
			return USBC_VBUS_STATUS_BELOW_SESSIONEND;
		//break;

		case 0x01:
			return USBC_VBUS_STATUS_ABOVE_SESSIONEND_BELOW_AVALID;
		//break;

		case 0x02:
			return USBC_VBUS_STATUS_ABOVE_AVALID_BELOW_VBUSVALID;
		//break;

		case 0x03:
			return USBC_VBUS_STATUS_ABOVE_VBUSVALID;
		//break;

		default:
			return USBC_VBUS_STATUS_BELOW_SESSIONEND;
	}
}

/*
***********************************************************************************
*                     USBC_OTG_SelectMode
*
* Description:
*    ѡ���豸�����͡���ǰ�豸����device, ������host
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/
void USBC_OTG_SelectMode(__hdle hUSB, __u32 mode)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	if(mode == USBC_OTG_HOST){

	}else{

	}
}

/*
***********************************************************************************
*                     USBC_ReadLenFromFifo
*
* Description:
*    ����fifo���Զ��������ݳ���
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    ep_type  :  input.  ep������, rx �� tx��
* Returns:
*    ���ر���fifo���Զ��������ݳ���
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_ReadLenFromFifo(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			return USBC_Readw(USBC_REG_COUNT0(usbc_otg->base_addr));
		//break;

		case USBC_EP_TYPE_TX:
			return 0;
		//break;

		case USBC_EP_TYPE_RX:
			return USBC_Readw(USBC_REG_RXCOUNT(usbc_otg->base_addr));
		//break;

		default:
			return 0;
	}
}

/*
***********************************************************************************
*                     USBC_WritePacket
*
* Description:
*    ��fifo����д���ݰ�
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    fifo    :  input.  fifo��ַ.
*    cnt     :  input.  д���ݳ���
*    buff    :  input.  ���Ҫд������
*
* Returns:
*    ���سɹ�д��ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_WritePacket(__hdle hUSB, __u32 fifo, __u32 cnt, void *buff)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

	if(usbc_otg == NULL || buff == NULL){
		return 0;
	}

    //--<1>--��������
	buf32 = buff;
	len   = cnt;

	i32 = len >> 2;
	i8  = len & 0x03;

    //--<2>--����4�ֽڵĲ���
	while (i32--){
		USBC_Writel(*buf32++, fifo);
	}

    //--<3>--�����4�ֽڵĲ���
	buf8 = (__u8 *)buf32;
	while (i8--){
		USBC_Writeb(*buf8++, fifo);
	}

	return len;
}

/*
***********************************************************************************
*                     USBC_ReadPacket
*
* Description:
*    ��fifo���������
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    fifo    :  input.  fifo��ַ.
*    cnt     :  input.  д���ݳ���
*    buff    :  input.  ���Ҫ��������
*
* Returns:
*    ���سɹ����ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_ReadPacket(__hdle hUSB, __u32 fifo, __u32 cnt, void *buff)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

    if(usbc_otg == NULL || buff == NULL){
		return 0;
	}

	//--<1>--��������
	buf32 = buff;
	len   = cnt;

    i32 = len >> 2;
	i8  = len & 0x03;

	//--<2>--����4�ֽڵĲ���
	while (i32--){
        *buf32++ = USBC_Readl(fifo);
    }

	//--<3>--�����4�ֽڵĲ���
	buf8 = (__u8 *)buf32;
	while (i8--){
        *buf8++ = USBC_Readb(fifo);
    }

	return len;
}

void USBC_ConfigFIFO_Base(__hdle hUSB, __u32 sram_base, __u32 fifo_mode)
{
    __fifo_info_t *usbc_info = &usbc_info_g;

	usbc_info->port0_fifo_addr = 0x00;
	usbc_info->port0_fifo_size = (8 * 1024);	//8k

	return ;
}

/* ���port fifo����ʼ��ַ */
__u32 USBC_GetPortFifoStartAddr(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	if(usbc_otg->port_num == 0){
		return usbc_info_g.port0_fifo_addr;
	}else if(usbc_otg->port_num == 1){
	    return usbc_info_g.port1_fifo_addr;
	}else {
	    return usbc_info_g.port2_fifo_addr;
	}
}

/* ���port fifo�Ĵ�С */
__u32 USBC_GetPortFifoSize(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	if(usbc_otg->port_num == 0){
		return usbc_info_g.port0_fifo_size;
	}else{
	    return usbc_info_g.port1_fifo_size;
	}
}


/*
***********************************************************************************
*                     USBC_SelectFIFO
*
* Description:
*    ѡ���豸�����͡���ǰ�豸����device, ������host
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    ep_index :  input.  ep�š�����ѡ����Ӧ��fifo
*
* Returns:
*    ����ѡ�е�fifo
*
* note:
*    ��
*
***********************************************************************************
*/
/*
__u32 USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 fifo = 0;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_index){
		case 0:
			fifo = USBC_REG_EPFIFO0(usbc_otg->base_addr);
		break;

		case 1:
			fifo = USBC_REG_EPFIFO1(usbc_otg->base_addr);
		break;

		case 2:
			fifo = USBC_REG_EPFIFO2(usbc_otg->base_addr);
		break;

		case 3:
			fifo = USBC_REG_EPFIFO3(usbc_otg->base_addr);
		break;

		case 4:
			fifo = USBC_REG_EPFIFO4(usbc_otg->base_addr);
		break;

		case 5:
			fifo = USBC_REG_EPFIFO5(usbc_otg->base_addr);
		break;

		default:
			fifo = 0;
	}

	return fifo;
}
*/

__u32 USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	return USBC_REG_EPFIFOx(usbc_otg->base_addr, ep_index);
}

static void __USBC_ConfigFifo_TxEp_Default(__u32 usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_TXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_TXFIFOSZ(usbc_base_addr));
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_TxEp
*
* Description:
*    ����tx ep ��fifo��ַ�ʹ�С��
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    is_double_fifo :  input.  �Ƿ�ʹ��Ӳ��˫fifo
*    fifo_size      :  input.  fifo��С = 2��fifo_size�η�
*    fifo_addr      :  input.  fifo����ʼ��ַ = fifo_addr * 8
*
* Returns:
*    ���سɹ����ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
void __USBC_ConfigFifo_TxEp(__u32 usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __u32 temp = 0;
    __u32 size = 0;   //fifo_size = (size + 3)��2�η�
    __u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>--����sz, ����512����512����
	temp = fifo_size + 511;
	temp &= ~511;  //��511���������
	temp >>= 3;
	temp >>= 1;
	while(temp){
		size++;
		temp >>= 1;
	}

	//--<2>--����addr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_TXFIFOAD(usbc_base_addr));

	//--<4>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_TXFIFOSZ(usbc_base_addr));
	if(is_double_fifo){
		USBC_REG_set_bit_b(USBC_BP_TXFIFOSZ_DPB, USBC_REG_TXFIFOSZ(usbc_base_addr));
	}
}

void __USBC_ConfigFifo_RxEp_Default(__u32 usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_RXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_RXFIFOSZ(usbc_base_addr));
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_RxEp
*
* Description:
*    ����tx ep ��fifo��ַ�ʹ�С��
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    is_double_fifo :  input.  �Ƿ�ʹ��Ӳ��˫fifo
*    fifo_size      :  input.  fifo��С = 2��fifo_size�η�
*    fifo_addr      :  input.  fifo����ʼ��ַ = fifo_addr * 8
*
* Returns:
*    ���سɹ����ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
void __USBC_ConfigFifo_RxEp(__u32 usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __u32 temp = 0;
    __u32 size = 0;   //fifo_size = (size + 3)��2�η�
    __u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>--����sz, ����512����512����
	temp = fifo_size + 511;
	temp &= ~511;  //��511���������
	temp >>= 3;
	temp >>= 1;
	while(temp){
		size++;
		temp >>= 1;
	}

	//--<2>--����addr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_RXFIFOAD(usbc_base_addr));

	//--<2>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_RXFIFOSZ(usbc_base_addr));
	if(is_double_fifo){
		USBC_REG_set_bit_b(USBC_BP_RXFIFOSZ_DPB, USBC_REG_RXFIFOSZ(usbc_base_addr));
	}
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_Default
*
* Description:
*    ����ep ��fifo��ַ�ʹ�С��
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*	 ep_type		:  input.  ep������
*
* Returns:
*    ���سɹ����ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
void USBC_ConfigFifo_Default(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support
		break;

		case USBC_EP_TYPE_TX:
			__USBC_ConfigFifo_TxEp_Default(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_ConfigFifo_RxEp_Default(usbc_otg->base_addr);
		break;

		default:
		break;
	}
}

/*
***********************************************************************************
*                     USBC_ConfigFifo
*
* Description:
*    ����ep ��fifo��ַ�ʹ�С��
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*	 ep_type		:  input.  ep������
*    is_double_fifo :  input.  �Ƿ�ʹ��Ӳ��˫fifo
*    fifo_size      :  input.  fifo��С = 2��fifo_size�η�
*    fifo_addr      :  input.  fifo����ʼ��ַ = fifo_addr * 8
*
* Returns:
*    ���سɹ����ĳ���
*
* note:
*    ��
*
***********************************************************************************
*/
void USBC_ConfigFifo(__hdle hUSB, __u32 ep_type, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support
		break;

		case USBC_EP_TYPE_TX:
			__USBC_ConfigFifo_TxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_ConfigFifo_RxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;

		default:
		break;
	}
}

/*
***********************************************************************************
*                     USBC_GetLastFrameNumber
*
* Description:
*    ������һ֡��֡��
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_GetLastFrameNumber(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return USBC_Readl(USBC_REG_FRNUM(usbc_otg->base_addr));
}

/*
***********************************************************************************
*                     USBC_GetStatus_Dp
*
* Description:
*    ���dp��״̬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/

__u32 USBC_GetStatus_Dp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;

    return temp;
}



/*
***********************************************************************************
*                     USBC_GetStatus_Dm
*
* Description:
*    ���dm��״̬
*
* Arguments:
*    hUSB :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_GetStatus_Dm(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;

    return temp;
}


/*
***********************************************************************************
*                     USBC_GetStatus_Dp
*
* Description:
*    ���dp��״̬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/

__u32 USBC_GetStatus_DpDm(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;
	__u32 dp = 0;
	__u32 dm = 0;


	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	dp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;
	dm = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;
	return ((dp << 1) | dm);

}

/*
***********************************************************************************
*                     USBC_GetOtgMode_Form_ID
*
* Description:
*    ��vendor0 �� id ��õ�ǰOTG��ģʽ
*
* Arguments:
*    hUSB :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*    USBC_OTG_DEVICE / USBC_OTG_HOST
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_GetOtgMode_Form_ID(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if(usbc_otg == NULL){
		return USBC_OTG_DEVICE;
	}

    mode = USBC_REG_test_bit_l(USBC_BP_ISCR_MERGED_ID_STATUS, USBC_REG_ISCR(usbc_otg->base_addr));
    if(mode){
		return USBC_OTG_DEVICE;
	}else{
	    return USBC_OTG_HOST;
	}
}

/*
***********************************************************************************
*                     USBC_GetOtgMode_Form_BDevice
*
* Description:
*    �� OTG Device �� B-Device ��õ�ǰOTG��ģʽ
*
* Arguments:
*    hUSB :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*    USBC_OTG_DEVICE / USBC_OTG_HOST
*
* note:
*    ��
*
***********************************************************************************
*/
__u32 USBC_GetOtgMode_Form_BDevice(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if(usbc_otg == NULL){
		return USBC_OTG_DEVICE;
	}

    mode = USBC_REG_test_bit_b(USBC_BP_DEVCTL_B_DEVICE, USBC_REG_DEVCTL(usbc_otg->base_addr));
    if(mode){
		return USBC_OTG_DEVICE;
	}else{
	    return USBC_OTG_HOST;
	}
}

/*
***********************************************************************************
*                     USBC_SelectBus
*
* Description:
*    ѡ�����ݴ�������߷�ʽ
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*    io_type  :  input.  ���߷�ʽ, pio����dma.
*    ep_type  :  input.  ep������, rx �� tx��
*    ep_index :  input.  ep��
*
* Returns:
*
*
* note:
*    ��
*
***********************************************************************************
*/
void USBC_SelectBus(__hdle hUSB, __u32 io_type, __u32 ep_type, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

    reg_val = USBC_Readb(USBC_REG_VEND0(usbc_otg->base_addr));
    if(io_type == USBC_IO_TYPE_DMA){
		if(ep_type == USBC_EP_TYPE_TX){
			reg_val |= ((ep_index - 0x01) << 1) << USBC_BP_VEND0_DRQ_SEL;  //drq_sel
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;   //io_dma
		}else{
		    reg_val |= ((ep_index << 1) - 0x01) << USBC_BP_VEND0_DRQ_SEL;
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;
		}
	}else{
	    //reg_val &= ~(0x1 << USBC_BP_VEND0_DRQ_SEL);  //���drq_sel, ѡ��pio
	    reg_val &= 0x00;  //���drq_sel, ѡ��pio
	}

	USBC_Writeb(reg_val, USBC_REG_VEND0(usbc_otg->base_addr));
}

/* ���tx ep�жϱ�־λ */
static __u32 __USBC_INT_TxPending(__u32 usbc_base_addr)
{
    return (USBC_Readw(USBC_REG_INTTx(usbc_base_addr)));
}

/* ���tx ep�жϱ�־λ */
static void __USBC_INT_ClearTxPending(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_Writew((1 << ep_index), USBC_REG_INTTx(usbc_base_addr));
}

/* �������tx ep�жϱ�־λ */
static void __USBC_INT_ClearTxPendingAll(__u32 usbc_base_addr)
{
    USBC_Writew(0xffff, USBC_REG_INTTx(usbc_base_addr));
}

/* ���rx ep�жϱ�־λ */
static __u32 __USBC_INT_RxPending(__u32 usbc_base_addr)
{
    return (USBC_Readw(USBC_REG_INTRx(usbc_base_addr)));
}

/* ���rx ep�жϱ�־λ */
static void __USBC_INT_ClearRxPending(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_Writew((1 << ep_index), USBC_REG_INTRx(usbc_base_addr));
}

/* ���rx ep�жϱ�־λ */
static void __USBC_INT_ClearRxPendingAll(__u32 usbc_base_addr)
{
    USBC_Writew(0xffff, USBC_REG_INTRx(usbc_base_addr));
}

/* ��ĳһ��tx ep���ж� */
static void __USBC_INT_EnableTxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_set_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* ��ĳһ��rx ep���ж� */
static void __USBC_INT_EnableRxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_set_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* ��ĳһ��tx ep���ж� */
static void __USBC_INT_DisableTxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_clear_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* ��ĳһ��rx ep���ж� */
static void __USBC_INT_DisableRxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_clear_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* �����е�tx ep�ж� */
static void __USBC_INT_DisableTxAll(__u32 usbc_base_addr)
{
    USBC_Writew(0, USBC_REG_INTTxE(usbc_base_addr));
}

/* �����е�rx ep�ж� */
static void __USBC_INT_DisableRxAll(__u32 usbc_base_addr)
{
    USBC_Writew(0, USBC_REG_INTRxE(usbc_base_addr));
}

/* ���ep�жϱ�־λ */
__u32 USBC_INT_EpPending(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    return __USBC_INT_TxPending(usbc_otg->base_addr);

		case USBC_EP_TYPE_RX:
		    return __USBC_INT_RxPending(usbc_otg->base_addr);

		default:
			return 0;
	}
}

/* ���ep�жϱ�־λ */
void USBC_INT_ClearEpPending(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    __USBC_INT_ClearTxPending(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_ClearRxPending(usbc_otg->base_addr, ep_index);
		break;

		default:
			break;
	}

	return ;
}

/* ���ep�жϱ�־λ */
void USBC_INT_ClearEpPendingAll(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    __USBC_INT_ClearTxPendingAll(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_ClearRxPendingAll(usbc_otg->base_addr);
		break;

		default:
			break;
	}

	return ;
}

/* ���usb misc�жϱ�־λ */
__u32 USBC_INT_MiscPending(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return (USBC_Readb(USBC_REG_INTUSB(usbc_otg->base_addr)));
}

/* ���usb misc�жϱ�־λ */
void USBC_INT_ClearMiscPending(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(mask, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* �������usb misc�жϱ�־λ */
void USBC_INT_ClearMiscPendingAll(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(0xff, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* ��ĳһ��ep�ж� */
void USBC_INT_EnableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_EnableTxEp(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_EnableRxEp(usbc_otg->base_addr, ep_index);
		break;

		default:
        break;
	}

	return ;
}

/* ��ĳһ��usb misc�ж� */
void USBC_INT_EnableUsbMiscUint(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val |= mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* ��ĳtx ep���ж� */
void USBC_INT_DisableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_DisableTxEp(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_DisableRxEp(usbc_otg->base_addr, ep_index);
		break;

		default:
        break;
	}

	return;
}

/* ��ĳһ��usb misc�ж� */
void USBC_INT_DisableUsbMiscUint(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val &= ~mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* �����е�ep�ж� */
void USBC_INT_DisableEpAll(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_DisableTxAll(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_DisableRxAll(usbc_otg->base_addr);
		break;

		default:
        break;
	}

	return;
}

/* �����е�usb misc�ж� */
void USBC_INT_DisableUsbMiscAll(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(0, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* ��õ�ǰ���ep */
__u32 USBC_GetActiveEp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return USBC_Readb(USBC_REG_EPIND(usbc_otg->base_addr));
}

/* ���õ�ǰ�ep */
void USBC_SelectActiveEp(__hdle hUSB, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	USBC_Writeb(ep_index, USBC_REG_EPIND(usbc_otg->base_addr));
}

/* ��ǿusb�����ź� */
void USBC_EnhanceSignal(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	return;
}

/* ���� TestPacket ģʽ */
void USBC_EnterMode_TestPacket(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ���� Test_K ģʽ */
void USBC_EnterMode_Test_K(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ���� Test_J ģʽ */
void USBC_EnterMode_Test_J(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ���� Test_SE0_NAK ģʽ */
void USBC_EnterMode_Test_SE0_NAK(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ������в���ģʽ */
void USBC_EnterMode_Idle(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ */
static __u32 __USBC_WakeUp_ClearChangeDetect(__u32 reg_val)
{
    __u32 temp = reg_val;

	temp &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	return temp;
}

void USBC_SetWakeUp_Default(__hdle hUSB)
{

	return;
}

void USBC_EnableIdPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

    //vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableIdPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

	//vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_EnableDpDmPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

    //vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableDpDmPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

	//vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

static void __USBC_ForceIdDisable(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//vbus, id, dpdm�仯λ��д1����, ��������ڲ�������bit��ʱ�������Щλ
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToLow(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//��д00����д10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToHigh(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//��д00����д10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force id to (id_type) */
void USBC_ForceId(__hdle hUSB, __u32 id_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

    switch(id_type){
		case USBC_ID_TYPE_HOST:
			__USBC_ForceIdToLow(usbc_otg->base_addr);
		break;

		case USBC_ID_TYPE_DEVICE:
			__USBC_ForceIdToHigh(usbc_otg->base_addr);
		break;

		default:
			__USBC_ForceIdDisable(usbc_otg->base_addr);
	}
}

static void __USBC_ForceVbusValidDisable(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//��д00����д10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToLow(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//��д00����д10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToHigh(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//��д00����д11
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force vbus valid to (id_type) */
void USBC_ForceVbusValid(__hdle hUSB, __u32 vbus_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    switch(vbus_type){
		case USBC_VBUS_TYPE_LOW:
			__USBC_ForceVbusValidToLow(usbc_otg->base_addr);
		break;

		case USBC_VBUS_TYPE_HIGH:
			__USBC_ForceVbusValidToHigh(usbc_otg->base_addr);
		break;

		default:
			__USBC_ForceVbusValidDisable(usbc_otg->base_addr);
	}
	return ;
}

void USBC_A_valid_InputSelect(__hdle hUSB, __u32 source)
{

    return;
}

void USBC_EnableUsbLineStateBypass(__hdle hUSB)
{

    return;
}

void USBC_DisableUsbLineStateBypass(__hdle hUSB)
{

    return;
}

void USBC_EnableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_HOSC_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ����Hosc */
void USBC_DisableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_HOSC_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ��ѯ�Ƿ���� vbus �ж� */
__u32 USBC_IsVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //��ȡ�仯λ��ͬʱ, д1�����λ
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ��ѯ�Ƿ���� id �ж� */
__u32 USBC_IsIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //��ȡ�仯λ��ͬʱ, д1�����λ
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_ID_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ��ѯ�Ƿ���� dpdm �ж� */
__u32 USBC_IsDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //��ȡ�仯λ��ͬʱ, д1�����λ
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ���� wake �ж� */
void USBC_DisableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_IRQ_ENABLE);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ���� vbus �ж� */
void USBC_DisableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ���� id �ж� */
void USBC_DisableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ���� dpdm �ж� */
void USBC_DisableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ʹ�� wake �ж� */
void USBC_EnableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_IRQ_ENABLE;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ʹ�� vbus �仯�ж� */
void USBC_EnableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ʹ��id�仯�ж� */
void USBC_EnableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ʹ��dmdp�仯�ж� */
void USBC_EnableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ����ģʽ, ��üĴ�����ֵ */
__u32 USBC_TestMode_ReadReg(__hdle hUSB, __u32 offset, __u32 reg_width)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return reg_val;
	}

    if(reg_width == 8){
		reg_val = USBC_Readb(usbc_otg->base_addr + offset);
	}else if(reg_width == 16){
	    reg_val = USBC_Readw(usbc_otg->base_addr + offset);
	}else if(reg_width == 32){
	    reg_val = USBC_Readl(usbc_otg->base_addr + offset);
	}else{
	    reg_val = 0;
	}

	return reg_val;
}

/*
***********************************************************************************
*                     USBC_open_otg
*
* Description:
*    ��bsp�����ö˿ں�Ϊotg_no��togʹ��Ȩ
*
* Arguments:
*    otg_no  :  input.  ��Ҫʹ�õ�TOG�˿ں�, ��ΧΪ: 0 ~ USBC_MAX_CTL_NUM
*
* Returns:
*    �ɹ�, ����usbc_otg�����ʧ��, ����NULL
*
* note:
*    ��
*
***********************************************************************************
*/
#if 0
__hdle USBC_open_otg(__u32 otg_no)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;
	__u32 i = 0;

    //--<1>--otg_no���ܳ�������֧�ֵķ�Χ
    if(otg_no >= USBC_MAX_CTL_NUM){
		return 0;
	}

    //--<2>--�ڹ�����������һ����λ, ���֧��ͬʱ��8��
    for(i = 0; i < USBC_MAX_OPEN_NUM; i++){
		if(usbc_otg[i].used == 0){
			usbc_otg[i].used      = 1;
			usbc_otg[i].no        = i;
			usbc_otg[i].port_num  = otg_no;
			usbc_otg[i].base_addr = usbc_base_address[otg_no];

			return (__hdle)(&(usbc_otg[i]));
		}
	}

    return 0;
}
#else
__hdle USBC_open_otg(__u32 otg_no)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;

    //--<1>--otg_no���ܳ�������֧�ֵķ�Χ
    if(otg_no >= USBC_MAX_CTL_NUM){
		return 0;
	}

    //--<2>--�ڹ�����������һ����λ, ���֧��ͬʱ��8��
	usbc_otg[otg_no].used      = 1;
	usbc_otg[otg_no].no        = otg_no;
	usbc_otg[otg_no].port_num  = otg_no;
	usbc_otg[otg_no].base_addr = usbc_base_address[otg_no];

	return (__hdle)(&(usbc_otg[otg_no]));
}

#endif

/*
***********************************************************************************
*                     USBC_close_otg
*
* Description:
*    �ͷ�tog��ʹ��Ȩ
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg��õľ��, ��¼��USBC����Ҫ��һЩ�ؼ�����
*
* Returns:
*    0  :  �ɹ�
*   !0  :  ʧ��
*
* note:
*    ��
*
***********************************************************************************
*/
__s32  USBC_close_otg(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	memset(usbc_otg, 0, sizeof(__usbc_otg_t));

	return 0;
}


/*
***********************************************************************************
*                           USBC_init
*
* Description:
*
*
* Arguments:
*
*
* Returns:
*    0  :  �ɹ�
*   !0  :  ʧ��
*
* note:
*
*
***********************************************************************************
*/
__s32 USBC_init(bsp_usbc_t *usbc)
{
 //   __usbc_otg_t *usbc_otg = usbc_otg_array;
    __u32 i = 0;

//    memset(usbc_base_address, 0, sizeof(usbc_base_address));
//    memset(&usbc_info_g, 0, sizeof(__fifo_info_t));
//    memset(usbc_otg, 0, (USBC_MAX_OPEN_NUM * sizeof(__usbc_otg_t)));

    /* ���� driver �������� usb �������Ļ�ַ */
/*
    for(i = 0; i < USBC_MAX_CTL_NUM; i++){
        __u32 port_num = 0;

        port_num = usbc->usbc_info[i].num;
        usbc_base_address[i] = usbc->usbc_info[i].base;
    }
*/
    for(i = 0; i < USBC_MAX_CTL_NUM; i++){
        __u32 port_num = 0;

		if(usbc->usbc_info[i].base){
	        port_num = usbc->usbc_info[i].num;
	        usbc_base_address[i] = usbc->usbc_info[i].base;
		}
    }

    return 0;
}

/*
***********************************************************************************
*                            USBC_exit
*
* Description:
*
*
* Arguments:
*
*
* Returns:
*    0  :  �ɹ�
*   !0  :  ʧ��
*
* note:
*
*
***********************************************************************************
*/
__s32 USBC_exit(bsp_usbc_t *usbc)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;

    memset(&usbc_info_g, 0, sizeof(__fifo_info_t));
    memset(usbc_otg, 0, (USBC_MAX_OPEN_NUM * sizeof(__usbc_otg_t)));
    memset(usbc_base_address, 0, sizeof(usbc_base_address));

    return 0;
}

/* USB��������ѡ��, ��д���ݵ� */
EXPORT_SYMBOL(USBC_OTG_SelectMode);

EXPORT_SYMBOL(USBC_ReadLenFromFifo);
EXPORT_SYMBOL(USBC_WritePacket);
EXPORT_SYMBOL(USBC_ReadPacket);

EXPORT_SYMBOL(USBC_ConfigFIFO_Base);
EXPORT_SYMBOL(USBC_GetPortFifoStartAddr);
EXPORT_SYMBOL(USBC_GetPortFifoSize);
EXPORT_SYMBOL(USBC_SelectFIFO);
EXPORT_SYMBOL(USBC_ConfigFifo_Default);
EXPORT_SYMBOL(USBC_ConfigFifo);

EXPORT_SYMBOL(USBC_SelectBus);

EXPORT_SYMBOL(USBC_GetActiveEp);
EXPORT_SYMBOL(USBC_SelectActiveEp);

EXPORT_SYMBOL(USBC_EnhanceSignal);

EXPORT_SYMBOL(USBC_GetLastFrameNumber);


/* usb �жϲ������� */
EXPORT_SYMBOL(USBC_INT_EpPending);
EXPORT_SYMBOL(USBC_INT_MiscPending);
EXPORT_SYMBOL(USBC_INT_ClearEpPending);
EXPORT_SYMBOL(USBC_INT_ClearMiscPending);
EXPORT_SYMBOL(USBC_INT_ClearEpPendingAll);
EXPORT_SYMBOL(USBC_INT_ClearMiscPendingAll);

EXPORT_SYMBOL(USBC_INT_EnableEp);
EXPORT_SYMBOL(USBC_INT_EnableUsbMiscUint);

EXPORT_SYMBOL(USBC_INT_DisableEp);
EXPORT_SYMBOL(USBC_INT_DisableUsbMiscUint);

EXPORT_SYMBOL(USBC_INT_DisableEpAll);
EXPORT_SYMBOL(USBC_INT_DisableUsbMiscAll);


/* usb ���Ʋ������� */
EXPORT_SYMBOL(USBC_GetVbusStatus);
EXPORT_SYMBOL(USBC_GetStatus_Dp);
EXPORT_SYMBOL(USBC_GetStatus_Dm);
EXPORT_SYMBOL(USBC_GetStatus_DpDm);

EXPORT_SYMBOL(USBC_GetOtgMode_Form_ID);
EXPORT_SYMBOL(USBC_GetOtgMode_Form_BDevice);

EXPORT_SYMBOL(USBC_SetWakeUp_Default);

EXPORT_SYMBOL(USBC_EnableIdPullUp);
EXPORT_SYMBOL(USBC_DisableIdPullUp);
EXPORT_SYMBOL(USBC_EnableDpDmPullUp);
EXPORT_SYMBOL(USBC_DisableDpDmPullUp);

EXPORT_SYMBOL(USBC_ForceId);
EXPORT_SYMBOL(USBC_ForceVbusValid);

EXPORT_SYMBOL(USBC_A_valid_InputSelect);

EXPORT_SYMBOL(USBC_EnableUsbLineStateBypass);
EXPORT_SYMBOL(USBC_DisableUsbLineStateBypass);
EXPORT_SYMBOL(USBC_EnableHosc);
EXPORT_SYMBOL(USBC_DisableHosc);

EXPORT_SYMBOL(USBC_IsVbusChange);
EXPORT_SYMBOL(USBC_IsIdChange);
EXPORT_SYMBOL(USBC_IsDpDmChange);

EXPORT_SYMBOL(USBC_DisableWakeIrq);
EXPORT_SYMBOL(USBC_DisableVbusChange);
EXPORT_SYMBOL(USBC_DisableIdChange);
EXPORT_SYMBOL(USBC_DisableDpDmChange);

EXPORT_SYMBOL(USBC_EnableWakeIrq);
EXPORT_SYMBOL(USBC_EnableVbusChange);
EXPORT_SYMBOL(USBC_EnableIdChange);
EXPORT_SYMBOL(USBC_EnableDpDmChange);

/* usb ����ģʽ */
EXPORT_SYMBOL(USBC_EnterMode_TestPacket);
EXPORT_SYMBOL(USBC_EnterMode_Test_K);
EXPORT_SYMBOL(USBC_EnterMode_Test_J);
EXPORT_SYMBOL(USBC_EnterMode_Test_SE0_NAK);
EXPORT_SYMBOL(USBC_EnterMode_Idle);

EXPORT_SYMBOL(USBC_TestMode_ReadReg);

EXPORT_SYMBOL(USBC_open_otg);
EXPORT_SYMBOL(USBC_close_otg);
EXPORT_SYMBOL(USBC_init);
EXPORT_SYMBOL(USBC_exit);



