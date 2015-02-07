

#ifndef _ARCH_ARM_MACH_MXC91231_CRM_REGS_H_
#define _ARCH_ARM_MACH_MXC91231_CRM_REGS_H_

#define CKIL_CLK_FREQ			32768

#define MXC_CRM_AP_BASE			MXC91231_IO_ADDRESS(MXC91231_CRM_AP_BASE_ADDR)
#define MXC_CRM_COM_BASE		MXC91231_IO_ADDRESS(MXC91231_CRM_COM_BASE_ADDR)
#define MXC_DSM_BASE			MXC91231_IO_ADDRESS(MXC91231_DSM_BASE_ADDR)
#define MXC_PLL0_BASE			MXC91231_IO_ADDRESS(MXC91231_PLL0_BASE_ADDR)
#define MXC_PLL1_BASE			MXC91231_IO_ADDRESS(MXC91231_PLL1_BASE_ADDR)
#define MXC_PLL2_BASE			MXC91231_IO_ADDRESS(MXC91231_PLL2_BASE_ADDR)
#define MXC_CLKCTL_BASE			MXC91231_IO_ADDRESS(MXC91231_CLKCTL_BASE_ADDR)

/* PLL Register Offsets */
#define MXC_PLL_DP_CTL			0x00
#define MXC_PLL_DP_CONFIG		0x04
#define MXC_PLL_DP_OP			0x08
#define MXC_PLL_DP_MFD			0x0C
#define MXC_PLL_DP_MFN			0x10
#define MXC_PLL_DP_HFS_OP		0x1C
#define MXC_PLL_DP_HFS_MFD		0x20
#define MXC_PLL_DP_HFS_MFN		0x24

/* PLL Register Bit definitions */
#define MXC_PLL_DP_CTL_DPDCK0_2_EN	0x1000
#define MXC_PLL_DP_CTL_ADE		0x800
#define MXC_PLL_DP_CTL_REF_CLK_DIV	0x400
#define MXC_PLL_DP_CTL_HFSM		0x80
#define MXC_PLL_DP_CTL_PRE		0x40
#define MXC_PLL_DP_CTL_UPEN		0x20
#define MXC_PLL_DP_CTL_RST		0x10
#define MXC_PLL_DP_CTL_RCP		0x8
#define MXC_PLL_DP_CTL_PLM		0x4
#define MXC_PLL_DP_CTL_BRM0		0x2
#define MXC_PLL_DP_CTL_LRF		0x1

#define MXC_PLL_DP_OP_MFI_OFFSET	4
#define MXC_PLL_DP_OP_MFI_MASK		0xF
#define MXC_PLL_DP_OP_PDF_OFFSET	0
#define MXC_PLL_DP_OP_PDF_MASK		0xF

#define MXC_PLL_DP_MFD_OFFSET		0
#define MXC_PLL_DP_MFD_MASK		0x7FFFFFF

#define MXC_PLL_DP_MFN_OFFSET		0
#define MXC_PLL_DP_MFN_MASK		0x7FFFFFF

/* CRM AP Register Offsets */
#define MXC_CRMAP_ASCSR			(MXC_CRM_AP_BASE + 0x00)
#define MXC_CRMAP_ACDR			(MXC_CRM_AP_BASE + 0x04)
#define MXC_CRMAP_ACDER1		(MXC_CRM_AP_BASE + 0x08)
#define MXC_CRMAP_ACDER2		(MXC_CRM_AP_BASE + 0x0C)
#define MXC_CRMAP_ACGCR			(MXC_CRM_AP_BASE + 0x10)
#define MXC_CRMAP_ACCGCR		(MXC_CRM_AP_BASE + 0x14)
#define MXC_CRMAP_AMLPMRA		(MXC_CRM_AP_BASE + 0x18)
#define MXC_CRMAP_AMLPMRB		(MXC_CRM_AP_BASE + 0x1C)
#define MXC_CRMAP_AMLPMRC		(MXC_CRM_AP_BASE + 0x20)
#define MXC_CRMAP_AMLPMRD		(MXC_CRM_AP_BASE + 0x24)
#define MXC_CRMAP_AMLPMRE1		(MXC_CRM_AP_BASE + 0x28)
#define MXC_CRMAP_AMLPMRE2		(MXC_CRM_AP_BASE + 0x2C)
#define MXC_CRMAP_AMLPMRF		(MXC_CRM_AP_BASE + 0x30)
#define MXC_CRMAP_AMLPMRG		(MXC_CRM_AP_BASE + 0x34)
#define MXC_CRMAP_APGCR			(MXC_CRM_AP_BASE + 0x38)
#define MXC_CRMAP_ACSR			(MXC_CRM_AP_BASE + 0x3C)
#define MXC_CRMAP_ADCR			(MXC_CRM_AP_BASE + 0x40)
#define MXC_CRMAP_ACR			(MXC_CRM_AP_BASE + 0x44)
#define MXC_CRMAP_AMCR			(MXC_CRM_AP_BASE + 0x48)
#define MXC_CRMAP_APCR			(MXC_CRM_AP_BASE + 0x4C)
#define MXC_CRMAP_AMORA			(MXC_CRM_AP_BASE + 0x50)
#define MXC_CRMAP_AMORB			(MXC_CRM_AP_BASE + 0x54)
#define MXC_CRMAP_AGPR			(MXC_CRM_AP_BASE + 0x58)
#define MXC_CRMAP_APRA			(MXC_CRM_AP_BASE + 0x5C)
#define MXC_CRMAP_APRB			(MXC_CRM_AP_BASE + 0x60)
#define MXC_CRMAP_APOR			(MXC_CRM_AP_BASE + 0x64)
#define MXC_CRMAP_ADFMR			(MXC_CRM_AP_BASE + 0x68)

/* CRM AP Register Bit definitions */
#define MXC_CRMAP_ASCSR_CRS			0x10000
#define MXC_CRMAP_ASCSR_AP_PATDIV2_OFFSET	15
#define MXC_CRMAP_ASCSR_AP_PATREF_DIV2		0x8000
#define MXC_CRMAP_ASCSR_USBSEL_OFFSET		13
#define MXC_CRMAP_ASCSR_USBSEL_MASK		(0x3 << 13)
#define MXC_CRMAP_ASCSR_CSISEL_OFFSET		11
#define MXC_CRMAP_ASCSR_CSISEL_MASK		(0x3 << 11)
#define MXC_CRMAP_ASCSR_SSI2SEL_OFFSET		7
#define MXC_CRMAP_ASCSR_SSI2SEL_MASK		(0x3 << 7)
#define MXC_CRMAP_ASCSR_SSI1SEL_OFFSET		5
#define MXC_CRMAP_ASCSR_SSI1SEL_MASK		(0x3 << 5)
#define MXC_CRMAP_ASCSR_APSEL_OFFSET		3
#define MXC_CRMAP_ASCSR_APSEL_MASK		(0x3 << 3)
#define MXC_CRMAP_ASCSR_AP_PATDIV1_OFFSET	2
#define MXC_CRMAP_ASCSR_AP_PATREF_DIV1		0x4
#define MXC_CRMAP_ASCSR_APISEL			0x1

#define MXC_CRMAP_ACDR_ARMDIV_OFFSET		8
#define MXC_CRMAP_ACDR_ARMDIV_MASK		(0xF << 8)
#define MXC_CRMAP_ACDR_AHBDIV_OFFSET		4
#define MXC_CRMAP_ACDR_AHBDIV_MASK		(0xF << 4)
#define MXC_CRMAP_ACDR_IPDIV_OFFSET		0
#define MXC_CRMAP_ACDR_IPDIV_MASK		0xF

#define MXC_CRMAP_ACDER1_CSIEN_OFFSET		30
#define MXC_CRMAP_ACDER1_CSIDIV_OFFSET		24
#define MXC_CRMAP_ACDER1_CSIDIV_MASK		(0x3F << 24)
#define MXC_CRMAP_ACDER1_SSI2EN_OFFSET		14
#define MXC_CRMAP_ACDER1_SSI2DIV_OFFSET		8
#define MXC_CRMAP_ACDER1_SSI2DIV_MASK		(0x3F << 8)
#define MXC_CRMAP_ACDER1_SSI1EN_OFFSET		6
#define MXC_CRMAP_ACDER1_SSI1DIV_OFFSET		0
#define MXC_CRMAP_ACDER1_SSI1DIV_MASK		0x3F

#define MXC_CRMAP_ACDER2_CRCT_CLK_DIV_OFFSET	24
#define MXC_CRMAP_ACDER2_CRCT_CLK_DIV_MASK	(0x7 << 24)
#define MXC_CRMAP_ACDER2_NFCEN_OFFSET		20
#define MXC_CRMAP_ACDER2_NFCDIV_OFFSET		16
#define MXC_CRMAP_ACDER2_NFCDIV_MASK		(0xF << 16)
#define MXC_CRMAP_ACDER2_USBEN_OFFSET		12
#define MXC_CRMAP_ACDER2_USBDIV_OFFSET		8
#define MXC_CRMAP_ACDER2_USBDIV_MASK		(0xF << 8)
#define MXC_CRMAP_ACDER2_BAUD_ISEL_OFFSET	5
#define MXC_CRMAP_ACDER2_BAUD_ISEL_MASK		(0x3 << 5)
#define MXC_CRMAP_ACDER2_BAUDDIV_OFFSET		0
#define MXC_CRMAP_ACDER2_BAUDDIV_MASK		0xF

#define MXC_CRMAP_AMLPMRA_MLPMA7_OFFSET		22
#define MXC_CRMAP_AMLPMRA_MLPMA7_MASK		(0x7 << 22)
#define MXC_CRMAP_AMLPMRA_MLPMA6_OFFSET		19
#define MXC_CRMAP_AMLPMRA_MLPMA6_MASK		(0x7 << 19)
#define MXC_CRMAP_AMLPMRA_MLPMA4_OFFSET		12
#define MXC_CRMAP_AMLPMRA_MLPMA4_MASK		(0x7 << 12)
#define MXC_CRMAP_AMLPMRA_MLPMA3_OFFSET		9
#define MXC_CRMAP_AMLPMRA_MLPMA3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRA_MLPMA2_OFFSET		6
#define MXC_CRMAP_AMLPMRA_MLPMA2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRA_MLPMA1_OFFSET		3
#define MXC_CRMAP_AMLPMRA_MLPMA1_MASK		(0x7 << 3)

#define MXC_CRMAP_AMLPMRB_MLPMB0_OFFSET		0
#define MXC_CRMAP_AMLPMRB_MLPMB0_MASK		0x7

#define MXC_CRMAP_AMLPMRC_MLPMC9_OFFSET		28
#define MXC_CRMAP_AMLPMRC_MLPMC9_MASK		(0x7 << 28)
#define MXC_CRMAP_AMLPMRC_MLPMC7_OFFSET		22
#define MXC_CRMAP_AMLPMRC_MLPMC7_MASK		(0x7 << 22)
#define MXC_CRMAP_AMLPMRC_MLPMC5_OFFSET		16
#define MXC_CRMAP_AMLPMRC_MLPMC5_MASK		(0x7 << 16)
#define MXC_CRMAP_AMLPMRC_MLPMC4_OFFSET		12
#define MXC_CRMAP_AMLPMRC_MLPMC4_MASK		(0x7 << 12)
#define MXC_CRMAP_AMLPMRC_MLPMC3_OFFSET		9
#define MXC_CRMAP_AMLPMRC_MLPMC3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRC_MLPMC2_OFFSET		6
#define MXC_CRMAP_AMLPMRC_MLPMC2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRC_MLPMC1_OFFSET		3
#define MXC_CRMAP_AMLPMRC_MLPMC1_MASK		(0x7 << 3)
#define MXC_CRMAP_AMLPMRC_MLPMC0_OFFSET		0
#define MXC_CRMAP_AMLPMRC_MLPMC0_MASK		0x7

#define MXC_CRMAP_AMLPMRD_MLPMD7_OFFSET		22
#define MXC_CRMAP_AMLPMRD_MLPMD7_MASK		(0x7 << 22)
#define MXC_CRMAP_AMLPMRD_MLPMD4_OFFSET		12
#define MXC_CRMAP_AMLPMRD_MLPMD4_MASK		(0x7 << 12)
#define MXC_CRMAP_AMLPMRD_MLPMD3_OFFSET		9
#define MXC_CRMAP_AMLPMRD_MLPMD3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRD_MLPMD2_OFFSET		6
#define MXC_CRMAP_AMLPMRD_MLPMD2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRD_MLPMD0_OFFSET		0
#define MXC_CRMAP_AMLPMRD_MLPMD0_MASK		0x7

#define MXC_CRMAP_AMLPMRE1_MLPME9_OFFSET	28
#define MXC_CRMAP_AMLPMRE1_MLPME9_MASK		(0x7 << 28)
#define MXC_CRMAP_AMLPMRE1_MLPME8_OFFSET	25
#define MXC_CRMAP_AMLPMRE1_MLPME8_MASK		(0x7 << 25)
#define MXC_CRMAP_AMLPMRE1_MLPME7_OFFSET	22
#define MXC_CRMAP_AMLPMRE1_MLPME7_MASK		(0x7 << 22)
#define MXC_CRMAP_AMLPMRE1_MLPME6_OFFSET	19
#define MXC_CRMAP_AMLPMRE1_MLPME6_MASK		(0x7 << 19)
#define MXC_CRMAP_AMLPMRE1_MLPME5_OFFSET	16
#define MXC_CRMAP_AMLPMRE1_MLPME5_MASK		(0x7 << 16)
#define MXC_CRMAP_AMLPMRE1_MLPME4_OFFSET	12
#define MXC_CRMAP_AMLPMRE1_MLPME4_MASK		(0x7 << 12)
#define MXC_CRMAP_AMLPMRE1_MLPME3_OFFSET	9
#define MXC_CRMAP_AMLPMRE1_MLPME3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRE1_MLPME2_OFFSET	6
#define MXC_CRMAP_AMLPMRE1_MLPME2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRE1_MLPME1_OFFSET	3
#define MXC_CRMAP_AMLPMRE1_MLPME1_MASK		(0x7 << 3)
#define MXC_CRMAP_AMLPMRE1_MLPME0_OFFSET	0
#define MXC_CRMAP_AMLPMRE1_MLPME0_MASK		0x7

#define MXC_CRMAP_AMLPMRE2_MLPME0_OFFSET	0
#define MXC_CRMAP_AMLPMRE2_MLPME0_MASK		0x7

#define MXC_CRMAP_AMLPMRF_MLPMF6_OFFSET		19
#define MXC_CRMAP_AMLPMRF_MLPMF6_MASK		(0x7 << 19)
#define MXC_CRMAP_AMLPMRF_MLPMF5_OFFSET		16
#define MXC_CRMAP_AMLPMRF_MLPMF5_MASK		(0x7 << 16)
#define MXC_CRMAP_AMLPMRF_MLPMF3_OFFSET		9
#define MXC_CRMAP_AMLPMRF_MLPMF3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRF_MLPMF2_OFFSET		6
#define MXC_CRMAP_AMLPMRF_MLPMF2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRF_MLPMF1_OFFSET		3
#define MXC_CRMAP_AMLPMRF_MLPMF1_MASK		(0x7 << 3)
#define MXC_CRMAP_AMLPMRF_MLPMF0_OFFSET		0
#define MXC_CRMAP_AMLPMRF_MLPMF0_MASK		(0x7 << 0)

#define MXC_CRMAP_AMLPMRG_MLPMG9_OFFSET		28
#define MXC_CRMAP_AMLPMRG_MLPMG9_MASK		(0x7 << 28)
#define MXC_CRMAP_AMLPMRG_MLPMG7_OFFSET		22
#define MXC_CRMAP_AMLPMRG_MLPMG7_MASK		(0x7 << 22)
#define MXC_CRMAP_AMLPMRG_MLPMG6_OFFSET		19
#define MXC_CRMAP_AMLPMRG_MLPMG6_MASK		(0x7 << 19)
#define MXC_CRMAP_AMLPMRG_MLPMG5_OFFSET		16
#define MXC_CRMAP_AMLPMRG_MLPMG5_MASK		(0x7 << 16)
#define MXC_CRMAP_AMLPMRG_MLPMG4_OFFSET		12
#define MXC_CRMAP_AMLPMRG_MLPMG4_MASK		(0x7 << 12)
#define MXC_CRMAP_AMLPMRG_MLPMG3_OFFSET		9
#define MXC_CRMAP_AMLPMRG_MLPMG3_MASK		(0x7 << 9)
#define MXC_CRMAP_AMLPMRG_MLPMG2_OFFSET		6
#define MXC_CRMAP_AMLPMRG_MLPMG2_MASK		(0x7 << 6)
#define MXC_CRMAP_AMLPMRG_MLPMG1_OFFSET		3
#define MXC_CRMAP_AMLPMRG_MLPMG1_MASK		(0x7 << 3)
#define MXC_CRMAP_AMLPMRG_MLPMG0_OFFSET		0
#define MXC_CRMAP_AMLPMRG_MLPMG0_MASK		0x7

#define MXC_CRMAP_AGPR_IPUPAD_OFFSET		20
#define MXC_CRMAP_AGPR_IPUPAD_MASK		(0x7 << 20)

#define MXC_CRMAP_APRA_EL1TEN_OFFSET		29
#define MXC_CRMAP_APRA_SIMEN_OFFSET		24
#define MXC_CRMAP_APRA_UART3DIV_OFFSET		17
#define MXC_CRMAP_APRA_UART3DIV_MASK		(0xF << 17)
#define MXC_CRMAP_APRA_UART3EN_OFFSET		16
#define MXC_CRMAP_APRA_SAHARA_DIV2_CLKEN_OFFSET	14
#define MXC_CRMAP_APRA_MQSPIEN_OFFSET		13
#define MXC_CRMAP_APRA_UART2EN_OFFSET		8
#define MXC_CRMAP_APRA_UART1EN_OFFSET		0

#define MXC_CRMAP_APRB_SDHC2_ISEL_OFFSET	13
#define MXC_CRMAP_APRB_SDHC2_ISEL_MASK		(0x7 << 13)
#define MXC_CRMAP_APRB_SDHC2_DIV_OFFSET		9
#define MXC_CRMAP_APRB_SDHC2_DIV_MASK		(0xF << 9)
#define MXC_CRMAP_APRB_SDHC2EN_OFFSET		8
#define MXC_CRMAP_APRB_SDHC1_ISEL_OFFSET	5
#define MXC_CRMAP_APRB_SDHC1_ISEL_MASK		(0x7 << 5)
#define MXC_CRMAP_APRB_SDHC1_DIV_OFFSET		1
#define MXC_CRMAP_APRB_SDHC1_DIV_MASK		(0xF << 1)
#define MXC_CRMAP_APRB_SDHC1EN_OFFSET		0

#define MXC_CRMAP_ACSR_ADS_OFFSET		8
#define MXC_CRMAP_ACSR_ADS			(0x1 << 8)
#define MXC_CRMAP_ACSR_ACS			0x1

#define MXC_CRMAP_ADCR_LFDF_0			(0x0 << 8)
#define MXC_CRMAP_ADCR_LFDF_2			(0x1 << 8)
#define MXC_CRMAP_ADCR_LFDF_4			(0x2 << 8)
#define MXC_CRMAP_ADCR_LFDF_8			(0x3 << 8)
#define MXC_CRMAP_ADCR_LFDF_OFFSET		8
#define MXC_CRMAP_ADCR_LFDF_MASK		(0x3 << 8)
#define MXC_CRMAP_ADCR_ALT_PLL			0x80
#define MXC_CRMAP_ADCR_DFS_DIVEN		0x20
#define MXC_CRMAP_ADCR_DIV_BYP			0x2
#define MXC_CRMAP_ADCR_VSTAT			0x8
#define MXC_CRMAP_ADCR_TSTAT			0x10
#define MXC_CRMAP_ADCR_DVFS_VCTRL		0x10
#define MXC_CRMAP_ADCR_CLK_ON			0x40

#define MXC_CRMAP_ADFMR_FC_OFFSET		16
#define MXC_CRMAP_ADFMR_FC_MASK			(0x1F << 16)
#define MXC_CRMAP_ADFMR_MF_OFFSET		1
#define MXC_CRMAP_ADFMR_MF_MASK			(0x3FF << 1)
#define MXC_CRMAP_ADFMR_DFM_CLK_READY		0x1
#define MXC_CRMAP_ADFMR_DFM_PWR_DOWN		0x8000

#define MXC_CRMAP_ACR_CKOHS_HIGH		(1 << 18)
#define MXC_CRMAP_ACR_CKOS_HIGH			(1 << 16)
#define MXC_CRMAP_ACR_CKOHS_MASK		(0x7 << 12)
#define MXC_CRMAP_ACR_CKOHD			(1 << 11)
#define MXC_CRMAP_ACR_CKOHDIV_MASK		(0xF << 8)
#define MXC_CRMAP_ACR_CKOHDIV_OFFSET		8
#define MXC_CRMAP_ACR_CKOD			(1 << 7)
#define MXC_CRMAP_ACR_CKOS_MASK			(0x7 << 4)

/* AP Warm reset */
#define MXC_CRMAP_AMCR_SW_AP			(1 << 14)

/* Bit definitions of ACGCR in CRM_AP for tree level clock gating */
#define MXC_CRMAP_ACGCR_ACG0_STOP_WAIT		0x00000001
#define MXC_CRMAP_ACGCR_ACG0_STOP		0x00000003
#define MXC_CRMAP_ACGCR_ACG0_RUN		0x00000007
#define MXC_CRMAP_ACGCR_ACG0_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG1_STOP_WAIT		0x00000008
#define MXC_CRMAP_ACGCR_ACG1_STOP		0x00000018
#define MXC_CRMAP_ACGCR_ACG1_RUN		0x00000038
#define MXC_CRMAP_ACGCR_ACG1_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG2_STOP_WAIT		0x00000040
#define MXC_CRMAP_ACGCR_ACG2_STOP		0x000000C0
#define MXC_CRMAP_ACGCR_ACG2_RUN		0x000001C0
#define MXC_CRMAP_ACGCR_ACG2_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG3_STOP_WAIT		0x00000200
#define MXC_CRMAP_ACGCR_ACG3_STOP		0x00000600
#define MXC_CRMAP_ACGCR_ACG3_RUN		0x00000E00
#define MXC_CRMAP_ACGCR_ACG3_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG4_STOP_WAIT		0x00001000
#define MXC_CRMAP_ACGCR_ACG4_STOP		0x00003000
#define MXC_CRMAP_ACGCR_ACG4_RUN		0x00007000
#define MXC_CRMAP_ACGCR_ACG4_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG5_STOP_WAIT		0x00010000
#define MXC_CRMAP_ACGCR_ACG5_STOP		0x00030000
#define MXC_CRMAP_ACGCR_ACG5_RUN		0x00070000
#define MXC_CRMAP_ACGCR_ACG5_DISABLED		0x00000000

#define MXC_CRMAP_ACGCR_ACG6_STOP_WAIT		0x00080000
#define MXC_CRMAP_ACGCR_ACG6_STOP		0x00180000
#define MXC_CRMAP_ACGCR_ACG6_RUN		0x00380000
#define MXC_CRMAP_ACGCR_ACG6_DISABLED		0x00000000

#define NUM_GATE_CTRL				6

/* CRM COM Register Offsets */
#define MXC_CRMCOM_CSCR				(MXC_CRM_COM_BASE + 0x0C)
#define MXC_CRMCOM_CCCR				(MXC_CRM_COM_BASE + 0x10)

/* CRM COM Bit Definitions */
#define MXC_CRMCOM_CSCR_PPD1			0x08000000
#define MXC_CRMCOM_CSCR_CKOHSEL			(1 << 18)
#define MXC_CRMCOM_CSCR_CKOSEL			(1 << 17)
#define MXC_CRMCOM_CCCR_CC_DIV_OFFSET		8
#define MXC_CRMCOM_CCCR_CC_DIV_MASK		(0x1F << 8)
#define MXC_CRMCOM_CCCR_CC_SEL_OFFSET		0
#define MXC_CRMCOM_CCCR_CC_SEL_MASK		0x3

/* DSM Register Offsets */
#define MXC_DSM_SLEEP_TIME			(MXC_DSM_BASE + 0x0c)
#define MXC_DSM_CONTROL0			(MXC_DSM_BASE + 0x20)
#define MXC_DSM_CONTROL1			(MXC_DSM_BASE + 0x24)
#define MXC_DSM_CTREN				(MXC_DSM_BASE + 0x28)
#define MXC_DSM_WARM_PER			(MXC_DSM_BASE + 0x40)
#define MXC_DSM_LOCK_PER			(MXC_DSM_BASE + 0x44)
#define MXC_DSM_MGPER				(MXC_DSM_BASE + 0x4c)
#define MXC_DSM_CRM_CONTROL			(MXC_DSM_BASE + 0x50)

/* Bit definitions of various registers in DSM */
#define MXC_DSM_CRM_CTRL_DVFS_BYP		0x00000008
#define MXC_DSM_CRM_CTRL_DVFS_VCTRL		0x00000004
#define MXC_DSM_CRM_CTRL_LPMD1			0x00000002
#define MXC_DSM_CRM_CTRL_LPMD0			0x00000001
#define MXC_DSM_CRM_CTRL_LPMD_STOP_MODE		0x00000000
#define MXC_DSM_CRM_CTRL_LPMD_WAIT_MODE		0x00000001
#define MXC_DSM_CRM_CTRL_LPMD_RUN_MODE		0x00000003
#define MXC_DSM_CONTROL0_STBY_COMMIT_EN		0x00000200
#define MXC_DSM_CONTROL0_MSTR_EN		0x00000001
#define MXC_DSM_CONTROL0_RESTART		0x00000010
/* Counter Block reset */
#define MXC_DSM_CONTROL1_CB_RST			0x00000002
/* State Machine reset */
#define MXC_DSM_CONTROL1_SM_RST			0x00000004
/* Bit needed to reset counter block */
#define MXC_CONTROL1_RST_CNT32			0x00000008
#define MXC_DSM_CONTROL1_RST_CNT32_EN		0x00000800
#define MXC_DSM_CONTROL1_SLEEP			0x00000100
#define MXC_DSM_CONTROL1_WAKEUP_DISABLE		0x00004000
#define MXC_DSM_CTREN_CNT32			0x00000001

/* Magic Fix enable bit */
#define MXC_DSM_MGPER_EN_MGFX			0x80000000
#define MXC_DSM_MGPER_PER_MASK			0x000003FF
#define MXC_DSM_MGPER_PER(n)			(MXC_DSM_MGPER_PER_MASK & n)

/* Address offsets of the CLKCTL registers */
#define MXC_CLKCTL_GP_CTRL	(MXC_CLKCTL_BASE + 0x00)
#define MXC_CLKCTL_GP_SER	(MXC_CLKCTL_BASE + 0x04)
#define MXC_CLKCTL_GP_CER	(MXC_CLKCTL_BASE + 0x08)

#endif /* _ARCH_ARM_MACH_MXC91231_CRM_REGS_H_ */
