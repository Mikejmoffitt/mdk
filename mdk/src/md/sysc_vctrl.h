#ifndef MD_SYSC_VCTRL
#define MD_SYSC_VCTRL

typedef enum MdCVReg
{
	MD_CVREG_SCREEN_BLANK      = 0x01,  // Set this bit to blank the video.
	MD_CVREG_PROTECTION_RESET  = 0x02,
	MD_CVREG_STANDARD_PAL_MODE = 0x04,
} MdCVReg;

void md_sysc_vctrl_set(MdCVReg reg_data);

#endif  // MD_SYSC_VCTRL
